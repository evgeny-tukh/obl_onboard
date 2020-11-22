#pragma once

#include <Windows.h>
#include "../../common/defs.h"

class tankDisplay {
    public:
        tankDisplay (tank& tankCfg);
        ~tankDisplay ();

        void draw (HDC drawCtx, HWND wnd, int& offsetFromBow, double level);
};