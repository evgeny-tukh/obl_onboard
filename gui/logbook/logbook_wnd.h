#pragma once

#include <vector>

#include "../../common/defs.h"
#include "../../common/db.h"
#include "../wui/WindowWrapper.h"
#include "../wui/GenericControlWrapper.h"
#include "../wui/ListCtrlWrapper.h"
#include "../wui/TabCtrlWrapper.h"
#include "../wui/ButtonWrapper.h"
#include "../wui/EditWrapper.h"
#include "../wui/TrackbarWrapper.h"
#include "../wui/DateTimePickerWrapper.h"
#include "../wui/ListBoxWrapper.h"

class LogbookWindow: public CWindowWrapper {
    public:
        LogbookWindow (HINSTANCE instance, HWND parent, config& _cfg, database& _db);
        virtual ~LogbookWindow ();
        
    private:
        config& cfg;
        database& db;
        CTabCtrlWrapper *tabSwitch;
        
        virtual void OnCreate ();
        virtual LRESULT OnCommand (WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnSize (const DWORD requestType, const WORD width, const WORD height);
        virtual LRESULT OnNotify (NMHDR *pHeader);
};
