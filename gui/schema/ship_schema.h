#pragma once

#include <map>

#include "../../common/defs.h"
#include "../../common/db.h"
#include "../wui/WindowWrapper.h"
#include "../wui/StaticWrapper.h"
#include "../wui/TrackbarWrapper.h"
#include "../wui/ButtonWrapper.h"
#include "gdiobjects.h"
#include "tank.h"
#include "fuelmeter.h"
#include "../data_history.h"
#include "status_ind.h"

class ShipSchema: public CWindowWrapper {
    public:
        ShipSchema (HINSTANCE instance, HWND parent, database& db, config& _cfg, dataHistory *hist);
        virtual ~ShipSchema ();

        inline void selectTank (int id) { selectedTank = id; }
        void setTimestamp (time_t ts);
        
        void onNewData ();

        void redrawTanks ();
        
    private:
        static const int DATE_TIME_WIDTH = 200;
        
        config& cfg;
        database& db;
        CTrackbarWrapper *timeline;
        CStaticWrapper *dateTime;
        StatusIndicator *statusIndicator;
        CButtonWrapper *historyMode, *onlineMode;
        gdiObjects objects;
        int selectedTank;
        dataHistory *history;
        time_t timestamp;
        uint32_t updateTimer;
        bool isHistoryMode;
        
        union {
            struct {
                HBITMAP tank;
            };
            HBITMAP image [1];
        } images;

        BITMAP tankImgProps;

        std::map<uint32_t, time_t> tankUpdates, fuelMeterUpdates;

        void recalc (tankDisplay::metrics&);

        void updateStatus ();

        virtual void OnCreate ();
        virtual LRESULT OnPaint ();
        virtual LRESULT OnMessage (UINT message, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnSize (const DWORD requestType, const WORD width, const WORD height);
        virtual LRESULT OnTimer (UINT uiTimerID);
        virtual LRESULT OnCommand (WPARAM wParam, LPARAM lParam);
};
