#include <stdlib.h>
#include <Windows.h>
#include "resource.h"
#include "wui/WindowWrapper.h"
#include "wui/ButtonWrapper.h"
#include "bunkering.h"

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
