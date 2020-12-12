#pragma once

#include "../wui/ListCtrlWrapper.h"
#include "../../common/defs.h"
#include "base_list_ctrl.h"

class FuelStateEditCtrl: public BaseListCtrl {
    public:
        FuelStateEditCtrl (HWND parent, UINT controlID);
        virtual ~FuelStateEditCtrl ();

        virtual void init ();
        void showState (fuelState& state);
        bool readState (fuelState& state);

        virtual bool isItemEditable (int item) {
            return true;
        }
};