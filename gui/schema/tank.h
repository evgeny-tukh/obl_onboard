#pragma once

#include <Windows.h>
#include "../../common/defs.h"
#include "gdiobjects.h"
#include "../wui/WindowWrapper.h"

class layoutElementDisplay {
    public:
        layoutElementDisplay (layoutElement& elem): layout (elem), x (0), y (0), width (0), height (0) {}

        static bool adjust (layoutUnit unit, layoutNode& layout, int& x, int&y, int& width, int& height, int wndWidth, int wndHeight);
        inline bool adjust (int wndWidth, int wndHeight) {
            return adjust (layout.unit, layout, x, y, width, height, wndWidth, wndHeight);
        }

        void paint (HDC drawCtx, HBITMAP srcImage);

    protected:
        layoutElement& layout;
        int x, y, width, height;
};

class pipeDisplay: public layoutElementDisplay {
    public:
        pipeDisplay (layoutElement& elem): layoutElementDisplay (elem) {}

        void paint (HDC drawCtx, gdiObjects& objects);
        bool adjust (int wndWidth, int wndHeight);

        struct point {
            int x, y;

            point (int _x, int _y): x (_x), y (_y) {}
        };

    protected:
        std::vector<point> nodes;
};

class tankDisplay: public layoutElementDisplay {
    public:
        static const int VER_EDGE = 80;
        static const int HOR_EDGE = 10;

        struct metrics {
            int stbd, port, middle, width, height;

            metrics () : stbd (VER_EDGE), port (VER_EDGE), middle (VER_EDGE) {}
        };

        tankDisplay (tank&, layoutElement&);
        ~tankDisplay ();

        void draw (HDC drawCtx, HWND wnd, metrics&, gdiObjects& objects, double volume, uint16_t id, const char *type, bool selected);
        void paint (HDC drawCtx, HBITMAP tankImage, double volume, uint16_t id, const char *type);
        void updateValue (HDC drawCtx, double volume, uint16_t id);

    protected:
        tank& tankCfg;
        BITMAP imageInfo;
        HBITMAP image;
};

class fmDisplay: public layoutElementDisplay {
    public:
        fmDisplay (fuelMeter&, layoutElement&);
        ~fmDisplay () {}

        void paint (HDC drawCtx, HBITMAP fmImage, double counter, uint16_t id);

    protected:
        fuelMeter& fmCfg;
};

#if 0
class tankDisplayWnd: public CWindowWrapper {
    public:
        static const int VER_EDGE = 80;
        static const int HOR_EDGE = 10;

        tankDisplayWnd (HINSTANCE instance, HWND parent, UINT_PTR id, tank& cfg, gdiObjects& _objects);
        virtual ~tankDisplayWnd () {}

        BOOL Create (const int nX, const int nY, const int nWidth, const int nHeight) {
            return CWindowWrapper::Create (0, nX, nY, nWidth, nHeight, WS_CHILD | WS_VISIBLE);
        }

        struct metrics {
            int stbd, port, middle, width, height;

            metrics () : stbd (VER_EDGE), port (VER_EDGE), middle (VER_EDGE) {}
        };

        inline void update (metrics *_metrics, double _volume, uint16_t _id, const char *_type, bool _selected) {
            tankMetrics = _metrics;
            volume = _volume;
            id = _id;
            type = _type;
            selected = _selected;

            InvalidateRect (m_hwndHandle, 0, TRUE);
        }
        
        virtual LRESULT OnPaint ();

    protected:
        metrics *tankMetrics;
        double volume;
        uint16_t id;
        bool selected;
        const char *type;
        gdiObjects& objects;
        tank& tankCfg;
};
#endif