#include <stdlib.h>
#include <Windows.h>
#include "bunkering_wnd.h"
#include "bunkering_edit.h"
#include "../resource.h"
#include "../../common/tools.h"
#include "fuel_state_edit_ctrl.h"

BunkeringWindow::BunkeringWindow (HINSTANCE instance, HWND parent, config& _cfg, database& _db):
    CWindowWrapper (instance, parent, "obl_bnk_wnd"), cfg (_cfg), db (_db),
    bunkerList (0), addBunker (0), removeBunker (0), bunkerLoadInfo (0), bunkerData (0), bunkeringLabel (0),
    beforeLabel (0), afterLabel (0), tankInfoBefore (0), tankInfoAfter (0),
    draftForeBeforeLabel (0), draftForeAfterLabel (0), draftAftBeforeLabel (0), draftAftAfterLabel (0),
    draftForeBefore (0), draftForeAfter (0), draftAftBefore (0), draftAftAfter (0) {}

BunkeringWindow::~BunkeringWindow () {
    delete bunkerList;
    delete addBunker, removeBunker;
    delete tabSwitch;
    #if 0
    delete bunkerLoadInfo, tankInfoBefore, tankInfoAfter;
    delete bunkeringLabel, beginLabel, endLabel, portLabel, bargeLabel, beforeLabel, afterLabel;
    delete beginDate, beginTime, endDate, endTime;
    delete save, discard;
    delete tankList;
    delete port, barge;
    delete draftForeAfterLabel, draftForeBeforeLabel, draftAftAfterLabel, draftAftBeforeLabel;
    delete draftForeAfter, draftForeBefore, draftAftAfter, draftAftBefore;
    #endif
}

