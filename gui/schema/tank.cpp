#include "tank.h"
#include "../../common/tools.h"

tankDisplay::tankDisplay (tank& _cfg, layoutElement& _layout): tankCfg (_cfg), layout (_layout), image (0), x (0), y (0), width (0), height (0) {
}

tankDisplay::~tankDisplay () {
    if (image) DeleteObject (image);
}

bool tankDisplay::adjust (int wndWidth, int wndHeight) {
    switch (layout.unit) {
        case layoutUnit::PERCENT: {
            x = layout.x * wndWidth / 100;
            y = layout.y * wndHeight / 100;
            width = layout.width * wndWidth / 100;
            height = layout.height * wndHeight / 100;
            return true;
        }
        case layoutUnit::PIXELS: {
            x = layout.x;
            y = layout.y;
            width = layout.width;
            height = layout.height;
            return true;
        }
    }

    return false;
}

void tankDisplay::paint (HDC drawCtx, HBITMAP tankImage, double volume, uint16_t id, const char *type) {
    HDC tempCtx = CreateCompatibleDC (drawCtx);
    BITMAP tankImgInfo;

    if (image) DeleteObject (image);

    GetObject (tankImage, sizeof (tankImgInfo),  & tankImgInfo);
    SelectObject (tempCtx, tankImage);
    StretchBlt (drawCtx, x, y, width, height, tempCtx, 0, 0, tankImgInfo.bmWidth, tankImgInfo.bmHeight, SRCCOPY);
    SelectObject (tempCtx, 0);
    DeleteDC (tempCtx);

    char idString [10], volString [50];
    int filledPartHeight = volume <  tankCfg.volume ? (int) ((volume / tankCfg.volume) * (double) height) : height;

    sprintf (idString, "#%d", id);
    sprintf (volString, "%.1f/%.f", volume, tankCfg.volume);

    RECT scale {
        x + width, y - 10, x + width + 11, y + height + 10
    };
    POINT gauge [3] {
        { x + width, y + height - filledPartHeight },
        { x + width + 10, y + height - filledPartHeight - 10 },
        { x + width + 10, y + height - filledPartHeight + 10 }
    };

    FillRect (drawCtx, & scale, (HBRUSH) GetStockObject (WHITE_BRUSH));
    SelectObject (drawCtx, GetStockObject (BLACK_BRUSH));
    Polygon (drawCtx, gauge, 3);

    /*switch (layout.labelPos) {
        case layoutLabelPos::ABOVE:
            y -= 45; break;
        case layoutLabelPos::BELOW:
            y += height - 15; break;
        default:
            return;
    }*/
    y += height / 2 - 55;

    SetTextColor (drawCtx, RGB (255, 255, 255));
    SetBkMode (drawCtx, TRANSPARENT);
    TextOutA (drawCtx, x + 10, y + 30, idString, strlen (idString));
    TextOutA (drawCtx, x + 10, y + 45, type, strlen (type));
    TextOutA (drawCtx, x + 10, y + 60, volString, strlen (volString));
}

void tankDisplay::updateValue (HDC drawCtx, double volume, uint16_t id) {

}

void tankDisplay::draw (
    HDC drawCtx,
    HWND wnd,
    metrics& tankMetrics,
    gdiObjects& objects,
    double volume,
    uint16_t id,
    const char *type,
    bool selected
) {
    int x, y;

    switch (tankCfg.side) {
        case tankSide::center:
            x = HOR_EDGE * 3 + tankMetrics.width;
            y = tankMetrics.middle;
            tankMetrics.middle += tankMetrics.height + HOR_EDGE; break;
        case tankSide::port:
            x = HOR_EDGE * 2;
            y = tankMetrics.port;
            tankMetrics.port += tankMetrics.height + HOR_EDGE; break;
        case tankSide::starboard:
            x = HOR_EDGE * 4 + tankMetrics.width * 2;
            y = tankMetrics.stbd;
            tankMetrics.stbd += tankMetrics.height + HOR_EDGE; break;
        dafault:
            return;
    }

    int filledPartHeight = (int) ((volume / tankCfg.volume) * (double) (tankMetrics.height - 35));
    char idString [10], volString [50];

    sprintf (idString, "#%d", id);
    sprintf (volString, "%.1f/%.f", volume, tankCfg.volume);

    #if 1
    //SelectObject (drawCtx, objects.tankBrush);
    //Ellipse (drawCtx, x, y + tankMetrics.height - 20, x + tankMetrics.width - 1, y + tankMetrics.height - 1);
    //Rectangle (drawCtx, x, y + 10, x + tankMetrics.width - 1, y + tankMetrics.height - 10);
    paintRectangleGradient (drawCtx, x, y + 10, x + tankMetrics.width - 1, y + tankMetrics.height - 9, RGB (100, 100, 100), RGB (200, 200, 200), true);
    paintEllipseGradient (drawCtx, x, y + tankMetrics.height - 20, x + tankMetrics.width - 1, y + tankMetrics.height - 1, RGB (100, 100, 100), RGB (200, 200, 200), true);
    //SelectObject (drawCtx, GetStockObject (BLACK_BRUSH));
    //Ellipse (drawCtx, x, y, x + tankMetrics.width - 1, y + 19);
    paintEllipseGradient (drawCtx, x, y, x + tankMetrics.width - 1, y + 19, RGB (30, 30, 30), RGB (120, 120, 120), true);
    //SelectObject (drawCtx, objects.tankPen);
    //MoveToEx (drawCtx, x + 1, y + tankMetrics.height - 11, 0);
    //LineTo (drawCtx, x + tankMetrics.width - 2, y + tankMetrics.height - 11);
    SelectObject (drawCtx, GetStockObject (BLACK_PEN));
    SelectObject (drawCtx, GetStockObject (WHITE_BRUSH));
    Rectangle (drawCtx, x + tankMetrics.width / 2 - 5, y + 25, x + tankMetrics.width / 2 + 5, y + tankMetrics.height - 10);
    SelectObject (drawCtx, objects.shadow);
    Rectangle (drawCtx, x + tankMetrics.width / 2 - 5, y + tankMetrics.height - 10 - filledPartHeight, x + tankMetrics.width / 2 + 5, y + tankMetrics.height - 10);
    SetTextColor (drawCtx, 0);
    SetBkMode (drawCtx, TRANSPARENT);
    TextOutA (drawCtx, x + 10, y + 30, idString, strlen (idString));
    TextOutA (drawCtx, x + 10, y + 45, type, strlen (type));
    TextOutA (drawCtx, x + 10, y + 60, volString, strlen (volString));
    #else
    if (selected)
    {
        SelectObject (drawCtx, (HBRUSH) GetStockObject (NULL_BRUSH));
        SelectObject (drawCtx, objects.selectionBorder);
        Rectangle (drawCtx, x - 5, y - 5, x + tankMetrics.width + 4, y + tankMetrics.height + 4);
    }

    SelectObject (drawCtx, objects.freeArea);
    SelectObject (drawCtx, (HBRUSH) GetStockObject (BLACK_PEN));
    RoundRect (drawCtx, x, y, x + tankMetrics.width - 1, y + tankMetrics.height - 1, 20, 20);

    if (volume > 1.0f) {
        SelectObject (drawCtx, objects.filledArea);
        RoundRect (drawCtx, x, y + tankMetrics.height - filledPartHeight, x + tankMetrics.width - 1, y + tankMetrics.height - 1, 20, 20);
        SelectObject (drawCtx, (HPEN) GetStockObject (NULL_PEN));
        Rectangle (drawCtx, x + 1, y + tankMetrics.height - filledPartHeight, x + tankMetrics.width - 1, y + tankMetrics.height - 20);
    }
    
    SetTextColor (drawCtx, 0);
    SetBkMode (drawCtx, TRANSPARENT);
    TextOutA (drawCtx, x + 10, y + 10, idString, strlen (idString));
    TextOutA (drawCtx, x + 10, y + 25, type, strlen (type));
    TextOutA (drawCtx, x + 10, y + 40, volString, strlen (volString));
    #endif
}

