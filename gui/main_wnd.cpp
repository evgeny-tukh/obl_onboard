#include <stdlib.h>
#include <Windows.h>
#include <Shlwapi.h>
#include "resource.h"
#include "main_wnd.h"
#include "../common/defs.h"
#include "../common/tools.h"
#include "../common/json.h"
#include "wui/StaticWrapper.h"
#include "wui/DateTimePickerWrapper.h"

CMainWnd::CMainWnd (HINSTANCE instance):
    CWindowWrapper (instance, HWND_DESKTOP, "obl_gui", menu = LoadMenu (instance, MAKEINTRESOURCE (IDR_MENU))),
    shipSchema (0),
    bunkerings (0),
    selectedTank (-1),
    viewMode (mode::SCHEMA),
    db (cfg)
{
    char path [MAX_PATH];

    GetModuleFileNameA (0, path, sizeof (path));
    PathRemoveFileSpecA (path);
    PathAppendA (path, "cfg.dat");

    cfg.cfgFile = path;

    parseCfgFile (cfg);

    history = new dataHistory (db, cfg);

    beginTimestamp = history->minTime ();
    endTimestamp = time (0);
}

CMainWnd::~CMainWnd () {
    delete modeSwitch;
    delete bunkerings;
}

void CMainWnd::switchToMode (mode newMode) {
    viewMode = newMode;

    shipSchema->Show (viewMode == mode::SCHEMA ? SW_SHOW : SW_HIDE);
    shipSchema->Update ();
    bunkerings->Show (viewMode == mode::BUNKERINGS ? SW_SHOW : SW_HIDE);
    bunkerings->Update ();
}

void CMainWnd::OnCreate () {
    RECT client;
    char buffer [100];

    GetClientRect (& client);

    modeSwitch = new CTabCtrlWrapper (m_hwndHandle, ID_MODE_SWITCH);

    modeSwitch->CreateControl (0, 0, client.right - 199, 50, WS_VISIBLE | TCS_BUTTONS, 0);
    modeSwitch->AddItem ("Мнемосхема", mode::SCHEMA);
    modeSwitch->AddItem ("Бункеровки", mode::BUNKERINGS);

    shipSchema = new ShipSchema (m_hInstance, m_hwndHandle, cfg, history);
    bunkerings = new BunkeringWindow (m_hInstance, m_hwndHandle, cfg, db);

    shipSchema->Create (0, 0, 50, client.right + 1, client.bottom - 49, WS_VISIBLE | WS_CHILD);
    bunkerings->Create (0, 0, 50, client.right + 1, client.bottom - 49, WS_VISIBLE | WS_CHILD);

    switchToMode (mode::SCHEMA);
}

void CMainWnd::RequestAppQuit () {
    if (MessageBox ("Выйти из приложения?", "Нужно подтверждение", MB_YESNO | MB_ICONQUESTION) == IDYES) Destroy ();
}

LRESULT CMainWnd::OnMessage (UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_DESTROY:
            DestroyMenu (menu);
            PostQuitMessage (0);

            break;
    }

    return CWindowWrapper::OnMessage (message, wParam, lParam);
}

void CMainWnd::exportLevels () {
    database::valueMap tankLevels, meterValues;

    db.collectCurrentVolumes (tankLevels);
    db.collectCurrentMeters (meterValues);

    json::hashNode root, volumes, meters, data;
    char buffer [50];

    for (auto& tankLevel: tankLevels) {
        auto value = new json::numberNode (ftoa (tankLevel.second, buffer, "%f"));
        volumes.add (_itoa (tankLevel.first, buffer, 10), value);
    }

    for (auto& meterValue: meterValues) {
        auto value = new json::numberNode (ftoa (meterValue.second, buffer, "%f"));
        meters.add (_itoa (meterValue.first, buffer, 10), value);
    }

    data.add ("volumes", & volumes);
    data.add ("meters", & meters);
    root.add ("type", new json::stringNode ("values"));
    root.add ("data", & data);

    exportJson (root, cfg);
}

LRESULT CMainWnd::OnCommand (WPARAM wParam, LPARAM lParam) {
    LRESULT result = FALSE;

    switch (LOWORD (wParam)) {
        case ID_EXPORT_LEVELS: {
            exportLevels ();
            MessageBox ("Данные экспортированы", "Информация", MB_ICONINFORMATION);
            break;
        }
        case ID_TANK_SELECTOR: {
            if (HIWORD (wParam) == CBN_SELCHANGE ) {
                int selection = tankSelector->GetCurSel ();

                if (selection >= 0)
                {
                    selectedTank = tankSelector->GetItemData (selection);

                    shipSchema->selectTank (selectedTank);
                    InvalidateRect (shipSchema->GetHandle (), 0, TRUE);
                    //loadBunkeringList ();
                }
            }
            break;
        }
        case ID_EXIT: {
            RequestAppQuit (); break;
        }
        default: {
            result = TRUE;
        }
    }

    return result;
}

LRESULT CMainWnd::OnSysCommand (WPARAM wParam, LPARAM lParam) {
    LRESULT result;

    switch (wParam) {
        case SC_CLOSE:
            RequestAppQuit();

            result = FALSE; break;

        default:
            result = CWindowWrapper::OnSysCommand (wParam, lParam);
    }

    return result;
}

LRESULT CMainWnd::OnSize (const DWORD requestType, const WORD width, const WORD height) {
    modeSwitch->Move (0, 0, width, 50, TRUE);
    shipSchema->Move (0, 50, width, height - 49, TRUE);
    bunkerings->Move (0, 50, width, height - 49, TRUE);

    return FALSE;
}

LRESULT CMainWnd::OnNotify (NMHDR *header) {
    switch (header->idFrom) {
        case ID_MODE_SWITCH: {
            auto selection = modeSwitch->GetCurSel ();

            switchToMode ((mode) modeSwitch->GetItemData (selection)); break;
        }
    }
    return FALSE;
}

LRESULT CMainWnd::OnTimer (unsigned int timerID) {
    return CWindowWrapper::OnTimer (timerID);
}
