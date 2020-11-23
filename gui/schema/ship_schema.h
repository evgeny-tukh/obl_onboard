#pragma once

#include "../../common/defs.h"
#include "../wui/WindowWrapper.h"
#include "tank.h"

class ShipSchema: public CWindowWrapper {
    public:
        ShipSchema (HINSTANCE instance, HWND parent, config& _cfg);
        virtual ~ShipSchema ();

        inline void selectTank (int id) { selectedTank = id; }
        
    private:
        config& cfg;
        tankDisplay::gdiObjects objects;
        int selectedTank;

        void recalc (tankDisplay::metrics&);

        virtual void OnCreate ();
        virtual LRESULT OnPaint ();
};
