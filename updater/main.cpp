#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <winsvc.h>
#include <Shlwapi.h>
#include <Psapi.h>
#include <cstdint>
#include <ShlObj.h>

#define SERVICE_NAME    "Obl---Daemon"

struct serviceAccess {
    SC_HANDLE manager, service;
    char name [100];

    serviceAccess (char *_name): service (0), manager (0) {
        attach (_name);
    }

    virtual ~serviceAccess () {
        if (service) CloseServiceHandle (service);
        if (manager) CloseServiceHandle (manager);
    }

    bool attach (char *_name) {
        strcpy (name, _name);
        manager = OpenSCManager (NULL, NULL, SC_MANAGER_CREATE_SERVICE);
        service = manager ? OpenService (manager, name, SERVICE_ALL_ACCESS) : 0;

        return service != 0;
    }

    bool installed () {
        return service != 0;
    }

    bool queryStatus (SERVICE_STATUS *status) {
        bool result = false;

        if (service) result = QueryServiceStatus (service, status) != 0;

        return result;
    }

    bool stop (SERVICE_STATUS *status) {
        return service ? ControlService (service, SERVICE_CONTROL_STOP, status) != 0 : false;
    }

    bool start () {
        return service ? StartService (service, 0, 0) != 0 : false;
    }

    bool getServiceBinaryFolder (char *folder, size_t size) {
        union {
            QUERY_SERVICE_CONFIG cfg;
            char buffer [1000];
        };

        DWORD bytesNeeded;

        bool result = service ? QueryServiceConfig (service, & cfg, sizeof (buffer), & bytesNeeded) != 0 : false;

        if (result) {
            strncpy (folder, cfg.lpBinaryPathName, size);
            PathRemoveFileSpec (folder);
        } else {
            auto error = GetLastError ();
            memset (folder, 0, size);
        }

        return result;
    }

    bool setStartupType (DWORD startupType) {
        return ChangeServiceConfig ( service, SERVICE_NO_CHANGE, startupType, 0, 0, 0, 0, 0, 0, 0, 0 ) != 0;
    }

    DWORD getStatus () {
        SERVICE_STATUS status;  
        char *statusName;
        if (queryStatus (& status)) {
            return status.dwCurrentState;
        } else {
            return 0;
        }
    }

    static char *getStatusName (DWORD status) {
        char *statusName;
        if (status) {
            switch (status) {
                case SERVICE_CONTINUE_PENDING: statusName = "continue pending"; break;
                case SERVICE_PAUSE_PENDING: statusName = "pause pending"; break;
                case SERVICE_PAUSED: statusName = "paused"; break;
                case SERVICE_RUNNING: statusName = "running"; break;
                case SERVICE_START_PENDING: statusName = "start pending"; break;
                case SERVICE_STOP_PENDING: statusName = "stop pending"; break;
                case SERVICE_STOPPED: statusName = "stopped"; break;
                default: statusName = "unknown";
            }
        } else {
            statusName = "N/A";
        }
        return statusName;
    }

    char *getStatusName () {
        return getStatusName (getStatus ());
    }
};

bool isAdminMode () {
    BOOL isRunAsAdmin = FALSE;
    DWORD error = ERROR_SUCCESS;
    PSID adminGroup = NULL;

    // Allocate and initialize a SID of the administrators group.
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid (&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, & adminGroup))
        error = GetLastError ();

    // Determine whether the SID of administrators group is enabled in 
    // the primary access token of the process.
    if (!error && !CheckTokenMembership (NULL, adminGroup, &isRunAsAdmin))
        error = GetLastError ();

    // Centralized cleanup for all allocated resources.
    if (adminGroup) {
        FreeSid (adminGroup);
        adminGroup = 0;
    }

    // Throw the error if something failed in the function.
    if (ERROR_SUCCESS != error)
        throw error;

    return isRunAsAdmin;
}

