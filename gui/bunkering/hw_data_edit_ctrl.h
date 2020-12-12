#pragma once

#include "../wui/ListCtrlWrapper.h"
#include "../../common/defs.h"
#include "base_list_ctrl.h"

class HwDataEditCtrl: public BaseListCtrl {
    public:
        HwDataEditCtrl (HWND parent, UINT controlID);
        virtual ~HwDataEditCtrl ();

        virtual void init ();
        void showState (fuelState& state, float volumentaryCounter);
        bool readState (fuelState& state, float& volumentaryCounter);

        inline virtual bool isItemEditable (int item) {
            return item == 5;
        }
};