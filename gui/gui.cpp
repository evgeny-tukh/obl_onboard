#include "resource.h"
#include "main_wnd.h"

int CALLBACK WinMain (HINSTANCE instance, HINSTANCE prevInstance, char *cmdLine, int showCmd)
{
    CMainWnd mainWindow (instance);

    if (mainWindow.Create ("Fuel Control Utility", 100, 100, 800, 600))
    {
        mainWindow.Show (SW_SHOW);
        mainWindow.Update ();

        CWindowWrapper::MessageLoop ();
    }

    return 0;
}
