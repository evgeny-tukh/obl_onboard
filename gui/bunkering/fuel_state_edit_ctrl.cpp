#include "fuel_state_edit_ctrl.h"
#include "../../common/tools.h"

FuelStateEditCtrl::FuelStateEditCtrl (HWND parent, UINT ctrlID): CListCtrlWrapper (parent, ctrlID) {
}

FuelStateEditCtrl::~FuelStateEditCtrl () {
}

void FuelStateEditCtrl::showState (fuelState& state) {
    char buffer [100];
    SetItemText (0, 1, ftoa (state.density, buffer, "%.4f"));
    SetItemText (1, 1, ftoa (state.viscosity, buffer, "%.2f"));
    SetItemText (2, 1, ftoa (state.sulphur, buffer, "%.2f"));
    SetItemText (3, 1, ftoa (state.temp, buffer, "%.1f"));
    SetItemText (4, 1, ftoa (state.vcf, buffer, "%.4f"));
    SetItemText (5, 1, ftoa (state.volume, buffer, "%.3f"));
    SetItemText (6, 1, ftoa (state.quantity, buffer, "%.3f"));
    SetItemText (7, 1, ftoa (state.fuelMeter, buffer, "%.3f"));
}

void FuelStateEditCtrl::readState (fuelState& state) {
    char buffer [100];
    for (auto i = 0; i < 7; ++ i) {
        GetItemText (i, 1, buffer, sizeof (buffer));
        float value = (float) atof (buffer);
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