bool runAsAdmin (char *cmd, char *params, bool waitForProcess) {
    SHELLEXECUTEINFO info { sizeof (info) };
    info.lpVerb = "runas";
    info.lpFile = cmd;
    info.lpParameters = params;
    info.hwnd = HWND_DESKTOP;
    info.nShow = SW_NORMAL;

    if (waitForProcess) info.fMask = SEE_MASK_NOCLOSEPROCESS;

    bool result;

    if (ShellExecuteEx (& info)) {
        printf ("ok"); 
        
        if (waitForProcess) {
            DWORD exitCode;

            while (GetExitCodeProcess (info.hProcess, & exitCode) && exitCode == STILL_ACTIVE)
                Sleep (100);

            CloseHandle (info.hProcess);
        }
        
        result = true;
    } else {
        printf ("failed, error %d", GetLastError ()); result = false;
    }

    return result;
}

void selfElevate () {
    char path [MAX_PATH], params [1000];
    
    if (GetModuleFileName (0, path, sizeof (path))) {
        strcpy (params, PathGetArgs (GetCommandLine ()));
    
        // Launch itself as admin
        printf ("\nSelf-elevation");

        runAsAdmin (path, params, false);
        exit (0);
    }  
}

void stopGui () {
    DWORD bytesNeeded, *pids = (DWORD *) malloc (1024);
    if (EnumProcesses (pids, 1024, & bytesNeeded) == 0) {
        pids = (DWORD *) realloc (pids, bytesNeeded);
        EnumProcesses (pids, bytesNeeded, & bytesNeeded);
    }

    for (auto i = 0; i < bytesNeeded / sizeof (*pids); ++ i) {
        char path [MAX_PATH];
        auto process = OpenProcess (GENERIC_READ | PROCESS_TERMINATE, 0, pids [i]);

        if (process) {
            GetModuleBaseName (process, 0, path, sizeof (path));
            strlwr (path);
            if (strstr (path, "gui.exe") || strstr (path, "gui-x86.exe")) {
                TerminateProcess (process, 0);
            }
                
            CloseHandle (process);
        }

    }

    free (pids);
}

void getIniPath (char *iniPath, size_t size) {
    GetModuleFileName (0, iniPath, size);
    PathRemoveFileSpec (iniPath);
    PathAppend (iniPath, "updater.ini");
}

void copyFiles (char *destFolder) {
    printf ("\nCopying files...");

    char iniPath [MAX_PATH], fileName [MAX_PATH];
    char key [10];
    int i, index = 1;

    getIniPath (iniPath, sizeof (iniPath));

    while (GetPrivateProfileString ("copy", _itoa (index ++, key, 10), "", fileName, sizeof (fileName), iniPath) > 0) {
        char sourcePath [MAX_PATH], destPath [MAX_PATH];

        GetModuleFileName (0, sourcePath, sizeof (sourcePath));
        PathRemoveFileSpec (sourcePath);
        PathAppend (sourcePath, "files");
        PathAppend (sourcePath, fileName);

        PathCombine (destPath, destFolder, fileName);

        printf ("\n'%s' => '%s': %s", sourcePath, destPath, CopyFile (sourcePath, destPath, 0) ? "ok" : "failed");
    }
}

bool runApp (char *app, char *cmd, uint32_t& error, bool waitWhileRunning) {
    STARTUPINFO startup;
    PROCESS_INFORMATION processInfo;
    memset (& startup, 0, sizeof (startup));
    startup.cb = sizeof (startup);
    memset (& processInfo, 0, sizeof (processInfo));
    
    bool result = CreateProcess (app, cmd, 0, 0, 1, 0, 0, 0, & startup, & processInfo) != 0;

    error = GetLastError ();

    if (result && waitWhileRunning) {
        DWORD exitCode;

        while (GetExitCodeProcess (processInfo.hProcess, & exitCode) && exitCode == STILL_ACTIVE)
            Sleep (100);
    }

    CloseHandle (processInfo.hProcess);
    CloseHandle (processInfo.hThread);

    return result;
}

