#include <Shlwapi.h>
#include "resource.h"
#include "main_wnd.h"
#include "../common/defs.h"
#include "wui/StaticWrapper.h"

CMainWnd::CMainWnd (HINSTANCE instance):
    CWindowWrapper (instance, HWND_DESKTOP, "obl_gui", menu = LoadMenu (instance, MAKEINTRESOURCE (IDR_MENU))),
    shipSchema (0),
    selectedTank (-1)
{
    char path [MAX_PATH];

    GetModuleFileNameA (0, path, sizeof (path));
    PathRemoveFileSpecA (path);
    PathAppendA (path, "cfg.dat");

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

    tankSelector = new CComboBoxWrapper (m_hwndHandle, ID_TANK_SELECTOR);

    tankSelector->CreateControl (SHIP_SCHEMA_WIDTH + 80, 0, 200, 100, CBS_AUTOHSCROLL | CBS_DROPDOWNLIST | WS_VISIBLE);

    for (auto iter = cfg.tanks.begin (); iter != cfg.tanks.end (); ++ iter) {
        tankSelector->AddString ((iter->name + " " + iter->type).c_str (), iter->id);
    }

    auto tankLabel = new CStaticWrapper (m_hwndHandle, -1);

    tankLabel->CreateControl (SHIP_SCHEMA_WIDTH + 5, 0, 75, 25, SS_LEFT | WS_VISIBLE, "Танк");
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
        case ID_TANK_SELECTOR:
        {
            int selection = tankSelector->GetCurSel ();

            if (selection >= 0)
            {
                selectedTank = tankSelector->GetItemData (selection);

                shipSchema->selectTank (selectedTank);
                
                InvalidateRect (shipSchema->GetHandle (), 0, TRUE);
            }

            break;
        }
        case ID_EXIT:
        {
            RequestAppQuit (); break;
        }
        default:
        {
            result = TRUE;
        }
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
