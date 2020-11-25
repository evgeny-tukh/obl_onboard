#pragma once

#include <Windows.h>
#include "../../common/defs.h"
#include "gdiobjects.h"

class fuelMeterDisplay {
    public:
        static const int WIDTH = 70;
        static const int HEIGHT = 25;
        static const int TOP = 20;
        static const int GAP = 20;

        fuelMeterDisplay (fuelMeter&);
        ~fuelMeterDisplay ();

        void draw (HDC drawCtx, HWND wnd, int x, gdiObjects& objects, double value, const char *name);

    protected:
        fuelMeter& fmCfg;
};