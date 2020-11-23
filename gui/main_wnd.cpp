#include <Shlwapi.h>
#include "resource.h"
#include "main_wnd.h"
#include "../common/defs.h"
#include "wui/StaticWrapper.h"
#include "wui/DateTimePickerWrapper.h"

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
    delete shipSchema;
    delete tankSelector;
    delete tankLabel, beginLabel, endLabel;
    delete beginDate, endDate, beginTime, endTime;
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
    beginDate = new CDateTimePickerWrapper (m_hwndHandle, ID_BEGIN_DATE);
    endDate = new CDateTimePickerWrapper (m_hwndHandle, ID_END_DATE);
    beginTime = new CDateTimePickerWrapper (m_hwndHandle, ID_BEGIN_TIME);
    endTime = new CDateTimePickerWrapper (m_hwndHandle, ID_END_TIME);

    tankSelector->CreateControl (SHIP_SCHEMA_WIDTH + 80, 0, 200, 100, CBS_AUTOHSCROLL | CBS_DROPDOWNLIST | WS_VISIBLE);
    beginDate->CreateControl (SHIP_SCHEMA_WIDTH + 80, 25, 100, 25, DTS_SHORTDATECENTURYFORMAT | DTS_UPDOWN | WS_VISIBLE, 0);
    endDate->CreateControl (SHIP_SCHEMA_WIDTH + 80, 50, 100, 25, DTS_SHORTDATECENTURYFORMAT | DTS_UPDOWN | WS_VISIBLE, 0);
    beginTime->CreateControl (SHIP_SCHEMA_WIDTH + 180, 25, 100, 25, DTS_TIMEFORMAT | DTS_UPDOWN | WS_VISIBLE, 0);
    endTime->CreateControl (SHIP_SCHEMA_WIDTH + 180, 50, 100, 25, DTS_TIMEFORMAT | DTS_UPDOWN | WS_VISIBLE, 0);

    for (auto iter = cfg.tanks.begin (); iter != cfg.tanks.end (); ++ iter) {
        tankSelector->AddString ((iter->name + " " + iter->type).c_str (), iter->id);
    }

    tankLabel = new CStaticWrapper (m_hwndHandle, IDC_STATIC);
    beginLabel = new CStaticWrapper (m_hwndHandle, IDC_STATIC);
    endLabel = new CStaticWrapper (m_hwndHandle, IDC_STATIC);

    tankLabel->CreateControl (SHIP_SCHEMA_WIDTH + 5, 0, 75, 25, SS_LEFT | WS_VISIBLE, "Танк");
    beginLabel->CreateControl (SHIP_SCHEMA_WIDTH + 5, 25, 75, 25, SS_LEFT | WS_VISIBLE, "Начало");
    endLabel->CreateControl (SHIP_SCHEMA_WIDTH + 5, 50, 75, 25, SS_LEFT | WS_VISIBLE, "Конец");
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