void BunkeringWindow::OnCreate () {
    RECT client;

    GetClientRect (& client);

    bunkerList = new CListCtrlWrapper (m_hwndHandle, ID_BUNKER_LIST);

    int bunkerListWidth = min (client.right - BUTTON_WIDTH, 480);
    int buttonWidth = client.right - bunkerListWidth;

    bunkerList->CreateControl (0, 0, bunkerListWidth, BUNK_LIST_HEIGHT, LVS_REPORT | WS_VISIBLE | WS_BORDER, 0);
    bunkerList->AddColumn ("Начало закачки", 110);
    bunkerList->AddColumn ("Конец закачки", 110);
    bunkerList->AddColumn ("Закачано по ОБР, т", 120);
    bunkerList->AddColumn ("Закачано по УМ, т", 120);

    addBunker = new CButtonWrapper (m_hwndHandle, ID_NEW_BUNKERING);
    removeBunker = new CButtonWrapper (m_hwndHandle, ID_DELETE_BUNKERING);

    addBunker->CreateControl (client.right - buttonWidth, 0, buttonWidth, BUTTON_HEIGHT, WS_VISIBLE, "Новая бункеровка");
    removeBunker->CreateControl (client.right - buttonWidth, BUTTON_HEIGHT, buttonWidth, BUTTON_HEIGHT, WS_VISIBLE, "Удалить бункеровку");

    tabSwitch = new CTabCtrlWrapper (m_hwndHandle, ID_BUNKERING_TABS);

    tabSwitch->CreateControl (0, BUNK_LIST_HEIGHT, bunkerListWidth, client.bottom - BUNK_LIST_HEIGHT, TCS_BUTTONS);
    tabSwitch->AddItem ("Общая информация", 0);
    tabSwitch->AddItem ("До закачки", 1);
    tabSwitch->AddItem ("После закачки", 2);

    auto switchHandle = tabSwitch->GetHandle ();

    addControlToGroup (0, bunkerLoadInfo = new FuelStateEditCtrl (switchHandle, ID_BUNK_HDR_FUEL_STATE))->CreateControl (0, 30, 262, 150, WS_BORDER | LVS_REPORT);
    addControlToGroup (0, beginLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 185, 55, 20, SS_LEFT, "Начало");
    addControlToGroup (0, endLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 205, 55, 20, SS_LEFT, "Конец");
    addControlToGroup (0, portLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 225, 55, 20, SS_LEFT, "Порт");
    addControlToGroup (0, bargeLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 245, 55, 20, SS_LEFT, "Баржа");
    addControlToGroup (0, port = new CEditWrapper (switchHandle, ID_PORT))->CreateControl (60, 225, 180, 20, SS_EDITCONTROL | WS_BORDER);
    addControlToGroup (0, barge = new CEditWrapper (switchHandle, ID_BARGE))->CreateControl (60, 245, 180, 20, SS_EDITCONTROL | WS_BORDER);
    addControlToGroup (0, beginDate = new CDateTimePickerWrapper (switchHandle, ID_BEGIN_DATE))->CreateControl (60, 185, 100, 20, DTS_SHORTDATECENTURYFORMAT | DTS_UPDOWN);
    addControlToGroup (0, beginTime = new CDateTimePickerWrapper (switchHandle, ID_BEGIN_TIME))->CreateControl (160, 185, 80, 20, DTS_TIMEFORMAT | DTS_UPDOWN);
    addControlToGroup (0, endDate = new CDateTimePickerWrapper (switchHandle, ID_END_DATE))->CreateControl (60, 205, 100, 20, DTS_SHORTDATECENTURYFORMAT | DTS_UPDOWN);
    addControlToGroup (0, endTime = new CDateTimePickerWrapper (switchHandle, ID_END_TIME))->CreateControl (160, 205, 80, 20, DTS_TIMEFORMAT | DTS_UPDOWN);

    addControlToGroup (1, draftForeBeforeLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 30, 120, 20, SS_LEFT, "Осадка в носу");
    addControlToGroup (1, draftAftBeforeLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 50, 120, 20, SS_LEFT, "Осадка в корме");
    addControlToGroup (1, draftForeBefore = new CEditWrapper (switchHandle, ID_DRAFT_FORE_BEFORE))->CreateControl (125, 30, 80, 20, WS_BORDER);
    addControlToGroup (1, draftAftBefore = new CEditWrapper (switchHandle, ID_DRAFT_AFT_BEFORE))->CreateControl (125, 50, 80, 20, WS_BORDER);
    addControlToGroup (1, tankInfoBefore = new FuelStateEditCtrl (switchHandle, ID_TANK_INFO_BEFORE))->CreateControl (0, 70, 260, 150, WS_BORDER | LVS_REPORT);

    addControlToGroup (2, draftForeAfterLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 30, 120, 20, SS_LEFT, "Осадка в носу");
    addControlToGroup (2, draftAftAfterLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 50, 120, 20, SS_LEFT, "Осадка в корме");
    addControlToGroup (2, draftForeAfter = new CEditWrapper (switchHandle, ID_DRAFT_FORE_AFTER))->CreateControl (125, 30, 80, 20, WS_BORDER);
    addControlToGroup (2, draftAftAfter = new CEditWrapper (switchHandle, ID_DRAFT_AFT_AFTER))->CreateControl (125, 50, 80, 20, WS_BORDER);
    addControlToGroup (2, tankInfoAfter = new FuelStateEditCtrl (switchHandle, ID_TANK_INFO_AFTER))->CreateControl (0, 70, 260, 150, WS_BORDER | LVS_REPORT);

    bunkerLoadInfo->init ();
    tankInfoBefore->init ();
    tankInfoAfter->init ();

    showControlGroup (0);
    #if 0
    bunkeringLabel = new CStaticWrapper (m_hwndHandle, IDC_STATIC);
    beginLabel = new CStaticWrapper (m_hwndHandle, IDC_STATIC);
    endLabel = new CStaticWrapper (m_hwndHandle, IDC_STATIC);
    portLabel = new CStaticWrapper (m_hwndHandle, IDC_STATIC);
    bargeLabel = new CStaticWrapper (m_hwndHandle, IDC_STATIC);
    beforeLabel = new CStaticWrapper (m_hwndHandle, IDC_STATIC);
    afterLabel = new CStaticWrapper (m_hwndHandle, IDC_STATIC);
    draftAftBeforeLabel = new CStaticWrapper (m_hwndHandle, IDC_STATIC);
    draftForeBeforeLabel = new CStaticWrapper (m_hwndHandle, IDC_STATIC);
    draftAftAfterLabel = new CStaticWrapper (m_hwndHandle, IDC_STATIC);
    draftForeAfterLabel = new CStaticWrapper (m_hwndHandle, IDC_STATIC);

    bunkeringLabel->CreateControl (client.right - buttonWidth, BUTTON_HEIGHT * 2, buttonWidth, 20, SS_CENTER, "Информация по закачке");
    beginLabel->CreateControl (client.right - buttonWidth, 180 + BUTTON_HEIGHT * 2, 60, 20, SS_LEFT, "Начало");
    endLabel->CreateControl (client.right - buttonWidth, 200 + BUTTON_HEIGHT * 2, 60, 20, SS_LEFT, "Конец");
    portLabel->CreateControl (client.right - buttonWidth, 220 + BUTTON_HEIGHT * 2, 60, 20, SS_LEFT, "Порт");
    bargeLabel->CreateControl (client.right - buttonWidth, 240 + BUTTON_HEIGHT * 2, 60, 20, SS_LEFT, "Баржа");

    port = new CEditWrapper (m_hwndHandle, ID_PORT);
    barge = new CEditWrapper (m_hwndHandle, ID_BARGE);

    port->CreateControl (client.right - buttonWidth + 60, 220 + BUTTON_HEIGHT * 2, buttonWidth - 60, 20);
    barge->CreateControl (client.right - buttonWidth + 60, 240 + BUTTON_HEIGHT * 2, buttonWidth - 60, 20);

    bunkerLoadInfo = new FuelStateEditCtrl (m_hwndHandle, ID_BUNK_HDR_FUEL_STATE);

    bunkerLoadInfo->CreateControl (client.right - buttonWidth, 20 + BUTTON_HEIGHT * 2, buttonWidth, 160, WS_BORDER | LVS_REPORT);
    bunkerLoadInfo->init ();

    beginDate = new CDateTimePickerWrapper (m_hwndHandle, ID_BEGIN_DATE);
    beginTime = new CDateTimePickerWrapper (m_hwndHandle, ID_BEGIN_TIME);
    endDate = new CDateTimePickerWrapper (m_hwndHandle, ID_END_DATE);
    endTime = new CDateTimePickerWrapper (m_hwndHandle, ID_END_TIME);

    beginDate->CreateControl (client.right - buttonWidth + 60, 180 + BUTTON_HEIGHT * 2, 80, 20, DTS_SHORTDATECENTURYFORMAT | DTS_UPDOWN);
    endDate->CreateControl (client.right - buttonWidth + 60, 200 + BUTTON_HEIGHT * 2, 80, 20, DTS_SHORTDATECENTURYFORMAT | DTS_UPDOWN);
    beginTime->CreateControl (client.right - buttonWidth + 140, 180 + BUTTON_HEIGHT * 2, 70, 20, DTS_TIMEFORMAT | DTS_UPDOWN);
    endTime->CreateControl (client.right - buttonWidth + 140, 200 + BUTTON_HEIGHT * 2, 70, 20, DTS_TIMEFORMAT | DTS_UPDOWN);

    save = new CButtonWrapper (m_hwndHandle, IDOK);
    discard = new CButtonWrapper (m_hwndHandle, IDCANCEL);

    save->CreateControl (client.right - buttonWidth + 210, 180 + BUTTON_HEIGHT * 2, buttonWidth - 210, 20, WS_VISIBLE, "Сохранить");
    discard->CreateControl (client.right - buttonWidth + 210, 200 + BUTTON_HEIGHT * 2, buttonWidth - 210, 20, WS_VISIBLE, "Сбросить");

    tankList = new CListBoxWrapper (m_hwndHandle, ID_TANK_SELECTOR);

    tankList->CreateControl (0, BUNK_LIST_HEIGHT, 100, client.bottom - BUNK_LIST_HEIGHT, WS_BORDER);

    for (auto& tank: cfg.tanks) {
        tankList->AddString (tank.name.c_str (), tank.id);
    }

    beforeLabel->CreateControl (100, BUNK_LIST_HEIGHT, 260, 20, SS_LEFT, "До закачки");

    tankInfoBefore = new FuelStateEditCtrl (m_hwndHandle, ID_TANK_INFO_BEFORE);
    tankInfoAfter = new FuelStateEditCtrl (m_hwndHandle, ID_TANK_INFO_AFTER);

    tankInfoBefore->CreateControl (100, 20 + BUNK_LIST_HEIGHT, 260, 150, WS_BORDER | LVS_REPORT);
    tankInfoBefore->init ();
    draftForeBeforeLabel->CreateControl (100, 170 + BUNK_LIST_HEIGHT, 120, 20, SS_LEFT, "Осадка в носу");
    draftAftBeforeLabel->CreateControl (100, 190 + BUNK_LIST_HEIGHT, 120, 20, SS_LEFT, "Осадка в корме");

    draftForeBefore = new CEditWrapper (m_hwndHandle, ID_DRAFT_FORE_BEFORE);
    draftForeAfter = new CEditWrapper (m_hwndHandle, ID_DRAFT_FORE_AFTER);
    draftAftBefore = new CEditWrapper (m_hwndHandle, ID_DRAFT_AFT_BEFORE);
    draftAftAfter = new CEditWrapper (m_hwndHandle, ID_DRAFT_AFT_AFTER);

    draftForeBefore->CreateControl (220, 170 + BUNK_LIST_HEIGHT, 80, 20, WS_BORDER);
    draftAftBefore->CreateControl (220, 190 + BUNK_LIST_HEIGHT, 80, 20, WS_BORDER);
    #endif
}

