#pragma once

#include <Windows.h>
#include <Shlwapi.h>
#include <commctrl.h>
#include "wui/WindowWrapper.h"
#include "wui/ListCtrlWrapper.h"
#include "wui/TreeCtrlWrapper.h"
#include "wui/TabCtrlWrapper.h"
#include "wui/ListBoxWrapper.h"
#include "wui/ComboBoxWrapper.h"
#include "wui/StaticWrapper.h"
#include "wui/InputBox.h"
#include "wui/DateTimePickerWrapper.h"
#include "wui/TrackbarWrapper.h"
#include "schema/ship_schema.h"
#include "../common/defs.h"
#include "../common/db.h"
#include "bunkering/bunkering_wnd.h"
#include "bunkering/bunkering_edit.h"
#include "logbook/logbook_wnd.h"
#include "daily_rep/daily_rep.h"
#include "data_history.h"
#include "agent.h"

class CMainWnd : public CWindowWrapper
{
    public:
        CMainWnd (HINSTANCE instance);
        virtual ~CMainWnd ();

    protected:
        enum mode {
            SCHEMA = 1,
            BUNKERINGS,
            LOGBOOK,
            DAILY_REPORT
        };

        static const int SHIP_SCHEMA_WIDTH = 300;
        static const int BUNK_INFO_HEIGHT = 200;
        
        mode               viewMode;
        CTabCtrlWrapper   *modeSwitch;
        CStaticWrapper    *navData;
        ShipSchema        *shipSchema;
        BunkeringWindow   *bunkerings;
        LogbookWindow     *logbook;
        DailyReportWindow *dailyReport;
        pollerContext     *context;

        CComboBoxWrapper       *tankSelector;
        CStaticWrapper         *tankLabel, *beginLabel, *endLabel, *dateTime;
        CDateTimePickerWrapper *beginDate, *endDate, *beginTime, *endTime;
        CTrackbarWrapper       *timeSelector;
        CTabCtrlWrapper        *bunkerInfo;
        HMENU                   menu;
        config                  cfg;
        int                     selectedTank;
        int32_t                 lat, lon;
        uint32_t                cog, sog, hdg;
        database                db;
        dataHistory            *history;
        time_t                  beginTimestamp, endTimestamp, lastDataUpdate;

        virtual LRESULT OnMessage (UINT message, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnCommand (WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnSysCommand (WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnSize (const DWORD requestType, const WORD width, const WORD height);
        virtual LRESULT OnTimer (unsigned int timerID);
        virtual LRESULT OnNotify (NMHDR *pHeader);

        void RequestAppQuit ();

        virtual void OnCreate ();

        //void loadBunkeringList ();
        void switchToMode (mode newMode);

        void exportLevels ();

        void showNavData ();
};
