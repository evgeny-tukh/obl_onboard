#include "tank.h"

tankDisplay::tankDisplay (tank& cfg): tankCfg (cfg) {

}

tankDisplay::~tankDisplay () {

}

void tankDisplay::draw (HDC drawCtx, HWND wnd, metrics& tankMetrics, gdiObjects& objects, double volume, uint16_t id, const char *type) {
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

    int filledPartHeight = (int) ((volume / tankCfg.volume) * (double) tankMetrics.height);
    char idString [10], volString [50];

    sprintf (idString, "#%d", id);
    sprintf (volString, "%.1f/%.f", volume, tankCfg.volume);

    SelectObject (drawCtx, objects.freeArea);
    SelectObject (drawCtx, (HBRUSH) GetStockObject (BLACK_PEN));
    RoundRect (drawCtx, x, y, x + tankMetrics.width - 1, y + tankMetrics.height - 1, 20, 20);
    SelectObject (drawCtx, objects.filledArea);
    RoundRect (drawCtx, x, y + tankMetrics.height - filledPartHeight, x + tankMetrics.width - 1, y + tankMetrics.height - 1, 20, 20);
    SelectObject (drawCtx, (HBRUSH) GetStockObject (NULL_PEN));
    Rectangle (drawCtx, x + 1, y + tankMetrics.height - filledPartHeight, x + tankMetrics.width - 1, y + tankMetrics.height - 20);

    SetTextColor (drawCtx, 0);
    SetBkMode (drawCtx, TRANSPARENT);
    TextOutA (drawCtx, x + 10, y + 10, idString, strlen (idString));
    TextOutA (drawCtx, x + 10, y + 25, type, strlen (type));
    TextOutA (drawCtx, x + 10, y + 40, volString, strlen (volString));
}
