#pragma once

#include "../../common/defs.h"
#include "../wui/WindowWrapper.h"

class ShipSchema: public CWindowWrapper {
    public:
        ShipSchema (HINSTANCE instance, HWND parent, config& _cfg);
        virtual ~ShipSchema ();

    private:
        config& cfg;
        virtual void OnCreate ();
        virtual LRESULT OnPaint ();
};
