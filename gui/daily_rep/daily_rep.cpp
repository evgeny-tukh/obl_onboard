#include <stdlib.h>
#include <Windows.h>
#include "daily_rep.h"
#include "../resource.h"
#include "../../common/tools.h"
#include "../../repgen/excel.h"
#include "../../common/json.h"

DailyReportWindow::DailyReportWindow (HINSTANCE instance, HWND parent, config& _cfg, database& _db):
    CWindowWrapper (instance, parent, "obl_lb_wnd"), cfg (_cfg), db (_db) {
}

DailyReportWindow::~DailyReportWindow () {
}

void DailyReportWindow::OnCreate () {
    RECT client;

    GetClientRect (& client);
}

LRESULT DailyReportWindow::OnSize (const DWORD requestType, const WORD width, const WORD height) {
    return FALSE;
}

LRESULT DailyReportWindow::OnCommand (WPARAM wParam, LPARAM lParam) {
    LRESULT result = TRUE;

    switch (LOWORD (wParam)) {
        default: {
            result = TRUE;
        }
    }

    return result;
}

LRESULT DailyReportWindow::OnNotify (NMHDR *header) {
    return FALSE;
}

