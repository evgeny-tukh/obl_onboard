#include <stdlib.h>
#include <Windows.h>
#include "fuelmeter.h"

HFONT fuelMeterDisplay::fontHandle = 0;

fuelMeterDisplay::fuelMeterDisplay (fuelMeter& cfg): fmCfg (cfg) {
    if (!fontHandle) {
        LOGFONTA font;
        memset (& font, 0, sizeof (font));
        font.lfHeight = 40;
        font.lfWeight = FW_BOLD;
        font.lfCharSet = RUSSIAN_CHARSET;
        font.lfOutPrecision = OUT_DEFAULT_PRECIS; 
        font.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        font.lfQuality = PROOF_QUALITY; 
        font.lfPitchAndFamily = VARIABLE_PITCH | FF_ROMAN; 
        
        strcpy (font.lfFaceName, "Arial");

        fontHandle = CreateFontIndirectA (& font);
    }
}

fuelMeterDisplay::~fuelMeterDisplay () {
}

void fuelMeterDisplay::draw (HDC drawCtx, HWND wnd, int x, gdiObjects& objects, double value, const char *name) {
    char buffer [50];
    sprintf (buffer, "%.1f", value);

    SelectObject (drawCtx, GetStockObject (NULL_PEN));
    SelectObject (drawCtx, objects.shadow);
    RoundRect (drawCtx, x + SHADOW_OFFSET, TOP + SHADOW_OFFSET, x + WIDTH + SHADOW_OFFSET, TOP + HEIGHT + SHADOW_OFFSET, 10, 10);
    SelectObject (drawCtx, objects.display);
    SelectObject (drawCtx, objects.displayEdge);
    RoundRect (drawCtx, x, TOP, x + WIDTH, TOP + HEIGHT, 10, 10);
    SetTextColor (drawCtx, RGB (255, 200, 0));
    SetBkMode (drawCtx, TRANSPARENT);
    SelectObject (drawCtx, fontHandle);
    //TextOutA (drawCtx, x + 5, TOP + 5, buffer, strlen (buffer));
    RECT textRect;
    textRect.left = x + 5;
    textRect.right = x + WIDTH - 5;
    textRect.top = TOP + 5;
    textRect.bottom = TOP + HEIGHT - 5;
    DrawTextExA (drawCtx, buffer, strlen (buffer), & textRect, DT_CENTER | DT_VCENTER, 0);
}
