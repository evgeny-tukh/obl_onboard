#pragma once

#include "../wui/ListCtrlWrapper.h"
#include "../wui/GenericControlWrapper.h"

class BaseListCtrl: public CListCtrlWrapper {
    public:
        BaseListCtrl (HWND parent, UINT controlID): CListCtrlWrapper (parent, controlID) {}
        virtual ~BaseListCtrl () {}

        virtual void init () {}
        bool editValue (int item, BaseListCtrl *labelHolder);
        virtual bool isItemEditable (int item) {
            return false;
        }
        inline void checkSelection (int item) {
            if (GetSelectedItem () != item)
                SetItemState (item, LVIS_SELECTED, LVIS_SELECTED);
        }

        inline BaseListCtrl *CreateControl (int x, int y, int width, int height) {
            BaseListCtrl *ctrl = (BaseListCtrl *) CGenericControlWrapper::CreateControl (x, y, width, height, WS_BORDER | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL);
        
            ctrl->SendMessage (LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
            ctrl->init ();

            return ctrl;
        }

        inline static char *getFormat (int item) {
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
};