void runningApps (char *destFolder) {
    printf ("\nRunning apps...");

    char iniPath [MAX_PATH], fileName [MAX_PATH];
    char key [10];
    int i, index = 1;

    getIniPath (iniPath, sizeof (iniPath));

    while (GetPrivateProfileString ("run", _itoa (index ++, key, 10), "", fileName, sizeof (fileName), iniPath) > 0) {
        char destPath [MAX_PATH];
        uint32_t error;

        PathCombine (destPath, destFolder, fileName);

        printf ("\nrunning '%s'...", destPath);
        if (runApp (destPath, 0, error, false))
            printf ("ok");
        else
            printf ("failed, error %d", error);
    }
}

bool selectDestinationFolder (char *path, size_t size) {
    BROWSEINFO browseInfo;
    bool result = false;
    
    memset (& browseInfo, 0, sizeof (browseInfo));
    
    browseInfo.hwndOwner = GetConsoleWindow ();
    browseInfo.lpszTitle = "Выберите папку назначения";
    browseInfo.ulFlags   = BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
    
    auto destFolder = SHBrowseForFolder (& browseInfo);
    
    if (destFolder)
    {
        IMalloc *mallocInterface;
        char     buffer [MAX_PATH];
        
        SHGetPathFromIDList (destFolder, buffer);
        SHGetMalloc (& mallocInterface);
        
        mallocInterface->Free (destFolder);
        mallocInterface->Release ();

        strncpy (path, buffer, size);

        result = true;
    }

    return result;
}

void installDaemon (char *folder) {
    char iniPath [MAX_PATH], daemonPath [MAX_PATH], daemonFile [100], cmd [50];

    printf ("\nInstalling daemon...");
    getIniPath (iniPath, sizeof (iniPath));
    GetPrivateProfileString ("daemon", "file", "daemon.exe", daemonFile, sizeof (daemonFile), iniPath);
    GetPrivateProfileString ("daemon", "cmd", "-i", cmd, sizeof (cmd), iniPath);
    PathCombine (daemonPath, folder, daemonFile);

    if (runAsAdmin (daemonPath, cmd, true)) {
        serviceAccess srvAccess (SERVICE_NAME);
        srvAccess.setStartupType (SERVICE_AUTO_START);
        printf ("ok");
    } else {
        printf ("failed, error %d", GetLastError ());
    }
}

int main (int argCount, char *args []) {
    printf ("OBL update tool v1.0\n");

    if (isAdminMode ()) {
        printf ("\nAlready in admin mode");
    } else {
        printf ("\nElevate itself...");
        selfElevate ();
    }

    printf ("\nRetrieving service name...");
    serviceAccess srvAccess (SERVICE_NAME);
    auto status = srvAccess.getStatus ();
    printf ("\nDaemon installation status: %s", srvAccess.installed () ? "installed" : "not installed");

    char binPath [MAX_PATH];
    if (srvAccess.installed ()) {
        if (srvAccess.getServiceBinaryFolder (binPath, sizeof (binPath))) {
            printf ("\nService binary located at %s", binPath);
        } else {
            exit (0);
        }
    } else if (argCount > 1) {
        strcpy (binPath, args [1]);
    } else if (!selectDestinationFolder (binPath, sizeof (binPath))) {
        exit (0);
    }

    if (srvAccess.installed ()) {
        printf ("\nDaemon status: %s", serviceAccess::getStatusName (status));

        if (status == SERVICE_RUNNING) {
            SERVICE_STATUS srvStatus;
            printf ("\nStopping daemon...");
            srvAccess.stop (& srvStatus);
            printf ("\nWaiting while daemon is being stopped...");
        }

        while (srvAccess.getStatus () != SERVICE_STOPPED) {
            Sleep (100);
        }
    }

    printf ("\nStopping GUI if one is running...");
    stopGui ();

    copyFiles (binPath);

    if (!srvAccess.installed ()) {
        installDaemon (binPath);
        srvAccess.attach (SERVICE_NAME);
    }

    if (srvAccess.installed ()) {
        printf ("\nStarting daemon...");
        srvAccess.start ();
        printf ("\nWaiting while daemon is being started...");

        while (srvAccess.getStatus () != SERVICE_RUNNING) {
            Sleep (100);
        }
    }
    
    runningApps (binPath);
    exit (0);
}
