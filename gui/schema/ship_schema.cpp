#include "ship_schema.h"

ShipSchema::ShipSchema (HINSTANCE instance, HWND parent, config& _cfg) :
    cfg (_cfg),
    selectedTank (-1),
    CWindowWrapper (instance, parent, "obl_ship_schema") {
    objects.filledArea = CreateSolidBrush (RGB (200, 160, 255));
    objects.freeArea = CreateSolidBrush (RGB (255, 200, 255));
    objects.selectionBorder = CreatePen (PS_SOLID, 5, RGB (255, 0, 0));
}

ShipSchema::~ShipSchema () {
    DeleteObject (objects.filledArea);
    DeleteObject (objects.freeArea);
    DeleteObject (objects.selectionBorder);
}

void ShipSchema::OnCreate () {
    RECT parent;

    ::GetClientRect (m_hwndParent, & parent);
    ::MoveWindow (m_hwndHandle, 0, 0, 100, parent.bottom, TRUE);
}

LRESULT ShipSchema::OnPaint () {
    PAINTSTRUCT data;
    RECT client;
    HDC paintCtx = BeginPaint (m_hwndHandle, & data);

    GetClientRect (& client);
    FillRect (paintCtx, & client, (HBRUSH) GetStockObject (GRAY_BRUSH));

    SelectObject (paintCtx, (HBRUSH) GetStockObject (WHITE_BRUSH));
    SelectObject (paintCtx, (HPEN) GetStockObject (BLACK_PEN));

    POINT vesselShape [12];

    vesselShape [0].x = client.right >> 1;
    vesselShape [0].y = 10;
    vesselShape [1].x = client.right - 30;
    vesselShape [1].y = 40;
    vesselShape [2].x = client.right - 10;
    vesselShape [2].y = 60;
    vesselShape [3].x = client.right - 10;
    vesselShape [3].y = client.bottom - 80;
    vesselShape [4].x = client.right - 30;
    vesselShape [4].y = client.bottom - 60;
    vesselShape [5].x = (client.right >> 2) * 3;
    vesselShape [5].y = client.bottom - 46;

    vesselShape [6].x = client.right >> 1;
    vesselShape [6].y = client.bottom - 40;

    vesselShape [7].x = client.right >> 2;
    vesselShape [7].y = client.bottom - 46;

    vesselShape [8].x = 30;
    vesselShape [8].y = client.bottom - 60;

    vesselShape [9].x = 10;
    vesselShape [9].y = client.bottom - 80;
    vesselShape [10].x = 10;
    vesselShape [10].y = 60;
    vesselShape [11].x = 30;
    vesselShape [11].y = 40;

    Polygon (paintCtx, vesselShape, 12);

    tankDisplay::metrics tankMetrics;

    recalc (tankMetrics);

    for (auto tank = cfg.tanks.begin (); tank != cfg.tanks.end (); ++ tank) {
        tankDisplay tankDisp (*tank);

        tankDisp.draw (paintCtx, m_hwndHandle, tankMetrics, objects, 50.0, tank->id, tank->type.c_str (), selectedTank == tank->id);
    }

    EndPaint (m_hwndHandle, & data);

    return 0;
}

void ShipSchema::recalc (tankDisplay::metrics& tankMetrics) {
    RECT client;

    GetClientRect (& client);

    int stbdCount = 0, portCount = 0, middleCount = 0;

    for (auto tank = cfg.tanks.begin (); tank != cfg.tanks.end (); ++ tank) {
        switch (tank->side) {
            case tankSide::starboard: stbdCount ++; break;
            case tankSide::port: portCount ++; break;
            case tankSide::center: middleCount ++; break;
        }
    }

    int maxCount = max (max (stbdCount, middleCount), portCount);

    tankMetrics.width = (client.right - tankDisplay::HOR_EDGE * 6) / 3;
    tankMetrics.height = (client.bottom - (maxCount - 1) * tankDisplay::HOR_EDGE - 2 * tankDisplay::VER_EDGE) / maxCount;
}