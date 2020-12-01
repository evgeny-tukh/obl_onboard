#pragma once

#include <Windows.h>
#include "../../common/defs.h"
#include "gdiobjects.h"

class fuelMeterDisplay {
    public:
        fuelMeterDisplay (fuelMeter&);
        ~fuelMeterDisplay ();

        void draw (HDC drawCtx, HWND wnd, int x, gdiObjects& objects, double value, const char *name);

        static const int WIDTH = 200;
        static const int HEIGHT = 50;
        static const int TOP = 20;
        static const int GAP = 20;
        static const int SHADOW_OFFSET = 6;

    protected:
        static HFONT fontHandle;

        fuelMeter& fmCfg;
};