LRESULT BunkeringWindow::OnSize (const DWORD requestType, const WORD width, const WORD height)
{
    int bunkerListWidth = min (width - BUTTON_WIDTH, 480);
    int buttonWidth = width - bunkerListWidth;

    bunkerList->Move (0, 0, bunkerListWidth, BUNK_LIST_HEIGHT, TRUE);
    addBunker->Move (width - buttonWidth, 0, buttonWidth, BUTTON_HEIGHT, TRUE);
    removeBunker->Move (width - buttonWidth, BUTTON_HEIGHT, buttonWidth, BUTTON_HEIGHT, TRUE);

    return FALSE;
}

LRESULT BunkeringWindow::OnCommand (WPARAM wParam, LPARAM lParam) {
    LRESULT result = TRUE;

    #if 0
    switch (LOWORD (wParam)) {
        case ID_DELETE_BUNKERING: {
            int selection = bunkerList->GetSelectedItem ();

            if (selection >= 0) {
                uint32_t bunkeringID = bunkerList->GetItemData (selection);
    
                if (MessageBox ("Удалить информацию по бункеровке?", "Удаление", MB_ICONQUESTION | MB_YESNO) == IDYES) {
                    db.deleteBunkering (bunkeringID);
                    loadBunkeringList ();
                }
            }
            break;
        }
        case ID_EDIT_BUNKERING: {
            int selection = bunkerList->GetSelectedItem ();

            if (selection >= 0) {
                uint32_t bunkeringID = bunkerList->GetItemData (selection);
                bunkeringData data;
                if (db.getBunkering (bunkeringID, data) && openBunkeringEditor (m_hInstance, m_hwndHandle, & data) == IDOK) {
                    db.saveBunkering (data);
                    loadBunkeringList ();
                }
            }
            break;
        }
        case ID_NEW_BUNKERING: {
            /*if (selectedTank > 0) {
                bunkeringData data;
                data.tank = selectedTank;
                if (openBunkeringEditor (m_hInstance, m_hwndHandle, & data) == IDOK) {
                    auto bunkeringID = db.createBunkering (data);
                    loadBunkeringList ();
                }
            } else {
                MessageBox ("Пожалуйста, выберите танк!", "Ошибка", MB_ICONEXCLAMATION);
            }*/
            
            break;
        }
        default:
        {
            result = TRUE;
        }
    }
    #endif

    return result;
}

