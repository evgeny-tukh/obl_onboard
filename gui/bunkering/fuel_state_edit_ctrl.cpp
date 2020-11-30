#include "fuel_state_edit_ctrl.h"
#include "../../common/tools.h"
#include "../wui/InputBox.h"

FuelStateEditCtrl::FuelStateEditCtrl (HWND parent, UINT ctrlID): CListCtrlWrapper (parent, ctrlID) {
}

FuelStateEditCtrl::~FuelStateEditCtrl () {
}

char *FuelStateEditCtrl::getFormat (int item) {
    switch (item) {
        case 0: return "%.4f";
        case 1: return "%.2f";
        case 2: return "%.2f";
        case 3: return "%.1f";
        case 4: return "%.4f";
        case 5: return "%.3f";
        case 6: return "%.3f";
        case 7: return "%.3f";
        default: return "%.f";
    }
}

void FuelStateEditCtrl::showState (fuelState& state) {
    char buffer [100];
    SetItemText (0, 1, ftoa (state.density, buffer, getFormat (0)));
    SetItemText (1, 1, ftoa (state.viscosity, buffer, getFormat (1)));
    SetItemText (2, 1, ftoa (state.sulphur, buffer, getFormat (2)));
    SetItemText (3, 1, ftoa (state.temp, buffer, getFormat (3)));
    SetItemText (4, 1, ftoa (state.vcf, buffer, getFormat (4)));
    SetItemText (5, 1, ftoa (state.volume, buffer, getFormat (5)));
    SetItemText (6, 1, ftoa (state.quantity, buffer, getFormat (6)));
    SetItemText (7, 1, ftoa (state.fuelMeter, buffer, getFormat (7)));
}

bool FuelStateEditCtrl::readState (fuelState& state) {
    bool result = true;
    char buffer [100];
    for (auto i = 0; i < 7; ++ i) {
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

bool FuelStateEditCtrl::editValue (int item) {
    char buffer [100];
    char label [50];
    GetItemText (item, 1, buffer, sizeof (buffer));
    GetItemText (item, 0, label, sizeof (label));

    CInputBox inputBox (m_hInstance, m_hwndHandle, "Редактирование параметра", label, buffer, sizeof (buffer));

    bool result = inputBox.Execute () == IDOK;

    if (result) {
        SetItemText (item, 1, ftoa (atof (buffer), buffer, getFormat (item)));
    }

    return result;
}