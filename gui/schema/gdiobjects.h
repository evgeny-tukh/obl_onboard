#pragma once

#include <Windows.h>

struct gdiObjects {
    HBRUSH freeArea, filledArea, display, shadow, tankBrush;
    HPEN selectionBorder, displayEdge, tankPen;

    gdiObjects () {
        filledArea = CreateSolidBrush (RGB (200, 160, 255));
        shadow = CreateSolidBrush (RGB (90, 90, 90));
        freeArea = CreateSolidBrush (RGB (255, 200, 255));
        selectionBorder = CreatePen (PS_SOLID, 5, RGB (255, 0, 0));
        display = CreateSolidBrush (RGB (50, 50, 50));
        displayEdge = CreatePen (PS_SOLID, 3, RGB (255, 200, 0));
        tankBrush = CreateSolidBrush (RGB (180, 180, 180));
        tankPen = CreatePen (PS_SOLID, 1, RGB (180, 180, 180));
    }

    virtual ~gdiObjects () {
        DeleteObject (filledArea);
        DeleteObject (freeArea);
        DeleteObject (selectionBorder);
        DeleteObject (display);
        DeleteObject (displayEdge);
        DeleteObject (shadow);
        DeleteObject (tankBrush);
        DeleteObject (tankPen);
    }
};