void BunkeringWindow::loadBunkeringList ()
{
    bunkeringList list;

    db.loadBunkeringList (-1, list, 0, time (0));

    bunkerList->DeleteAllItems ();

    for (auto bunkering = list.begin (); bunkering != list.end (); ++ bunkering) {
        char buffer [50];

        auto item = bunkerList->AddItem (formatTimestamp (bunkering->begin, buffer), bunkering->id);

        bunkerList->SetItemText (item, 1, formatTimestamp (bunkering->end, buffer));
        bunkerList->SetItemText (item, 2, ftoa (bunkering->loaded.volume, buffer, "%.13"));
        bunkerList->SetItemText (item,3, ftoa (bunkering->loaded.fuelMeter, buffer, "%.13"));
    }
}

void BunkeringWindow::showControlGroup (int groupToShow) {
    for (auto i = 0; i < controlGroups.size (); ++ i) {
        for (auto ctrl: controlGroups.at (i)) {
            ctrl->Show (groupToShow == i ? SW_SHOW : SW_HIDE);
        }
    }
}

LRESULT BunkeringWindow::OnNotify (NMHDR *header)
{
    switch (header->idFrom) {
        case ID_BUNKERING_TABS: {
            auto selection = tabSwitch->GetCurSel ();

            showControlGroup (selection); break;
        }
    }
    return FALSE;
}

