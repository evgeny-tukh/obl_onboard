#include <stdlib.h>
#include <Windows.h>
#include "fuelmeter.h"

fuelMeterDisplay::fuelMeterDisplay (fuelMeter& cfg): fmCfg (cfg) {
}

fuelMeterDisplay::~fuelMeterDisplay () {
}

void fuelMeterDisplay::draw (HDC drawCtx, HWND wnd, int x, gdiObjects& objects, double value, const char *name) {
    char buffer [50];
    sprintf (buffer, "%.1f", value);

    SelectObject (drawCtx, objects.display);
    SelectObject (drawCtx, objects.displayEdge);
    RoundRect (drawCtx, x, TOP, x + WIDTH, TOP + HEIGHT, 5, 5);
    SetTextColor (drawCtx, RGB (255, 200, 0));
    SetBkMode (drawCtx, TRANSPARENT);
    TextOutA (drawCtx, x + 5, TOP + 5, buffer, strlen (buffer));
}
