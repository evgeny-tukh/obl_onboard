#include "base_list_ctrl.h"
#include "../../common/tools.h"
#include "../wui/InputBox.h"

bool BaseListCtrl::editValue (int item, BaseListCtrl *labelHolder) {
    bool result = false;
    
    if (isItemEditable (item)) {
        char buffer [100];
        char label [50];
        GetItemText (item, 1, buffer, sizeof (buffer));
        labelHolder->GetItemText (item, 0, label, sizeof (label));

        CInputBox inputBox (m_hInstance, m_hwndHandle, "Редактирование параметра", label, buffer, sizeof (buffer));

        result = inputBox.Execute () == IDOK;

        if (result) {
            SetItemText (item, 1, ftoa (atof (buffer), buffer, getFormat (item)));
        }
    }

    return result;
}