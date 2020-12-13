#pragma once

#include "../wui/ListCtrlWrapper.h"
#include "../../common/defs.h"
#include "base_list_ctrl.h"

class HwDataEditCtrl: public BaseListCtrl {
    public:
        HwDataEditCtrl (HWND parent, UINT controlID);
        virtual ~HwDataEditCtrl ();

        virtual void init ();
        void showState (fuelState& state);
        bool readState (fuelState& state);

        inline virtual bool isItemEditable (int item) {
            return item == 5 || item == 6;
        }
};