#if 0
tankDisplayWnd::tankDisplayWnd (HINSTANCE instance, HWND parent, UINT_PTR id, tank& _tankCfg, gdiObjects& _objects):
    CWindowWrapper (instance, parent, "obl_status_ind", (HMENU) id, 0, 0, NULL_BRUSH), objects (_objects), tankCfg (_tankCfg) {
}

LRESULT tankDisplayWnd::OnPaint () {
    PAINTSTRUCT data;
    HDC paintCtx = BeginPaint (m_hwndHandle, & data);
    RECT client;
    
    int x, y;

    switch (tankCfg.side) {
        case tankSide::center:
            x = HOR_EDGE * 3 + tankMetrics->width;
            y = tankMetrics->middle;
            tankMetrics->middle += tankMetrics->height + HOR_EDGE; break;
        case tankSide::port:
            x = HOR_EDGE * 2;
            y = tankMetrics->port;
            tankMetrics->port += tankMetrics->height + HOR_EDGE; break;
        case tankSide::starboard:
            x = HOR_EDGE * 4 + tankMetrics->width * 2;
            y = tankMetrics->stbd;
            tankMetrics->stbd += tankMetrics->height + HOR_EDGE; break;
        dafault:
            EndPaint (m_hwndHandle, & data); return 0;
    }

    int filledPartHeight = (int) ((volume / tankCfg.volume) * (double) (tankMetrics->height - 35));
    char idString [10], volString [50];

    sprintf (idString, "#%d", id);
    sprintf (volString, "%.1f/%.f", volume, tankCfg.volume);

    paintRectangleGradient (paintCtx, x, y + 10, x + tankMetrics->width - 1, y + tankMetrics->height - 9, RGB (100, 100, 100), RGB (200, 200, 200), true);
    paintEllipseGradient (paintCtx, x, y + tankMetrics->height - 20, x + tankMetrics->width - 1, y + tankMetrics->height - 1, RGB (100, 100, 100), RGB (200, 200, 200), true);
    paintEllipseGradient (paintCtx, x, y, x + tankMetrics->width - 1, y + 19, RGB (30, 30, 30), RGB (120, 120, 120), true);
    SelectObject (paintCtx, GetStockObject (BLACK_PEN));
    SelectObject (paintCtx, GetStockObject (WHITE_BRUSH));
    Rectangle (paintCtx, x + tankMetrics->width / 2 - 5, y + 25, x + tankMetrics->width / 2 + 5, y + tankMetrics->height - 10);
    SelectObject (paintCtx, objects.shadow);
    Rectangle (paintCtx, x + tankMetrics->width / 2 - 5, y + tankMetrics->height - 10 - filledPartHeight, x + tankMetrics->width / 2 + 5, y + tankMetrics->height - 10);
    SetTextColor (paintCtx, 0);
    SetBkMode (paintCtx, TRANSPARENT);
    TextOutA (paintCtx, x + 10, y + 30, idString, strlen (idString));
    TextOutA (paintCtx, x + 10, y + 45, type, strlen (type));
    TextOutA (paintCtx, x + 10, y + 60, volString, strlen (volString));
    EndPaint (m_hwndHandle, & data);

    return 0L;
}
#endif