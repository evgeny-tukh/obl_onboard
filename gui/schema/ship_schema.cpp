#include "ship_schema.h"
#include "tank.h"

ShipSchema::ShipSchema (HINSTANCE instance, HWND parent, config& _cfg) :
    cfg (_cfg),
    CWindowWrapper (instance, parent, "obl_ship_schema") {

}

ShipSchema::~ShipSchema () {

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

    int offsetFromBow = 0;

    SelectObject (paintCtx, (HBRUSH) GetStockObject (WHITE_BRUSH));
    SelectObject (paintCtx, (HPEN) GetStockObject (BLACK_PEN));

    POINT vesselShape [10];

    vesselShape [0].x = client.right >> 1;
    vesselShape [0].y = 10;
    vesselShape [1].x = client.right - 30;
    vesselShape [1].y = 40;
    vesselShape [2].x = client.right - 10;
    vesselShape [2].y = 60;
    vesselShape [3].x = client.right - 10;
    vesselShape [3].y = client.bottom - 60;
    vesselShape [4].x = (client.right >> 2) * 3;
    vesselShape [4].y = client.bottom - 46;
    vesselShape [5].x = client.right >> 1;
    vesselShape [5].y = client.bottom - 40;
    vesselShape [6].x = client.right >> 2;
    vesselShape [6].y = client.bottom - 46;
    vesselShape [7].x = 10;
    vesselShape [7].y = client.bottom - 60;
    vesselShape [8].x = 10;
    vesselShape [8].y = 60;
    vesselShape [9].x = 30;
    vesselShape [9].y = 40;

    Polygon (paintCtx, vesselShape, 10);

    for (auto tank = cfg.tanks.begin (); tank != cfg.tanks.end (); ++ tank) {
        tankDisplay tankDisp (*tank);

        tankDisp.draw (paintCtx, m_hwndHandle, offsetFromBow, 50.0);
    }

    EndPaint (m_hwndHandle, & data);

    return 0;
}
