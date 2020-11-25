#include <stdlib.h>
#include <Windows.h>
#include "resource.h"
#include "wui/WindowWrapper.h"
#include "wui/ButtonWrapper.h"
#include "bunkering.h"
#include "../common/TimeFunctions.h"

void setData (HWND dialog, float data, uint32_t ctrlID) {
    char buffer [100];
    sprintf (buffer, "%.1f", data);
    SetDlgItemText (dialog, ctrlID, buffer);
}

void setDateTime (HWND dialog, time_t timestamp, uint32_t dateCtrlID, uint32_t timeCtrlID) {
    SYSTEMTIME date, time;
    TimetToSystemTime (timestamp, & date);
    time = date;
    auto res = SendDlgItemMessage (dialog, dateCtrlID, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM) & date);
    res = SendDlgItemMessage (dialog, timeCtrlID, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM) & time);
}

void initBunkeringEditor (HWND dialog, bunkeringData *data) {
    SetWindowLongPtr (dialog, GWLP_USERDATA, (LPARAM) data);
    setData (dialog, data->density, ID_DENSITY);
    setData (dialog, data->viscosity, ID_VISCOSITY);
    setData (dialog, data->sulphur, ID_SULPHUR);
    setData (dialog, data->temp, ID_TEMPERATURE);
    setData (dialog, data->volume, ID_VOLUME);
    setData (dialog, data->quantity, ID_QUANTITY);
    setDateTime (dialog, data->begin, ID_BEGIN_DATE, ID_BEGIN_TIME);
    setDateTime (dialog, data->end, ID_END_DATE, ID_END_TIME);
}

float getData (HWND dialog, uint32_t ctrlID) {
    char buffer [100];
    GetDlgItemText (dialog, ctrlID, buffer, sizeof (buffer));
    return (float) atof (buffer);
}

time_t getDateTime (HWND dialog, uint32_t dateCtrlID, uint32_t timeCtrlID) {
    SYSTEMTIME date, time;
    SendDlgItemMessage (dialog, dateCtrlID, DTM_GETSYSTEMTIME, 0, (LPARAM) & date);
    SendDlgItemMessage (dialog, timeCtrlID, DTM_GETSYSTEMTIME, 0, (LPARAM) & time);
    time.wYear = date.wYear;
    time.wMonth = date.wMonth;
    time.wDay = date.wDay;
    return SystemTimeToTimet (& time);
}

bool saveBunkeringData (HWND dialog) {
    auto incompleteDataMsg = [dialog] () {
        MessageBox (dialog, "Incomplete data", "Error", MB_ICONSTOP);
    };

    bunkeringData *data = (bunkeringData *) GetWindowLongPtr (dialog, GWLP_USERDATA);

    if (GetDlgItemText (dialog, ID_PORT, data->port, sizeof (data->port)) == 0) {
        SetFocus (GetDlgItem (dialog, ID_PORT)); 
        incompleteDataMsg ();
        return false;
    }
    if (GetDlgItemText (dialog, ID_BARGE, data->barge, sizeof (data->barge)) == 0) {
        SetFocus (GetDlgItem (dialog, ID_BARGE));
        incompleteDataMsg ();
        return false;
    }

    data->density = getData (dialog, ID_DENSITY);
    data->viscosity = getData (dialog, ID_VISCOSITY);
    data->sulphur = getData (dialog, ID_SULPHUR);
    data->temp = getData (dialog, ID_TEMPERATURE);
    data->volume = getData (dialog, ID_VOLUME);
    data->quantity = getData (dialog, ID_QUANTITY);
    data->begin = getDateTime (dialog, ID_BEGIN_DATE, ID_BEGIN_TIME);
    data->end = getDateTime (dialog, ID_END_DATE, ID_END_TIME);

    if (data->quantity < 1.0f || data->volume < 0.5f) {
        incompleteDataMsg (); return false;
    }

    return true;
}

INT_PTR CALLBACK bunkeringEditorProc (HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_COMMAND: {
            auto cmd = LOWORD (wParam);
            switch (cmd) {
                case IDOK:
                    if (!saveBunkeringData (dialog)) return TRUE;
                case IDCANCEL:
                    EndDialog (dialog, cmd); break;
            }
            break;
        }
        case WM_INITDIALOG: {
            initBunkeringEditor (dialog, (bunkeringData *) lParam); break;
        }
        default:
        {
            return FALSE;
        }
    }
    return TRUE;
}

int openBunkeringEditor (HINSTANCE instance, HWND parent, bunkeringData *data) {
    return DialogBoxParamA (instance, MAKEINTRESOURCE (IDD_BUNKERING), parent, bunkeringEditorProc, (LPARAM) data);
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
