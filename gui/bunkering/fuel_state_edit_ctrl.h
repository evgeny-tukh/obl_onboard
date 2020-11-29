#pragma once

#include "../wui/ListCtrlWrapper.h"
#include "../../common/defs.h"

class FuelStateEditCtrl: public CListCtrlWrapper {
    public:
        FuelStateEditCtrl (HWND parent, UINT controlID);
        virtual ~FuelStateEditCtrl ();

        void init ();
        void showState (fuelState& state);
        void readState (fuelState& state);
        bool editValue (int item);

        char *getFormat (int item);
};