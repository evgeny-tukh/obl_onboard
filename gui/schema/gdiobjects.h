#pragma once

#include <Windows.h>

struct gdiObjects {
    HBRUSH freeArea, filledArea, display;
    HPEN selectionBorder, displayEdge;

    gdiObjects () {
        filledArea = CreateSolidBrush (RGB (200, 160, 255));
        freeArea = CreateSolidBrush (RGB (255, 200, 255));
        selectionBorder = CreatePen (PS_SOLID, 5, RGB (255, 0, 0));
        display = CreateSolidBrush (RGB (50, 50, 50));
        displayEdge = CreatePen (PS_SOLID, 3, RGB (255, 200, 0));
    }

    virtual ~gdiObjects () {
        DeleteObject (filledArea);
        DeleteObject (freeArea);
        DeleteObject (selectionBorder);
        DeleteObject (display);
        DeleteObject (displayEdge);
    }
};

