#include "bunkering_header.h"
#include "../resource.h"

BunkeringHeaderWindow::BunkeringHeaderWindow (HINSTANCE instance, HWND parent, config& _cfg):
    CWindowWrapper (instance, parent, "obl_bnk_hdr_wnd"), cfg (_cfg), bunkerData (0), fuelState (0) {
}

BunkeringHeaderWindow::~BunkeringHeaderWindow () {
    delete fuelState;
}

void BunkeringHeaderWindow::OnCreate () {
    fuelState = new FuelStateEditCtrl (m_hwndHandle, ID_BUNK_HDR_FUEL_STATE);
}
