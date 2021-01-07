#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <shlwapi.h>
#include <functional>

auto const SERVICE_NAME = "Obl---Daemon";

struct serviceData {
    SERVICE_STATUS status;
    SERVICE_STATUS_HANDLE handle;
} srvData;

void stopPoller ();
void doNormalWork ();

void log (char *msg) {
    char path [MAX_PATH];

    GetModuleFileName (0, path, sizeof (path));
    PathRenameExtension (path, ".log");

    auto logFile = CreateFile (path, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);

    if (logFile != INVALID_HANDLE_VALUE) {
        DWORD bytesWritten;

        SetFilePointer (logFile, 0, 0, SEEK_END);
        WriteFile (logFile, msg, strlen (msg), & bytesWritten, 0);
        WriteFile (logFile, "\r\n", 2, & bytesWritten, 0);
        CloseHandle (logFile);
    }
}

void setServiceStatus (unsigned long currentState, unsigned long exitCode = NO_ERROR, unsigned long waitHint = 0) {
    static unsigned long checkPoint = 1;

    srvData.status.dwCurrentState  = currentState;
    srvData.status.dwWin32ExitCode = exitCode;
    srvData.status.dwWaitHint      = waitHint;
    srvData.status.dwCheckPoint    = ((currentState == SERVICE_RUNNING) || (currentState == SERVICE_STOPPED)) ? 0 : checkPoint ++;

    ::SetServiceStatus (srvData.handle, & srvData.status);
}

int installService () {
    SC_HANDLE scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    DWORD error;
  
    if (!scmHandle) {
        printf ("Unable to connect service control manager, error code %d\n", GetLastError ()); return -1;
    }
  
    char path [MAX_PATH];

    GetModuleFileName (0, path, sizeof (path));

    SC_HANDLE srvHandle = CreateService (
        scmHandle,
        SERVICE_NAME,
        SERVICE_NAME,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        path,
        0,
        0,
        0,
        0,
        0
    );

    if (!srvHandle)
        printf ("Unable to create service, error code %d\n", GetLastError ());

    CloseServiceHandle (scmHandle);

    if (srvHandle) {
        CloseServiceHandle (srvHandle); return 0;
    } else {
        return -1;
    }
}

int removeService () {
    SC_HANDLE scmHandle = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS);
  
    if (!scmHandle) {
        printf ("Unable to connect SCM, error %d\n", GetLastError ()); return -1;
    }

    SC_HANDLE srvHandle = OpenService (scmHandle, SERVICE_NAME, SERVICE_STOP | DELETE);
  
    if (!srvHandle) {
        printf ("Unable to open service, error %d\n", GetLastError ());
        CloseServiceHandle (scmHandle); return -1;
    }
  
    if (DeleteService (srvHandle)) {
        printf ("Service has been uninstalled.\n");
    } else {
        printf ("Unable to uninstall service, error %d\n", GetLastError ());
    }

    CloseServiceHandle (srvHandle);
    CloseServiceHandle (scmHandle);
    return 0;
}

int startService () {
    SC_HANDLE scmHandle = OpenSCManager (NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    SC_HANDLE srvHandle = scmHandle ? OpenService (scmHandle, SERVICE_NAME, SERVICE_START) : 0;
  
    if (!StartService (srvHandle, 0, NULL)) {
        printf ("Unable to start service, error %d\n", GetLastError ());
        CloseServiceHandle (scmHandle);
        return -1;
    } else {
        printf ("Service has been started.\n");
    }
  
    if (srvHandle) CloseServiceHandle (srvHandle);
    if (scmHandle) CloseServiceHandle (scmHandle);

    return 0;
}

int stopService () {
    SC_HANDLE scmHandle = OpenSCManager (NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    SC_HANDLE srvHandle = scmHandle ? OpenService (scmHandle, SERVICE_NAME, SERVICE_STOP) : 0;
    
    if (ControlService (srvHandle, SERVICE_CONTROL_STOP, & srvData.status))
        printf ("Service has been stopped\n");
    else
        printf ("Unable to stop service, error %d\n", GetLastError ());

    if (srvHandle) CloseServiceHandle (srvHandle);
    if (scmHandle) CloseServiceHandle (scmHandle);
    
    return 0;
}

void controlHandler (DWORD request) {
    switch (request) {
        case SERVICE_CONTROL_STOP: {
            stopPoller ();
            setServiceStatus (SERVICE_STOPPED); return;
        }
        case SERVICE_CONTROL_SHUTDOWN: {
            setServiceStatus (SERVICE_STOPPED); return;
        }
    }

    setServiceStatus (SERVICE_RUNNING);
}

bool initService () {
    return true;
}

void serviceMain (int argc, char** argv) {
    srvData.status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    srvData.status.dwCurrentState = SERVICE_START_PENDING;
    srvData.status.dwControlsAccepted = SERVICE_ACCEPT_STOP /*| SERVICE_ACCEPT_SHUTDOWN*/;
    srvData.status.dwWin32ExitCode = NO_ERROR;
    srvData.status.dwServiceSpecificExitCode = 0;
    srvData.status.dwCheckPoint = 0;
    srvData.status.dwWaitHint = 0;

    srvData.handle = RegisterServiceCtrlHandler  (SERVICE_NAME, (LPHANDLER_FUNCTION) controlHandler);

    if (!srvData.handle) {
        setServiceStatus (SERVICE_STOPPED, -1); return;
    }

    setServiceStatus (SERVICE_START_PENDING, NO_ERROR, 3000);

    if (!initService ()) {
        setServiceStatus (SERVICE_STOPPED, -1);
    } else {
        setServiceStatus (SERVICE_RUNNING);
    }

    doNormalWork ();
}

void runService () {
    SERVICE_TABLE_ENTRY serviceTable [] {
        { (char *) SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) serviceMain },
        { 0, 0 },
    };

    StartServiceCtrlDispatcher (serviceTable);
}

#if 0

void start ();
void stopAllReaders ();

class oblService : public service {
    public:
        oblService () : service (SERVICE_NAME, SERVICE_NAME " service"), daemonThread (0), keepRunning (false) {}
        virtual ~oblService ();

    protected:
        bool keepRunning;
        std::thread *daemonThread;

        static void daemonProc (oblService *);
        void daemonProcInternal ();

        virtual void onStart (unsigned int numOgArgs = 0, const char **args = 0);
        virtual void onStop ();
};

oblService::~oblService () {
    if (daemonThread) {
        if (daemonThread->joinable ()) daemonThread->join ();

        delete daemonThread;
    }
}

void oblService::daemonProc (oblService *instance) {
    if (instance) instance->daemonProcInternal ();
}

void oblService::daemonProcInternal () {
    keepRunning = true;

    while (keepRunning) {
        std::this_thread::sleep_for (std::chrono::seconds (1));
    }
}

void oblService::onStart (unsigned int numOgArgs, const char **args) {
    writeErrorLogEntry ("onStart called", 0);
    daemonThread = new std::thread (daemonProc, this);
}

void oblService::onStop () {
    if (daemonThread && daemonThread->joinable ()) {
        keepRunning = false;

        daemonThread->detach ();
    }
}

void installService () {
    oblService service;
    service.install (true);
}

void uninstallService () {
    oblService service;
    service.uninstall (true);
}

void startService () {
    oblService service;
    service.startStop (true);
}

void stopService () {
    oblService service;
    service.startStop (false);
}

void runService() {
    oblService service;
    service.run ();
}
#endif