﻿#include <stdlib.h>
#include <Windows.h>
#include "bunkering_wnd.h"
#include "bunkering_edit.h"
#include "../resource.h"
#include "../../common/tools.h"
#include "fuel_state_edit_ctrl.h"

void generateReport (config& cfg, bunkeringData& data);

BunkeringWindow::BunkeringWindow (HINSTANCE instance, HWND parent, config& _cfg, database& _db):
    CWindowWrapper (instance, parent, "obl_bnk_wnd"), cfg (_cfg), db (_db),
    mode (_mode::browseList), editingItem (-1),
    bunkerList (0), addBunker (0), removeBunker (0), editBunker (0), bunkerLoadInfo (0), bunkeringLabel (0),
    beforeLabel (0), afterLabel (0), tanksBefore (0), tanksAfter (0),
    save (0), discard (0), tabSwitch (0), editMode (false), createReport (0),
    draftForeBeforeLabel (0), draftForeAfterLabel (0), draftAftBeforeLabel (0), draftAftAfterLabel (0),
    fmInBeforeLabel (0), fmOutBeforeLabel (0), fmInAfterLabel (0), fmOutAfterLabel (0),
    fmInBefore (0), fmOutBefore (0), fmInAfter (0), fmOutAfter (0),
    draftForeBefore (0), draftForeAfter (0), draftAftBefore (0), draftAftAfter (0) {}

BunkeringWindow::~BunkeringWindow () {
    delete bunkerList;
    delete addBunker, removeBunker, editBunker;
    delete tabSwitch;
    delete tanksBefore, tanksAfter;
    delete save, discard, createReport;
    delete fmInBeforeLabel, fmOutBeforeLabel, fmInAfterLabel, fmOutAfterLabel;
    delete fmInBefore, fmOutBefore, fmInAfter, fmOutAfter;

    for (auto& ctrl: tankInfoBefore) delete ctrl;
    for (auto& ctrl: tankInfoAfter) delete ctrl;
}

