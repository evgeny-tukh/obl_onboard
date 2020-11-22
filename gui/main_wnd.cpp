#include <Shlwapi.h>
#include "resource.h"
#include "main_wnd.h"
#include "../common/defs.h"

CMainWnd::CMainWnd (HINSTANCE instance): CWindowWrapper (instance, HWND_DESKTOP, "obl_gui", menu = LoadMenu (instance, MAKEINTRESOURCE (IDR_MENU))), shipSchema (0)
{
    char path [MAX_PATH];

    GetModuleFileName (0, path, sizeof (path));
    PathRemoveFileSpec (path);
    PathAppend (path, "cfg.dat");

    cfg.cfgFile = path;

    parseCfgFile (cfg);
}

CMainWnd::~CMainWnd ()
{
}

void CMainWnd::OnCreate ()
{
    RECT client;

    GetClientRect (& client);

    shipSchema = new ShipSchema (m_hInstance, m_hwndHandle, cfg);

    shipSchema->Create (0, 0, 0, SHIP_SCHEMA_WIDTH, client.bottom + 1, WS_VISIBLE | WS_CHILD);
    shipSchema->Show (SW_SHOW);
    shipSchema->Update ();
}

void CMainWnd::RequestAppQuit ()
{
    if (MessageBox ("Выйти из приложения?", "Нужно подтверждение", MB_YESNO | MB_ICONQUESTION) == IDYES) Destroy ();
}

LRESULT CMainWnd::OnMessage (UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_DESTROY:
            DestroyMenu (menu);
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
    shipSchema->Move (0, 0, SHIP_SCHEMA_WIDTH, height, TRUE);

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
