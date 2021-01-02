#include "tank.h"
#include "../../common/tools.h"
#include "gdiobjects.h"

bool layoutElementDisplay::adjust (layoutUnit unit, layoutNode& layout, int& x, int&y, int& width, int& height, int wndWidth, int wndHeight) {
    switch (unit) {
        case layoutUnit::PERCENT: {
            x = layout.x * wndWidth / 100;
            y = layout.y * wndHeight / 100;
            height = layout.height > 0 ? (layout.height * wndHeight / 100) : 0;

            if (layout.width > 0) {
                if (layout.height > 0) {
                    width = (layout.width * height / layout.height);
                } else {
                    width = layout.width * wndWidth / 100;
                }
            } else {
                width = 0;
            }

            if (layout.offsetX) x += layout.offsetX;
            if (layout.offsetY) y += layout.offsetY;
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

void layoutElementDisplay::paint (HDC drawCtx, HBITMAP srcImage) {
    HDC tempCtx = CreateCompatibleDC (drawCtx);
    BITMAP imgInfo;

    GetObject (srcImage, sizeof (imgInfo),  & imgInfo);
    SelectObject (tempCtx, srcImage);
    
    if (width > 0 & height > 0)
        StretchBlt (drawCtx, x, y, width, height, tempCtx, 0, 0, imgInfo.bmWidth, imgInfo.bmHeight, SRCCOPY);
    else
        BitBlt (drawCtx, x, y, imgInfo.bmWidth, imgInfo.bmHeight, tempCtx, 0, 0, SRCCOPY);

    SelectObject (tempCtx, 0);
    DeleteDC (tempCtx);
}

fmDisplay::fmDisplay (fuelMeter& _cfg, layoutElement& _layout): layoutElementDisplay (_layout), fmCfg (_cfg) {}

void fmDisplay::paint (HDC drawCtx, HBITMAP fmImage, double value, uint16_t id) {
    layoutElementDisplay::paint (drawCtx, fmImage);

    BITMAP imgInfo;
    GetObject (fmImage, sizeof (imgInfo),  & imgInfo);

    char valueString [50];
    sprintf (valueString, "%.3f", value);
    SetTextColor (drawCtx, RGB (80, 80, 80));
    SetBkMode (drawCtx, TRANSPARENT);

    if (!layout.labelText.empty ()) {
        WCHAR label [100];
        MultiByteToWideChar (CP_UTF8, 0, layout.labelText.c_str (), -1/*layout.labelText.length ()*/, label, 100);
        TextOutW (drawCtx, x + 15, y + imgInfo.bmHeight / 2 - 20, label, wcslen (label));
    }
    TextOutA (drawCtx, x + 15, y + imgInfo.bmHeight / 2 - 5, valueString, strlen (valueString));
}

tankDisplay::tankDisplay (tank& _cfg, layoutElement& _layout): layoutElementDisplay (_layout), tankCfg (_cfg), image (0) {}

tankDisplay::~tankDisplay () {
    if (image) DeleteObject (image);
}

void tankDisplay::paint (HDC drawCtx, HBITMAP tankImage, double volume, uint16_t id, const char *type) {
    layoutElementDisplay::paint (drawCtx, tankImage);

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

    paintRectangleGradient (drawCtx, x, y + 10, x + tankMetrics.width - 1, y + tankMetrics.height - 9, RGB (100, 100, 100), RGB (200, 200, 200), true);
    paintEllipseGradient (drawCtx, x, y + tankMetrics.height - 20, x + tankMetrics.width - 1, y + tankMetrics.height - 1, RGB (100, 100, 100), RGB (200, 200, 200), true);
    paintEllipseGradient (drawCtx, x, y, x + tankMetrics.width - 1, y + 19, RGB (30, 30, 30), RGB (120, 120, 120), true);
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
}

bool pipeDisplay::adjust (int wndWidth, int wndHeight) {
    bool result = layoutElementDisplay::adjust (wndWidth, wndHeight);

    for (auto& nodeLayout: layout.nodes) {
        int x, y, width, height;
        
        layoutElementDisplay::adjust (layout.unit, nodeLayout, x, y, width, height, wndWidth, wndHeight);

        nodes.emplace_back (x, y);
    }

    return result;
}

void pipeDisplay::paint (HDC drawCtx, gdiObjects& gdiObj) {
    HPEN pen;

    switch (layout.color) {
        case penColor::BLACK: pen = gdiObj.pipeBlack; break;
        case penColor::BLUE: pen = gdiObj.pipeBlue; break;
        case penColor::RED: pen = gdiObj.pipeRed; break;
        case penColor::GREEN: pen = gdiObj.pipeGreen; break;
        case penColor::GRAY: pen = gdiObj.pipeGray; break;
        default: pen = gdiObj.pipeBlack; break;
    }

    auto oldPen = SelectObject (drawCtx, pen);
    MoveToEx (drawCtx, nodes.front ().x, nodes.front ().y, 0);

    for (auto node = (nodes.begin () ++); node != nodes.end (); ++ node)
        LineTo (drawCtx, node->x, node->y);

    SelectObject (drawCtx, oldPen);
}
