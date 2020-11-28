#include "resource.h"
#include "main_wnd.h"

int CALLBACK WinMain (HINSTANCE instance, HINSTANCE prevInstance, char *cmdLine, int showCmd)
{
    CMainWnd mainWindow (instance);
    INITCOMMONCONTROLSEX ctlInitData;

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
