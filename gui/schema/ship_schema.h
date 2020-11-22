#pragma once

#include "../../common/defs.h"
#include "../wui/WindowWrapper.h"
#include "tank.h"

class ShipSchema: public CWindowWrapper {
    public:
        ShipSchema (HINSTANCE instance, HWND parent, config& _cfg);
        virtual ~ShipSchema ();

    private:
        config& cfg;
        tankDisplay::gdiObjects objects;

        void recalc (tankDisplay::metrics&);

        virtual void OnCreate ();
        virtual LRESULT OnPaint ();
};
