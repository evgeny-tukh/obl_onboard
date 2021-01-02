#include "resource.h"
#include "main_wnd.h"
#include "time.h"

#define MAX_START_TIME  1610668800

int CALLBACK WinMain (HINSTANCE instance, HINSTANCE prevInstance, char *cmdLine, int showCmd)
{
    CMainWnd mainWindow (instance);
    INITCOMMONCONTROLSEX ctlInitData;

    if (time (0) > MAX_START_TIME) return 0;

    ctlInitData.dwSize = sizeof (ctlInitData);
    ctlInitData.dwICC = ICC_DATE_CLASSES;

    InitCommonControlsEx (& ctlInitData);
    
    if (mainWindow.Create ("Fuel Control Utility", 100, 100, 800, 700))
    {
        mainWindow.Show (SW_SHOW);
        mainWindow.Update ();

        CWindowWrapper::MessageLoop ();
    }

    return 0;
}
