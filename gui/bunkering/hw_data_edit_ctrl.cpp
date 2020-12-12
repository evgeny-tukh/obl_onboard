#include "hw_data_edit_ctrl.h"
#include "../../common/tools.h"
#include "../wui/InputBox.h"

HwDataEditCtrl::HwDataEditCtrl (HWND parent, UINT ctrlID): BaseListCtrl (parent, ctrlID) {
}

HwDataEditCtrl::~HwDataEditCtrl () {
}

void HwDataEditCtrl::showState (fuelState& state, float volumentaryCounter) {
    char buffer [100];
    //SetItemText (0, 1, ftoa (state.density, buffer, getFormat (0)));
    //SetItemText (4, 1, ftoa (state.vcf, buffer, getFormat (4)));
    SetItemText (5, 1, ftoa (state.volume, buffer, getFormat (5)));
    //SetItemText (6, 1, ftoa (state.quantity, buffer, getFormat (6)));
}

bool HwDataEditCtrl::readState (fuelState& state, float& volumentaryCounter) {
    char buffer [100];

    auto getValue = [this, &buffer] (int item, float& value) {
        bool result = true;

        GetItemText (item, 0, buffer, sizeof (buffer));        
        
        value = (float) atof (buffer);

        if (value < 0.01f) result = false;

        return result;
    };

    return getValue (5, volumentaryCounter);
    /*for (auto i = 0; i < 7; ++ i) {
        GetItemText (i, 1, buffer, sizeof (buffer));
        float value = (float) atof (buffer);
        if (value < 0.01f) result = false;
        switch (i) {
            case 0: state.density = value; break;
            case 1: state.viscosity = value; break;
            case 2: state.sulphur = value; break;
            case 3: state.temp = value; break;
            case 4: state.vcf = value; break;
            case 5: state.volume = value; break;
            case 6: state.quantity = value; break;
            case 7: state.fuelMeter = value; break;
        }
    }

    return result;*/
}

void HwDataEditCtrl::init () {
    SendMessage (LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

    AddColumn ("Уровнемер", 80);
    AddColumn ("Расходомер", 80);

    for (auto i = 0; i < 8; AddItem (""), ++ i);
}
