#include <stdlib.h>
#include <Windows.h>
#include "resource.h"
#include "wui/WindowWrapper.h"
#include "wui/ButtonWrapper.h"
#include "bunkering.h"

INT_PTR CALLBACK bunkeringEditorProc (HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_COMMAND: {
            auto cmd = LOWORD (wParam);
            switch (cmd) {
                case IDOK:
                case IDCANCEL:
                    EndDialog (dialog, cmd); break;
            }
            break;
        }
        case WM_INITDIALOG: {
            char buffer [100];
            bunkeringData *data = (bunkeringData *) lParam;

            sprintf (buffer, "%.1f", data->density);
            SetDlgItemText (dialog, ID_DENSITY, buffer);
            sprintf (buffer, "%.1f", data->viscosity);
            SetDlgItemText (dialog, ID_VISCOSITY, buffer);
            sprintf (buffer, "%.1f", data->sulphur);
            SetDlgItemText (dialog, ID_SULPHUR, buffer);
            sprintf (buffer, "%.1f", data->temp);
            SetDlgItemText (dialog, ID_TEMPERATURE, buffer);
            sprintf (buffer, "%.1f", data->volume);
            SetDlgItemText (dialog, ID_VOLUME, buffer);
            sprintf (buffer, "%.1f", data->quantity);
            SetDlgItemText (dialog, ID_QUANTITY, buffer);

            SetWindowLongPtr (dialog, GWLP_USERDATA, lParam);
            return TRUE;
        }
        default:
        {
            return FALSE;
        }
    }
    return TRUE;
}

void openBunkeringEditor (HINSTANCE instance, HWND parent, bunkeringData *data) {
    DialogBoxParamA (instance, MAKEINTRESOURCE (IDD_BUNKERING), parent, bunkeringEditorProc, (LPARAM) data);
}

BunkeringEditor::BunkeringEditor (HINSTANCE instance, HWND parent): CWindowWrapper (instance, parent, "obl_be") {
    Create ("Бункировка", 100, 100, 600, 500, WS_VISIBLE | WS_POPUP | WS_DLGFRAME | WS_CAPTION | WS_SYSMENU);
}

BunkeringEditor::~BunkeringEditor () {
    delete ok;
    delete cancel;
    delete densityLabel;
}

LRESULT BunkeringEditor::OnMessage (UINT message, WPARAM wParam, LPARAM lParam) {
    return CWindowWrapper::OnMessage (message,wParam, lParam);
}

LRESULT BunkeringEditor::OnCommand (WPARAM wParam, LPARAM lParam) {
    switch (LOWORD (wParam))
    {
        case IDCANCEL:
        {
            Destroy (); break;
        }
    }

    return CWindowWrapper::OnCommand (wParam, lParam);
}

void BunkeringEditor::OnCreate () {
    RECT client;

    ok = new CButtonWrapper (m_hwndHandle, IDOK);
    cancel = new CButtonWrapper (m_hwndHandle, IDCANCEL);

    densityLabel = new CStaticWrapper (m_hwndHandle, IDC_STATIC);

    GetClientRect (& client);

    densityLabel->CreateControl (10, 10, 100, 25, WS_VISIBLE | SS_LEFT, "Плотность при 15 гр., кг/м.куб.");

    ok->CreateControl (10, client.bottom - 50, 80, 30, WS_VISIBLE | BS_PUSHBUTTON, "Сохранить");
    cancel->CreateControl (100, client.bottom - 50, 80, 30, WS_VISIBLE | BS_PUSHBUTTON, "Отмена");
}
