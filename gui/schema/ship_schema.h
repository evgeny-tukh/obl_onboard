#pragma once

#include "../../common/defs.h"
#include "../wui/WindowWrapper.h"
#include "../wui/StaticWrapper.h"
#include "../wui/TrackbarWrapper.h"
#include "../wui/ButtonWrapper.h"
#include "gdiobjects.h"
#include "tank.h"
#include "fuelmeter.h"
#include "../data_history.h"

class ShipSchema: public CWindowWrapper {
    public:
        ShipSchema (HINSTANCE instance, HWND parent, config& _cfg, dataHistory *hist);
        virtual ~ShipSchema ();

        inline void selectTank (int id) { selectedTank = id; }
        void setTimestamp (time_t ts);
        
    private:
        static const int DATE_TIME_WIDTH = 200;
        
        config& cfg;
        CTrackbarWrapper *timeline;
        CStaticWrapper *dateTime;
        CButtonWrapper *historyMode, *onlineMode;
        gdiObjects objects;
        int selectedTank;
        dataHistory *history;
        time_t timestamp;
        uint32_t updateTimer;
        bool isHistoryMode;

        void recalc (tankDisplay::metrics&);

        virtual void OnCreate ();
        virtual LRESULT OnPaint ();
        virtual LRESULT OnMessage (UINT message, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnSize (const DWORD requestType, const WORD width, const WORD height);
        virtual LRESULT OnTimer (UINT uiTimerID);
        virtual LRESULT OnCommand (WPARAM wParam, LPARAM lParam);
};
