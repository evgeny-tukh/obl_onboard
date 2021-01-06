#include <stdlib.h>
#include <Windows.h>
#include "logbook_wnd.h"
#include "../resource.h"
#include "../../common/tools.h"
#include "../../repgen/excel.h"
#include "../../common/json.h"

LogbookWindow::LogbookWindow (HINSTANCE instance, HWND parent, config& _cfg, database& _db):
    CWindowWrapper (instance, parent, "obl_lb_wnd"), cfg (_cfg), db (_db) {
}

LogbookWindow::~LogbookWindow () {
}

void LogbookWindow::OnCreate () {
    RECT client;

    GetClientRect (& client);
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