void BunkeringWindow::OnCreate () {
    RECT client;

    GetClientRect (& client);

    bunkerList = new CListCtrlWrapper (m_hwndHandle, ID_BUNKER_LIST);

    int bunkerListWidth = min (client.right - BUTTON_WIDTH, 480);
    int buttonWidth = client.right - bunkerListWidth;

    bunkerList->CreateControl (0, 0, bunkerListWidth, BUNK_LIST_HEIGHT, LVS_REPORT | LVS_SHOWSELALWAYS | WS_VISIBLE | WS_BORDER, 0);
    bunkerList->AddColumn ("Начало закачки", 110);
    bunkerList->AddColumn ("Конец закачки", 110);
    bunkerList->AddColumn ("Закачано по ОБР, т", 120);
    bunkerList->AddColumn ("Закачано по УМ, т", 120);
    bunkerList->SendMessage (LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    addBunker = new CButtonWrapper (m_hwndHandle, ID_NEW_BUNKERING);
    removeBunker = new CButtonWrapper (m_hwndHandle, ID_DELETE_BUNKERING);
    createReport = new CButtonWrapper (m_hwndHandle, ID_CREATE_REPORT);
    editBunker = new CButtonWrapper (m_hwndHandle, ID_EDIT_BUNKERING);
    save = new CButtonWrapper (m_hwndHandle, IDOK);
    discard = new CButtonWrapper (m_hwndHandle, IDCANCEL);
    
    addBunker->CreateControl (client.right - buttonWidth, 0, buttonWidth, BUTTON_HEIGHT, WS_VISIBLE, "Новая бункеровка");
    removeBunker->CreateControl (client.right - buttonWidth, BUTTON_HEIGHT, buttonWidth, BUTTON_HEIGHT, WS_VISIBLE, "Удалить бункеровку");
    createReport->CreateControl (client.right - buttonWidth, BUTTON_HEIGHT * 2, buttonWidth, BUTTON_HEIGHT, WS_VISIBLE, "Сгенерировать расписку");
    editBunker->CreateControl (client.right - buttonWidth, BUTTON_HEIGHT * 3, buttonWidth, BUTTON_HEIGHT, WS_VISIBLE, "Изменить бункеровку");
    save->CreateControl (client.right - buttonWidth, BUTTON_HEIGHT * 4, buttonWidth, BUTTON_HEIGHT, WS_VISIBLE, "Сохранить");
    discard->CreateControl (client.right - buttonWidth, BUTTON_HEIGHT * 5, buttonWidth, BUTTON_HEIGHT, WS_VISIBLE, "Сбросить");
    enableButtons (false, false);

    tabSwitch = new CTabCtrlWrapper (m_hwndHandle, ID_BUNKERING_TABS);

    tabSwitch->CreateControl (0, BUNK_LIST_HEIGHT, bunkerListWidth, client.bottom - BUNK_LIST_HEIGHT, TCS_BUTTONS);
    tabSwitch->Show (SW_HIDE);
    tabSwitch->AddItem ("Общая информация", 0);
    tabSwitch->AddItem ("До закачки", 1);
    tabSwitch->AddItem ("После закачки", 2);

    auto switchHandle = tabSwitch->GetHandle ();

    addControlToGroup (0, bunkerLoadInfo = new FuelStateEditCtrl (switchHandle, ID_BUNK_HDR_FUEL_STATE))->CreateControl (0, 30, 262, 150, WS_BORDER | LVS_REPORT);
    addControlToGroup (0, beginLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 185, 55, 20, SS_LEFT, "Начало");
    addControlToGroup (0, endLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 205, 55, 20, SS_LEFT, "Конец");
    addControlToGroup (0, portLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 225, 55, 20, SS_LEFT, "Порт");
    addControlToGroup (0, bargeLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 245, 55, 20, SS_LEFT, "Баржа");
    addControlToGroup (0, port = new CEditWrapper (switchHandle, ID_PORT))->CreateControl (60, 225, 180, 20, WS_BORDER);
    addControlToGroup (0, barge = new CEditWrapper (switchHandle, ID_BARGE))->CreateControl (60, 245, 180, 20, WS_BORDER);
    addControlToGroup (0, beginDate = new CDateTimePickerWrapper (switchHandle, ID_BEGIN_DATE))->CreateControl (60, 185, 100, 20, DTS_SHORTDATECENTURYFORMAT | DTS_UPDOWN);
    addControlToGroup (0, beginTime = new CDateTimePickerWrapper (switchHandle, ID_BEGIN_TIME))->CreateControl (160, 185, 80, 20, DTS_TIMEFORMAT | DTS_UPDOWN);
    addControlToGroup (0, endDate = new CDateTimePickerWrapper (switchHandle, ID_END_DATE))->CreateControl (60, 205, 100, 20, DTS_SHORTDATECENTURYFORMAT | DTS_UPDOWN);
    addControlToGroup (0, endTime = new CDateTimePickerWrapper (switchHandle, ID_END_TIME))->CreateControl (160, 205, 80, 20, DTS_TIMEFORMAT | DTS_UPDOWN);

    addControlToGroup (1, draftForeBeforeLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 30, 120, 20, SS_LEFT, "Осадка в носу");
    addControlToGroup (1, draftAftBeforeLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 50, 120, 20, SS_LEFT, "Осадка в корме");
    addControlToGroup (1, draftForeBefore = new CEditWrapper (switchHandle, ID_DRAFT_FORE_BEFORE))->CreateControl (125, 30, 80, 20, WS_BORDER);
    addControlToGroup (1, draftAftBefore = new CEditWrapper (switchHandle, ID_DRAFT_AFT_BEFORE))->CreateControl (125, 50, 80, 20, WS_BORDER);
    addControlToGroup (1, fmInBeforeLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (220, 30, 150, 20, SS_LEFT, "Р/м входящий до");
    addControlToGroup (1, fmOutBeforeLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (220, 50, 150, 20, SS_LEFT, "Р/м исходящий до");
    addControlToGroup (1, fmInBefore = new CEditWrapper (switchHandle, ID_DRAFT_FORE_AFTER))->CreateControl (370, 30, 80, 20, WS_BORDER);
    addControlToGroup (1, fmOutBefore = new CEditWrapper (switchHandle, ID_DRAFT_FORE_AFTER))->CreateControl (370, 50, 80, 20, WS_BORDER);

    addControlToGroup (2, draftForeAfterLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 30, 120, 20, SS_LEFT, "Осадка в носу");
    addControlToGroup (2, draftAftAfterLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 50, 120, 20, SS_LEFT, "Осадка в корме");
    addControlToGroup (2, draftForeAfter = new CEditWrapper (switchHandle, ID_DRAFT_FORE_AFTER))->CreateControl (125, 30, 80, 20, WS_BORDER);
    addControlToGroup (2, draftAftAfter = new CEditWrapper (switchHandle, ID_DRAFT_AFT_AFTER))->CreateControl (125, 50, 80, 20, WS_BORDER);
    addControlToGroup (2, fmInAfterLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (220, 30, 150, 20, SS_LEFT, "Р/м входящий после");
    addControlToGroup (2, fmOutAfterLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (220, 50, 150, 20, SS_LEFT, "Р/м исходящий после");
    addControlToGroup (2, fmInAfter = new CEditWrapper (switchHandle, ID_DRAFT_FORE_AFTER))->CreateControl (370, 30, 80, 20, WS_BORDER);
    addControlToGroup (2, fmOutAfter = new CEditWrapper (switchHandle, ID_DRAFT_FORE_AFTER))->CreateControl (370, 50, 80, 20, WS_BORDER);

    tanksBefore = new CTabCtrlWrapper (switchHandle, ID_TANKS_BEFORE);
    tanksAfter = new CTabCtrlWrapper (switchHandle, ID_TANKS_AFTER);

    addControlToGroup (1, tanksBefore->CreateControl (0, 80, bunkerListWidth, client.bottom - 80, TCS_BUTTONS));
    addControlToGroup (2, tanksAfter->CreateControl (0, 80, bunkerListWidth, client.bottom - 80, TCS_BUTTONS));

    bunkerLoadInfo->init ();

    auto count = 0;
    FuelStateEditCtrl *tankInfoCtrl;

    for (auto& tankInfo: cfg.tanks) {
        tanksBefore->AddItem ((char *) tankInfo.name.c_str (), tankInfo.id);
        tanksAfter->AddItem ((char *) tankInfo.name.c_str (), tankInfo.id);

        tankInfoCtrl = new FuelStateEditCtrl (tanksBefore->GetHandle (), ID_TANK_INFO_BEFORE_FIRST + count);
        
        tankInfoCtrl->CreateControl (10, 30, 262, 150, WS_BORDER | LVS_REPORT);
        tankInfoCtrl->init ();

        tankInfoBefore.push_back (tankInfoCtrl);

        tankInfoCtrl = new FuelStateEditCtrl (tanksAfter->GetHandle (), ID_TANK_INFO_AFTER_FIRST + count);
        
        tankInfoCtrl->CreateControl (10, 30, 262, 150, WS_BORDER | LVS_REPORT);
        tankInfoCtrl->init ();

        tankInfoAfter.push_back (tankInfoCtrl);

        ++ count;
    }

    showControlGroup (0);
    loadBunkeringList ();
}

LRESULT BunkeringWindow::OnSize (const DWORD requestType, const WORD width, const WORD height)
{
    int bunkerListWidth = min (width - BUTTON_WIDTH, 480);
    int buttonWidth = width - bunkerListWidth;

    bunkerList->Move (0, 0, bunkerListWidth, BUNK_LIST_HEIGHT, TRUE);
    addBunker->Move (width - buttonWidth, 0, buttonWidth, BUTTON_HEIGHT, TRUE);
    removeBunker->Move (width - buttonWidth, BUTTON_HEIGHT, buttonWidth, BUTTON_HEIGHT, TRUE);
    createReport->Move (width - buttonWidth, BUTTON_HEIGHT * 2, buttonWidth, BUTTON_HEIGHT, TRUE);
    editBunker->Move (width - buttonWidth, BUTTON_HEIGHT * 3, buttonWidth, BUTTON_HEIGHT, TRUE);
    save->Move (width - buttonWidth, BUTTON_HEIGHT * 4, buttonWidth, BUTTON_HEIGHT, TRUE);
    discard->Move (width - buttonWidth, BUTTON_HEIGHT * 5, buttonWidth, BUTTON_HEIGHT, TRUE);

    return FALSE;
}

LRESULT BunkeringWindow::OnCommand (WPARAM wParam, LPARAM lParam) {
    LRESULT result = TRUE;

    switch (LOWORD (wParam)) {
        case ID_CREATE_REPORT: {
            int index = bunkerList->GetSelectedItem ();
            if (index >= 0) {
                generateReport (cfg, list [index]);
            }
            break;
        }
        case IDCANCEL: {
            int index = bunkerList->GetSelectedItem ();
            bool selected = index >= 0;
            if (selected) setBunkeringData (list [index]);
            enableButtons (true, false);
            enableEditor (false);
            addBunker->Enable (TRUE);
            mode = _mode::view;
            break;
        }
        case IDOK: {
            bunkeringData bunkData (cfg);
            if (checkData (bunkData)) {
                if (mode == _mode::add) {
                    db.createBunkering (bunkData);
                } else {
                    int selection = bunkerList->GetSelectedItem ();
                    auto& curData = list [selection];
                    bunkData.id = curData.id;
                    for (auto i = 0; i < curData.tankStates.size (); ++ i) {
                        bunkData.tankStates [i].id = curData.tankStates [i].id;
                    }
                    db.saveBunkering (bunkData);
                }
                enableButtons (true, false);
                enableEditor (false);
                loadBunkeringList ();
                addBunker->Enable (TRUE);
                mode = _mode::view;
            }
            break;
        }
        case ID_DELETE_BUNKERING: {
            int selection = bunkerList->GetSelectedItem ();

            if (selection >= 0) {
                auto& bunkData = list [selection];
    
                if (MessageBox ("Удалить информацию по бункеровке?", "Удаление", MB_ICONQUESTION | MB_YESNO) == IDYES) {
                    db.deleteBunkering (bunkData.id);
                    enableButtons (false, false);
                    enableEditor (false);
                    addBunker->Enable (TRUE);
                    loadBunkeringList ();
                    mode = _mode::browseList;
                    editingItem = -1;
                }
            } else {
                MessageBox ("Выберите бункеровку", "Внимание", MB_ICONEXCLAMATION);
            }
            break;
        }
        case ID_EDIT_BUNKERING: {
            int selection = bunkerList->GetSelectedItem ();

            if (selection >= 0) {
                auto& data = list [selection];
                setBunkeringData (data);
                enableButtons (false, true);
                enableEditor (true);
                addBunker->Enable (FALSE);
                mode = _mode::edit;
            } else {
                MessageBox ("Выберите бункеровку", "Внимание", MB_ICONEXCLAMATION);
            }
            break;
        }
        case ID_NEW_BUNKERING: {
            bunkeringData data (cfg);
            setBunkeringData (data);
            enableButtons (false, true);
            enableEditor (true);
            addBunker->Enable (FALSE);
            mode = _mode::add;
            break;
        }
        default: {
            result = TRUE;
        }
    }

    return result;
}

void BunkeringWindow::loadBunkeringList ()
{
    list.clear ();

    db.loadBunkeringList (-1, list, 0, time (0));

    bunkerList->DeleteAllItems ();

    for (auto bunkering = list.begin (); bunkering != list.end (); ++ bunkering) {
        char buffer [50];

        auto item = bunkerList->AddItem (formatTimestamp (bunkering->begin, buffer), bunkering - list.begin ());

        bunkerList->SetItemText (item, 1, formatTimestamp (bunkering->end, buffer));
        bunkerList->SetItemText (item, 2, ftoa (bunkering->loaded.volume, buffer, "%.3f"));
        bunkerList->SetItemText (item,3, ftoa (bunkering->loaded.fuelMeter, buffer, "%.3f"));
    }

    editingItem = -1;
}

void BunkeringWindow::showControlGroup (int groupToShow) {
    for (auto i = 0; i < controlGroups.size (); ++ i) {
        for (auto ctrl: controlGroups.at (i)) {
            ctrl->Show (groupToShow == i ? SW_SHOW : SW_HIDE);
        }
    }
}

void BunkeringWindow::showOnlySelectedTank (bool before) {
    CTabCtrlWrapper *switchCtrl = before ? tanksBefore : tanksAfter;
    int selection = switchCtrl->GetCurSel ();

    for (int i = 0; i < (int) cfg.tanks.size (); ++ i) {
        tankInfoBefore [i]->Show ((before && i == selection) ? SW_SHOW : SW_HIDE);
        tankInfoAfter [i]->Show ((!before && i == selection) ? SW_SHOW : SW_HIDE);
    }
}

void BunkeringWindow::enableButtons (bool enableAction, bool enableSave) {
    if (editBunker) editBunker->Enable (enableAction);
    if (removeBunker) removeBunker->Enable (enableAction);
    if (createReport) createReport->Enable (enableAction);
    if (save) save->Enable (enableSave);
    if (discard) discard->Enable (enableSave);
    if (tabSwitch) tabSwitch->Show ((enableAction || enableSave) ? SW_SHOW : SW_HIDE);
}

LRESULT BunkeringWindow::OnNotify (NMHDR *header) {
    if (header->code == NM_DBLCLK && (mode == _mode::add || mode == _mode::edit)) {
        NMITEMACTIVATE *info = (NMITEMACTIVATE *) header;
        if (header->idFrom >= ID_TANK_INFO_BEFORE_FIRST && header->idFrom < (ID_TANK_INFO_BEFORE_FIRST + cfg.tanks.size ())) {
            tankInfoBefore [header->idFrom-ID_TANK_INFO_BEFORE_FIRST]->editValue (info->iItem); return FALSE;
        } else if (header->idFrom >= ID_TANK_INFO_AFTER_FIRST && header->idFrom < (ID_TANK_INFO_AFTER_FIRST + cfg.tanks.size ())) {
            tankInfoAfter [header->idFrom-ID_TANK_INFO_AFTER_FIRST]->editValue (info->iItem); return FALSE;
        }
    }
    switch (header->idFrom) {
        case ID_BUNKER_LIST: {
            if (header->code == LVN_ITEMCHANGED) {
                NMLISTVIEW *data = (NMLISTVIEW *) header;

                if (data->uOldState != data->uNewState && (data->uNewState & LVIS_SELECTED)) {
                    int index = bunkerList->GetItemData (data->iItem);
                    setBunkeringData (list [index]);
                    enableButtons (true, false);
                    enableEditor (false);
                    mode = _mode::view;
                    editingItem = data->iItem;
                }
            } else if (header->code == LVN_ITEMCHANGING && (mode == _mode::add || mode == _mode::edit)) {
                NMLISTVIEW *data = (NMLISTVIEW *) header;

                if (data->uOldState != data->uNewState && (data->uNewState & LVIS_SELECTED) && data->iItem != editingItem) {
                    static int count = 0;
                    
                    if (((++count) & 1) || MessageBox ("Изменения будут потеряны. Продолжать?", "Внимание", MB_ICONQUESTION | MB_YESNO) != IDYES) return TRUE;
                }
            } else if (header->code == NM_DBLCLK) {
                NMITEMACTIVATE *info = (NMITEMACTIVATE *) header;
                if (MessageBox ("Редактировать бункеровку?", "Подтверждение", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    auto& data = list [info->iItem];
                    editingItem = info->iItem;
                    setBunkeringData (data);
                    enableButtons (false, true);
                    enableEditor (true);
                    addBunker->Enable (FALSE);
                    mode = _mode::edit;
                }
            }
            break;
        }
        case ID_TANKS_AFTER:
        case ID_TANKS_BEFORE: {
            if (header->code == TCN_SELCHANGE) {
                showOnlySelectedTank (header->idFrom == ID_TANKS_BEFORE);
            }
            break;
        }
        case ID_BUNK_HDR_FUEL_STATE: {
            if (header->code == NM_DBLCLK) {
                NMITEMACTIVATE *info = (NMITEMACTIVATE *) header;
                bunkerLoadInfo->editValue (info->iItem);
            }
            break;
        }
        case ID_BUNKERING_TABS: {
            auto selection = tabSwitch->GetCurSel ();

            showControlGroup (selection);
            if (selection > 0) showOnlySelectedTank (selection == 1);
            break;
        }
    }
    return FALSE;
}

void BunkeringWindow::setBunkeringData (bunkeringData& _data) {
    char buffer [100];

    // Loaded page
    bunkerLoadInfo->showState (_data.loaded);
    beginDate->SetTimestamp (_data.begin);
    beginTime->SetTimestamp (_data.begin);
    endDate->SetTimestamp (_data.end);
    endTime->SetTimestamp (_data.end);
    port->SetText (_data.port.c_str ());
    barge->SetText (_data.barge.c_str ());

    // Before page
    draftForeBefore->SetText (ftoa (_data.draftBefore.fore, buffer, "%.1f"));
    draftAftBefore->SetText (ftoa (_data.draftBefore.aft, buffer, "%.1f"));
    fmInBefore->SetText (ftoa (_data.pmBefore.in, buffer, "%.3f"));
    fmOutBefore->SetText (ftoa (_data.pmBefore.out, buffer, "%.3f"));

    for (auto i = 0; i < _data.tankStates.size (); ++ i) {
        tankInfoBefore [i]->showState (_data.tankStates [i].before);
    }

    // After page
    draftForeAfter->SetText (ftoa (_data.draftAfter.fore, buffer, "%.1f"));
    draftAftAfter->SetText (ftoa (_data.draftAfter.aft, buffer, "%.1f"));
    fmInAfter->SetText (ftoa (_data.pmAfter.in, buffer, "%.3f"));
    fmOutAfter->SetText (ftoa (_data.pmAfter.out, buffer, "%.3f"));

    for (auto i = 0; i < _data.tankStates.size (); ++ i) {
        tankInfoAfter [i]->showState (_data.tankStates [i].after);
    }
}

bool BunkeringWindow::checkData (bunkeringData& data) {
    static const char *ERROR_TITLE = "Ошибка";

    auto showError = [this] (const char *text) {
        MessageBox (text, ERROR_TITLE, MB_ICONSTOP);
    };

    if (port->GetTextLength () == 0) {
        showError ("Порт не указан");
        tabSwitch->SetCurSel (0);
        showControlGroup (0);
        port->SetFocus (); return false;
    }
    
    if (barge->GetTextLength () == 0) {
        showError ("Баржа-бункеровщик не указана");
        tabSwitch->SetCurSel (0);
        showControlGroup (0);
        barge->SetFocus (); return false;
    }

    if (!bunkerLoadInfo->readState (data.loaded)) {
        tabSwitch->SetCurSel (0);
        showError ("Информация о загруженном топливе неполная");
        return false;
    }

    auto getText = [] (CEditWrapper *edit, std::string& str) {
        char buffer [100];
        edit->GetText (buffer, sizeof (buffer));
        str = buffer;
    };
    auto getFloat = [] (CEditWrapper *edit) {
        char buffer [100];
        edit->GetText (buffer, sizeof (buffer));
        return (float) atof (buffer);
    };

    data.begin = composeDateAndTime (beginDate->GetTimestamp (), beginTime->GetTimestamp ());
    data.end = composeDateAndTime (endDate->GetTimestamp (), endTime->GetTimestamp ());
    
    getText (port, data.port);
    getText (barge, data.barge);

    if ((data.pmBefore.in = getFloat (fmInBefore)) < 0.01f) {
        tabSwitch->SetCurSel (1);
        showControlGroup (1);
        fmInBefore->SetFocus ();
        showError ("Отсутствует информация о показаниях входящего расходомера до закачки");
        return false;
    }

    if ((data.pmBefore.out = getFloat (fmOutBefore)) < 0.01f) {
        tabSwitch->SetCurSel (1);
        showControlGroup (1);
        fmOutBefore->SetFocus ();
        showError ("Отсутствует информация о показаниях исходящего расходомера до закачки");
        return false;
    }

    if ((data.pmAfter.in = getFloat (fmInAfter)) < 0.01f) {
        tabSwitch->SetCurSel (2);
        showControlGroup (2);
        fmInAfter->SetFocus ();
        showError ("Отсутствует информация о показаниях входящего расходомера после закачки");
        return false;
    }

    if ((data.pmAfter.out = getFloat (fmOutAfter)) < 0.01f) {
        tabSwitch->SetCurSel (2);
        showControlGroup (2);
        fmOutAfter->SetFocus ();
        showError ("Отсутствует информация о показаниях исходящего расходомера после закачки");
        return false;
    }

    if ((data.draftBefore.fore = getFloat (draftForeBefore)) < 0.01f) {
        tabSwitch->SetCurSel (1);
        showControlGroup (1);
        draftForeBefore->SetFocus ();
        showError ("Отсутствует информация об осадке в носу до закачки");
        return false;
    }
    if ((data.draftBefore.aft = getFloat (draftAftBefore)) < 0.01f) {
        tabSwitch->SetCurSel (1);
        showControlGroup (1);
        draftAftBefore->SetFocus ();
        showError ("Отсутствует информация об осадке в корме до закачки");
        return false;
    }
    if ((data.draftAfter.fore = getFloat (draftForeAfter)) < 0.01f) {
        tabSwitch->SetCurSel (2);
        showControlGroup (2);
        draftForeAfter->SetFocus ();
        showError ("Отсутствует информация об осадке в носу после закачки");
        return false;
    }
    if ((data.draftAfter.aft = getFloat (draftAftAfter)) < 0.01f) {
        tabSwitch->SetCurSel (2);
        showControlGroup (2);
        draftAftAfter->SetFocus ();
        showError ("Отсутствует информация об осадке в корме после закачки");
        return false;
    }

    for (auto i = 0; i < cfg.tanks.size (); ++ i) {
        if (!tankInfoBefore [i]->readState (data.tankStates [i].before)) {
            showError ((cfg.tanks [i].name + ": данные до загрузки неполные").c_str ());
            tabSwitch->SetCurSel (1);
            tanksBefore->SetCurSel (i);
            showControlGroup (1);
            showOnlySelectedTank (true);
            return false;
        }
        if (!tankInfoAfter [i]->readState (data.tankStates [i].after)) {
            showError ((cfg.tanks [i].name + ": данные после загрузки неполные").c_str ());
            tabSwitch->SetCurSel (2);
            tanksAfter->SetCurSel (i);
            showControlGroup (2);
            showOnlySelectedTank (false);
            return false;
        }
    }

    return true;
}

void BunkeringWindow::enableEditor (bool enable) {
    editMode = enable;

    for (auto& group: controlGroups) {
        for (auto& ctrl: group) {
            if (ctrl != tanksBefore && ctrl != tanksAfter) ctrl->Enable (enable);
        }
        for (auto& ctrl: tankInfoBefore) ctrl->Enable (enable);
        for (auto& ctrl: tankInfoAfter) ctrl->Enable (enable);
    }
}