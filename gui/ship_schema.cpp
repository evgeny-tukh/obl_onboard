#include "ship_schema.h"

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
    EndPaint (m_hwndHandle, & data);

    return 0;
}
