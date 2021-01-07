#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <winsvc.h>
#include <Shlwapi.h>
#include <Psapi.h>
#include <cstdint>

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

void getIniPath (char *iniPath) {
    GetModuleFileName (0, iniPath, sizeof (iniPath));
    PathRemoveFileSpec (iniPath);
    PathAppend (iniPath, "updater.ini");
}

void copyFiles (char *destFolder) {
    printf ("\nCopying files...");

    char iniPath [MAX_PATH], fileName [MAX_PATH];
    char key [10];
    int i, index = 1;

    getIniPath (iniPath);

    while (GetPrivateProfileString ("copy", _itoa (index ++, key, 10), "", fileName, sizeof (fileName), iniPath) > 0) {
        char sourcePath [MAX_PATH], destPath [MAX_PATH];

        GetModuleFileName (0, sourcePath, sizeof (sourcePath));
        PathRemoveFileSpec (sourcePath);
        PathAppend (sourcePath, "files");
        PathAppend (sourcePath, fileName);

        PathCombine (destPath, destFolder, fileName);

        printf ("\n'%s' => '%s'", sourcePath, destPath);
    }
}

bool runApp (char *app, char *cmd, uint32_t& error, bool waitWhileRunning) {
    STARTUPINFO startup;
    PROCESS_INFORMATION processInfo;
    memset (& startup, 0, sizeof (startup));
    startup.cb = sizeof (startup);
    memset (& processInfo, 0, sizeof (processInfo));
    
    bool result = CreateProcess (app, cmd, 0, 0, 0, 0, 0, 0, & startup, & processInfo) != 0;

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

    getIniPath (iniPath);

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

void installDaemon () {
    char iniPath [MAX_PATH], daemonPath [MAX_PATH], daemonFile [100], cmd [50];
    uint32_t error;

    getIniPath (iniPath);
    GetPrivateProfileString ("daemon", "file", "daemon.exe", daemonFile, sizeof (daemonFile), iniPath);
    GetPrivateProfileString ("daemon", "cmd", "-i", cmd, sizeof (cmd), iniPath);
    
    OPENFILENAME ofn;
    char file [MAX_PATH];

    memset (& ofn, 0, sizeof (ofn));
    memset (file, 0, sizeof (file));

    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.hwndOwner = GetConsoleWindow ();
    ofn.hInstance = (HINSTANCE) GetWindowLongPtr (ofn.hwndOwner, GWLP_HINSTANCE);
    ofn.lpstrFile = file;
    ofn.lpstrFilter = daemonFile;
    ofn.lpstrTitle = "Select daemon binary file";
    ofn.lStructSize = sizeof (ofn);
    ofn.nMaxFile = sizeof (file);

    if (GetOpenFileName (& ofn)) {
        printf ("\nInstalling daemon...");

        if (runApp (file, cmd, error, true)) {
            printf ("ok");
        } else {
            printf ("failed, error %d", error);
        }
    }
}

int main (int argCount, char *args []) {
    printf ("OBL update tool v1.0\n");

    bool installDaemonMode = argCount > 1 && stricmp (args [1], "install") == 0;

    printf ("\nRetrieving service name...");
    serviceAccess srvAccess (SERVICE_NAME);
    auto status = srvAccess.getStatus ();
    printf ("\nDaemon installation status: %s", srvAccess.installed () ? "installed" : "not installed");
    printf ("\nDaemon status: %s", serviceAccess::getStatusName (status));

    char binPath [MAX_PATH];
    if (srvAccess.getServiceBinaryFolder (binPath, sizeof (binPath))) {
        printf ("\nService binary located at %s", binPath);
    }

    if (status == SERVICE_RUNNING) {
        SERVICE_STATUS srvStatus;
        printf ("\nStopping daemon...");
        srvAccess.stop (& srvStatus);
        printf ("\nWaiting while daemon is being stopped...");
    }

    while (srvAccess.getStatus () != SERVICE_STOPPED) {
        Sleep (100);
    }

    printf ("\nStopping GUI if one is running...");
    stopGui ();

    copyFiles (binPath);

    if (!srvAccess.installed () || installDaemonMode) {
        installDaemon ();
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
