#pragma once

#include "../wui/WindowWrapper.h"
#include "../wui/StaticWrapper.h"
#include "../wui/EditWrapper.h"
#include "../../common/defs.h"
#include "fuel_state_edit_ctrl.h"

class BunkeringHeaderWindow: public CWindowWrapper {
    public:
        BunkeringHeaderWindow (HINSTANCE instance, HWND parent, config& _cfg);
        virtual ~BunkeringHeaderWindow ();

        inline void setBunkeringData (bunkeringData& _data) { bunkerData = & _data; }

    protected:
        bunkeringData *bunkerData;
        FuelStateEditCtrl *fuelState;
        config& cfg;

        virtual void OnCreate ();
};