#include "hw_data_edit_ctrl.h"
#include "../../common/tools.h"
#include "../wui/InputBox.h"

HwDataEditCtrl::HwDataEditCtrl (HWND parent, UINT ctrlID): BaseListCtrl (parent, ctrlID) {
}

HwDataEditCtrl::~HwDataEditCtrl () {
}

void HwDataEditCtrl::showState (fuelState& state) {
    char buffer [100];
    SetItemText (5, 0, ftoa (state.volume.byVolume, buffer, getFormat (5)));
    SetItemText (5, 1, ftoa (state.volume.byCounter, buffer, getFormat (5)));
    SetItemText (6, 0, ftoa (state.quantity.byVolume, buffer, getFormat (6)));
    SetItemText (6, 1, ftoa (state.quantity.byCounter, buffer, getFormat (6)));
}

bool HwDataEditCtrl::readState (fuelState& state) {
    char buffer [100];

    auto getValue = [this, &buffer] (int item, int column, float& value) {
        bool result = true;

        GetItemText (item, column, buffer, sizeof (buffer));        
        
        value = (float) atof (buffer);

        if (value < 0.01f) result = false;

        return result;
    };

    return (
        getValue (5, 0, state.volume.byVolume) &&
        getValue (5, 1, state.volume.byCounter) &&
        getValue (6, 0, state.quantity.byVolume) &&
        getValue (6, 1, state.quantity.byCounter)
    );
}

void HwDataEditCtrl::init () {
    SendMessage (LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

    AddColumn ("Уровнемер", 80);
    AddColumn ("Расходомер", 80);

    for (auto i = 0; i < 8; AddItem (""), ++ i);
}
