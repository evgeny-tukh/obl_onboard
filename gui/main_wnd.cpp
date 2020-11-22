#include "resource.h"
#include "main_wnd.h"

CMainWnd::CMainWnd (HINSTANCE instance): CWindowWrapper (instance, HWND_DESKTOP, "obl_gui", LoadMenu (instance, MAKEINTRESOURCE (IDR_MENU)))
{
}

CMainWnd::~CMainWnd ()
{
}

void CMainWnd::Initialize()
{
    RECT client;

    GetClientRect (& client);
}

void CMainWnd::RequestAppQuit ()
{
    if (MessageBox ("Выйти из приложения?", "Нужно подтверждение", MB_YESNO | MB_ICONQUESTION) == IDYES) Destroy ();
}

LRESULT CMainWnd::OnMessage (UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
            Initialize (); break;

        case WM_DESTROY:
            //DestroyMenu (menu);
            PostQuitMessage (0);

            break;
    }

    return CWindowWrapper::OnMessage (message, wParam, lParam);
}

LRESULT CMainWnd::OnCommand (WPARAM wParam, LPARAM lParam)
{
    LRESULT result = FALSE;

    switch (LOWORD (wParam))
    {
        case ID_EXIT:
            RequestAppQuit (); break;

        default:
            result = TRUE;
    }

    return result;
}

LRESULT CMainWnd::OnSysCommand (WPARAM wParam, LPARAM lParam)
{
    LRESULT result;

    switch (wParam)
    {
        case SC_CLOSE:
            RequestAppQuit();

            result = FALSE; break;

        default:
            result = CWindowWrapper::OnSysCommand (wParam, lParam);
    }

    return result;
}

LRESULT CMainWnd::OnSize (const DWORD requestType, const WORD width, const WORD height)
{
    return FALSE;
}

LRESULT CMainWnd::OnNotify (NMHDR *header)
{
    return FALSE;
}

LRESULT CMainWnd::OnTimer (unsigned int timerID)
{
    return CWindowWrapper::OnTimer (timerID);
}
