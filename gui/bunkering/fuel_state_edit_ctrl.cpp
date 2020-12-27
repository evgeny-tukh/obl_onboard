#include "fuel_state_edit_ctrl.h"
#include "../../common/tools.h"
#include "../wui/InputBox.h"

FuelStateEditCtrl::FuelStateEditCtrl (HWND parent, UINT ctrlID): BaseListCtrl (parent, ctrlID) {
}

FuelStateEditCtrl::~FuelStateEditCtrl () {
}

void FuelStateEditCtrl::showState (fuelState& state) {
    char buffer [100];
    SetItemText (0, 1, ftoa (state.density.reported, buffer, getFormat (0)));
    SetItemText (1, 1, ftoa (state.viscosity, buffer, getFormat (1)));
    SetItemText (2, 1, ftoa (state.sulphur, buffer, getFormat (2)));
    SetItemText (3, 1, ftoa (state.temp.reported, buffer, getFormat (3)));
    SetItemText (4, 1, ftoa (state.vcf.reported, buffer, getFormat (4)));
    SetItemText (5, 1, ftoa (state.volume.reported, buffer, getFormat (5)));
    SetItemText (6, 1, ftoa (state.quantity.reported, buffer, getFormat (6)));
}

bool FuelStateEditCtrl::readState (fuelState& state) {
    bool result = true;
    char buffer [100];
    for (auto i = 0; i < 7; ++ i) {
        GetItemText (i, 1, buffer, sizeof (buffer));
        float value = (float) atof (buffer);
        //if (value < 0.01f) result = false;
        switch (i) {
            case 0: state.density.reported = value; break;
            case 1: state.viscosity = value; break;
            case 2: state.sulphur = value; break;
            case 3: state.temp.reported = value; break;
            case 4: state.vcf.reported = value; break;
            case 5: state.volume.reported = value; break;
            case 6: state.quantity.reported = value; break;
        }
    }

    return result;
}

void FuelStateEditCtrl::init () {
    SendMessage (LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

    AddColumn ("Параметер", 180);
    AddColumn ("Значение", 80);

    AddItem ("Плотность при 15°, кг/м.куб.");
    AddItem ("Вязкость при 50°, сСт макс.");
    AddItem ("Содержание серы, %");
    AddItem ("Температура, °");
    AddItem ("V.C.F.");
    AddItem ("Объем, м.куб.");
    AddItem ("Количество, т");
    AddItem ("По расходомеру");
}

