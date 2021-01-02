#pragma once

#include <Windows.h>

struct gdiObjects {
    union {
        struct {
            HBRUSH freeArea, filledArea, display, shadow, tankBrush;
            HPEN selectionBorder, displayEdge, tankPen, pipeBlack, pipeRed, pipeGray, pipeGreen, pipeBlue;
        };
        HGDIOBJ objects [50];
    };

    gdiObjects () {
        memset (objects, 0, sizeof (objects));

        filledArea = CreateSolidBrush (RGB (200, 160, 255));
        shadow = CreateSolidBrush (RGB (90, 90, 90));
        freeArea = CreateSolidBrush (RGB (255, 200, 255));
        selectionBorder = CreatePen (PS_SOLID, 5, RGB (255, 0, 0));
        display = CreateSolidBrush (RGB (50, 50, 50));
        displayEdge = CreatePen (PS_SOLID, 3, RGB (255, 200, 0));
        tankBrush = CreateSolidBrush (RGB (180, 180, 180));
        tankPen = CreatePen (PS_SOLID, 1, RGB (180, 180, 180));
        pipeBlack = CreatePen (PS_SOLID, 5, 0);
        pipeRed = CreatePen (PS_SOLID, 5, RGB (255, 0, 0));
        pipeGray = CreatePen (PS_SOLID, 5, RGB (180, 180, 180));
        pipeBlue = CreatePen (PS_SOLID, 5, RGB (0, 0, 255));
        pipeGreen = CreatePen (PS_SOLID, 5, RGB (0, 255, 0));
    }

    virtual ~gdiObjects () {
        for (size_t i = 0; i < sizeof (objects) / sizeof (*objects); ++ i) {
            if (objects [i]) DeleteObject (objects [i]);
        }
    }
};

