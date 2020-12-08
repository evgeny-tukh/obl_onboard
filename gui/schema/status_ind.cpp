#include "status_ind.h"

StatusIndicator::StatusIndicator (HINSTANCE instance, HWND parent, UINT_PTR id):
    CWindowWrapper (instance, parent, "obl_status_ind", (HMENU) id, 0, 0, NULL_BRUSH),
    status (dataStatus::ok), red (0), green (0), yellow (0)
{
    red = CreateSolidBrush (RGB (255, 0, 0));
    green = CreateSolidBrush (RGB (0, 255, 0));
    yellow = CreateSolidBrush (RGB (255, 150, 0));
}

StatusIndicator::~StatusIndicator () {
    DeleteObject (red);
    DeleteObject (green);
    DeleteObject (yellow);
}

LRESULT StatusIndicator::OnPaint () {
    PAINTSTRUCT data;
    HDC paintCtx = BeginPaint (m_hwndHandle, & data);
    RECT client;
    
    GetClientRect (& client);
    SelectObject (paintCtx, GetStockObject (BLACK_PEN));
    switch (status) {
        case dataStatus::ok: SelectObject (paintCtx, green); break;
        case dataStatus::warning: SelectObject (paintCtx, yellow); break;
        case dataStatus::failure: SelectObject (paintCtx, red); break;
    }
    Ellipse (paintCtx, 0, 0, client.right, client.bottom);
    EndPaint (m_hwndHandle, & data);

    return 0L;
}
