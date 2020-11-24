#pragma once

#include <Windows.h>
#include "../../common/defs.h"
#include "gdiobjects.h"

class tankDisplay {
    public:
        static const int VER_EDGE = 80;
        static const int HOR_EDGE = 10;

        struct metrics {
            int stbd, port, middle, width, height;

            metrics () : stbd (VER_EDGE), port (VER_EDGE), middle (VER_EDGE) {}
        };

        tankDisplay (tank&);
        ~tankDisplay ();

        void draw (HDC drawCtx, HWND wnd, metrics&, gdiObjects& objects, double volume, uint16_t id, const char *type, bool selected);

    protected:
        tank& tankCfg;
};