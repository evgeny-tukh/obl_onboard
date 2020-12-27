#include "hw_data_edit_ctrl.h"
#include "../../common/tools.h"
#include "../wui/InputBox.h"

HwDataEditCtrl::HwDataEditCtrl (HWND parent, UINT ctrlID, bool showFcValues): BaseListCtrl (parent, ctrlID), showFuelCounterValues (showFcValues) {
}

HwDataEditCtrl::~HwDataEditCtrl () {
}

void HwDataEditCtrl::showState (fuelState& state) {
    char buffer [100];
    SetItemText (0, 0, ftoa (state.density.byVolume, buffer, getFormat (0)));
    SetItemText (3, 0, ftoa (state.temp.byVolume, buffer, getFormat (3)));
    SetItemText (4, 0, ftoa (state.vcf.byVolume, buffer, getFormat (4)));
    SetItemText (5, 0, ftoa (state.volume.byVolume, buffer, getFormat (5)));
    SetItemText (6, 0, ftoa (state.quantity.byVolume, buffer, getFormat (6)));

    if (showFuelCounterValues) {
        SetItemText (0, 1, ftoa (state.density.byCounter, buffer, getFormat (0)));
        SetItemText (3, 1, ftoa (state.temp.byCounter, buffer, getFormat (3)));
        SetItemText (4, 1, ftoa (state.vcf.byCounter, buffer, getFormat (4)));
        SetItemText (5, 1, ftoa (state.volume.byCounter, buffer, getFormat (5)));
        SetItemText (6, 1, ftoa (state.quantity.byCounter, buffer, getFormat (6)));
    }
}

bool HwDataEditCtrl::readState (fuelState& state) {
    char buffer [100];

    auto getValue = [this, &buffer] (int item, int column, float& value) {
        bool result = true;

        GetItemText (item, column, buffer, sizeof (buffer));        
        
        value = (float) atof (buffer);

        //if (value < 0.01f) result = false;

        return result;
    };

    bool result = (
        getValue (0, 0, state.density.byVolume) &&
        getValue (3, 0, state.temp.byVolume) &&
        getValue (4, 0, state.vcf.byVolume) &&
        getValue (5, 0, state.volume.byVolume) &&
        getValue (6, 0, state.quantity.byVolume)
    );

    if (showFuelCounterValues) result = result && (
        getValue (0, 1, state.density.byCounter) &&
        getValue (3, 1, state.temp.byCounter) &&
        getValue (4, 1, state.vcf.byCounter) &&
        getValue (5, 1, state.volume.byCounter) &&
        getValue (6, 1, state.quantity.byCounter)
    );

    return result;
}

void HwDataEditCtrl::init () {
    SendMessage (LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

    AddColumn ("Уровнемер", 80);

    if (showFuelCounterValues) AddColumn ("Расходомер", 80);

    for (auto i = 0; i < 8; AddItem (""), ++ i);
}
