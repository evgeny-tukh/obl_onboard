#pragma once

#include "../../common/defs.h"
#include "../wui/WindowWrapper.h"
#include "tank.h"
#include "../data_history.h"

class ShipSchema: public CWindowWrapper {
    public:
        ShipSchema (HINSTANCE instance, HWND parent, config& _cfg, dataHistory *hist);
        virtual ~ShipSchema ();

        inline void selectTank (int id) { selectedTank = id; }
        void setTimestamp (time_t ts);
        
    private:
        config& cfg;
        tankDisplay::gdiObjects objects;
        int selectedTank;
        dataHistory *history;
        time_t timestamp;

        void recalc (tankDisplay::metrics&);

        virtual void OnCreate ();
        virtual LRESULT OnPaint ();
};
