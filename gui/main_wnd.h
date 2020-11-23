#pragma once

#include <Windows.h>
#include <Shlwapi.h>
#include <commctrl.h>
#include "wui/WindowWrapper.h"
#include "wui/ListCtrlWrapper.h"
#include "wui/TreeCtrlWrapper.h"
#include "wui/ListBoxWrapper.h"
#include "wui/ComboBoxWrapper.h"
#include "wui/StaticWrapper.h"
#include "wui/InputBox.h"
#include "wui/DateTimePickerWrapper.h"
#include "wui/TrackbarWrapper.h"
#include "schema/ship_schema.h"
#include "../common/defs.h"

class CMainWnd : public CWindowWrapper
{
    public:
        CMainWnd (HINSTANCE instance);
        virtual ~CMainWnd ();

    protected:
        static const int SHIP_SCHEMA_WIDTH = 300;
        
        ShipSchema             *shipSchema;
        CComboBoxWrapper       *tankSelector;
        CStaticWrapper         *tankLabel, *beginLabel, *endLabel;
        CDateTimePickerWrapper *beginDate, *endDate, *beginTime, *endTime;
        CTrackbarWrapper       *timeSelector;
        HMENU                   menu;
        config                  cfg;
        int                     selectedTank;

        virtual LRESULT OnMessage (UINT message, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnCommand (WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnSysCommand (WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnSize (const DWORD requestType, const WORD width, const WORD height);
        virtual LRESULT OnTimer (unsigned int timerID);
        virtual LRESULT OnNotify (NMHDR *pHeader);

        void RequestAppQuit ();

        virtual void OnCreate ();
};
