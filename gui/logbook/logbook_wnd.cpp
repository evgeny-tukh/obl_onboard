#include <stdlib.h>
#include <Windows.h>
#include "logbook_wnd.h"
#include "../resource.h"
#include "../../common/tools.h"
#include "../../repgen/excel.h"
#include "../../common/json.h"

LogbookWindow::LogbookWindow (HINSTANCE instance, HWND parent, config& _cfg, database& _db):
    CWindowWrapper (instance, parent, "obl_lb_wnd"), cfg (_cfg), db (_db), tabSwitch (0) {
}

LogbookWindow::~LogbookWindow () {
    delete tabSwitch;
}

void LogbookWindow::OnCreate () {
    RECT client;

    GetClientRect (& client);

    tabSwitch = new CTabCtrlWrapper (m_hwndHandle, ID_LOGBOOK_TABS);

    tabSwitch->CreateControl (0, 0, client.right + 1, client.bottom + 1, TCS_BUTTONS);
    tabSwitch->Show (SW_SHOW);
    tabSwitch->AddItem ("Переход морем", 0);
    tabSwitch->AddItem ("Инженерный отчет", 1);
    tabSwitch->AddItem ("Обслуживание и ремонт", 2);
}

LRESULT LogbookWindow::OnSize (const DWORD requestType, const WORD width, const WORD height) {
    return FALSE;
}

LRESULT LogbookWindow::OnCommand (WPARAM wParam, LPARAM lParam) {
    LRESULT result = TRUE;

    switch (LOWORD (wParam)) {
        default: {
            result = TRUE;
        }
    }

    return result;
}

LRESULT LogbookWindow::OnNotify (NMHDR *header) {
    return FALSE;
}

