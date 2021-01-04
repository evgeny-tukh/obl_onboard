#include <stdlib.h>
#include <Windows.h>
#include <Shlwapi.h>
#include "resource.h"
#include "main_wnd.h"
#include "../common/defs.h"
#include "../common/tools.h"
#include "../common/json.h"
#include "../common/path.h"
#include "../nmea/nmea.h"
#include "wui/StaticWrapper.h"
#include "wui/DateTimePickerWrapper.h"

CMainWnd::CMainWnd (HINSTANCE instance):
    CWindowWrapper (instance, HWND_DESKTOP, "obl_gui", menu = LoadMenu (instance, MAKEINTRESOURCE (IDR_MENU))),
    shipSchema (0),
    navData (0),
    modeSwitch (0),
    bunkerings (0),
    selectedTank (-1),
    viewMode (mode::SCHEMA),
    lat (nmea::NO_VALID_DATA),
    lon (nmea::NO_VALID_DATA),
    cog (nmea::NO_VALID_DATA),
    sog (nmea::NO_VALID_DATA),
    hdg (nmea::NO_VALID_DATA),
    lastDataUpdate (0),
    db (cfg, getPath ())
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
    delete navData;
    delete shipSchema;
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

    navData = new CStaticWrapper (m_hwndHandle, ID_NAV_DATA);
    navData->CreateControl (client.right - 199, 0, 200, 50, SS_LEFT, "POS: N/A\nSOG: N/A\nCOG: N/A");

    shipSchema = new ShipSchema (m_hInstance, m_hwndHandle, db, cfg, history);
    bunkerings = new BunkeringWindow (m_hInstance, m_hwndHandle, cfg, db);

    shipSchema->Create (0, 0, 50, client.right + 1, client.bottom - 49, WS_VISIBLE | WS_CHILD);
    bunkerings->Create (0, 0, 50, client.right + 1, client.bottom - 49, WS_VISIBLE | WS_CHILD);

    switchToMode (mode::SCHEMA);

    SetTimer (1100, 5000);
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
        default:
            if (message == cfg.newDataMsg) {
                shipSchema->onNewData ();
            } else if (message == cfg.posChangedMsg) {
                lastDataUpdate = time (0);
                lat = wParam;
                lon = lParam;
                showNavData ();
            } else if (message == cfg.cogChangedMsg) {
                lastDataUpdate = time (0);
                cog = lParam;
                showNavData ();
            } else if (message == cfg.sogChangedMsg) {
                lastDataUpdate = time (0);
                sog = lParam;
                showNavData ();
            }
    }

    return CWindowWrapper::OnMessage (message, wParam, lParam);
}

void CMainWnd::exportLevels () {
    database::valueMap tankLevels, meterValues;

    logbookRecord record;

    db.collectCurrentVolumes (tankLevels);
    db.collectCurrentMeters (meterValues);
    db.getRecentLogbookRecord (record);

    json::hashNode root, volumes, meters, data, logbook;
    char buffer [50];

    for (auto& tankLevel: tankLevels) {
        auto value = new json::numberNode (ftoa (tankLevel.second.value, buffer, "%f"));
        volumes.add (_itoa (tankLevel.first, buffer, 10), value);
    }

    for (auto& meterValue: meterValues) {
        auto value = new json::numberNode (ftoa (meterValue.second.value, buffer, "%f"));
        meters.add (_itoa (meterValue.first, buffer, 10), value);
    }

    logbook.add ("timestamp", new json::numberNode (record.timestamp));
    logbook.add ("lat", record.lat.second ? new json::numberNode (record.lat.first) : new json::node);
    logbook.add ("lon", record.lon.second ? new json::numberNode (record.lon.first) : new json::node);
    logbook.add ("sog", record.sog.second ? new json::numberNode (record.sog.first) : new json::node);
    logbook.add ("cog", record.cog.second ? new json::numberNode (record.cog.first) : new json::node);
    logbook.add ("hdg", record.hdg.second ? new json::numberNode (record.hdg.first) : new json::node);

    data.add ("volumes", & volumes);
    data.add ("meters", & meters);
    root.add ("type", new json::stringNode ("values"));
    root.add ("data", & data);
    root.add ("logbook", & logbook);

    exportJson (root, cfg);
}

LRESULT CMainWnd::OnCommand (WPARAM wParam, LPARAM lParam) {
    LRESULT result = FALSE;

    switch (LOWORD (wParam)) {
        case ID_NEW_BUNKERING: {
            modeSwitch->SetCurSel (1);
            switchToMode (mode::BUNKERINGS);
            bunkerings->SendMessage (WM_COMMAND, wParam, lParam);
            break;
        }
        case ID_EDIT_BUNKERING:
        case ID_DELETE_BUNKERING: {
            if (modeSwitch->GetCurSel () == 1) bunkerings->SendMessage (WM_COMMAND, wParam, lParam);
            break;
        }
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
                    InvalidateRect (shipSchema->GetHandle (), 0, FALSE);
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
    modeSwitch->Move (0, 0, width - 200, 50, TRUE);
    shipSchema->Move (0, 50, width, height - 49, TRUE);
    bunkerings->Move (0, 50, width, height - 49, TRUE);
    navData->Move (width - 200, 0, 200, 50, TRUE);

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
    if (timerID == 1100) {
        if ((time (0) - lastDataUpdate) > cfg.timeout) {
            lat = nmea::NO_VALID_DATA;
            lon = nmea::NO_VALID_DATA;
            sog = nmea::NO_VALID_DATA;
            cog = nmea::NO_VALID_DATA;
            hdg = nmea::NO_VALID_DATA;

            showNavData ();
        }
    }

    return CWindowWrapper::OnTimer (timerID);
}

void CMainWnd::showNavData () {
    std::string result;
    char buffer [100];

    result = "POS: ";
    if (lat == nmea::NO_VALID_DATA || lon == nmea::NO_VALID_DATA)
        result += "N/A";
    else
        result += nmea::formatPos ((double) lat / 60000.0, (double) lon / 60000.0, buffer);

    result += "\nSOG: ";
    if (sog == nmea::NO_VALID_DATA)
        result += "N/A";
    else
        result += ftoa ((double) sog * 0.1, buffer, "%.1f");

    result += "\nCOG: ";
    if (cog == nmea::NO_VALID_DATA)
        result += "N/A";
    else
        result += ftoa ((double) cog * 0.1, buffer, "%05.1f");

    navData->SetText (result.c_str ());
}