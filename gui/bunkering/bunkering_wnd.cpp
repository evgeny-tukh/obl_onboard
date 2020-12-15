#include <stdlib.h>
#include <Windows.h>
#include "bunkering_wnd.h"
#include "bunkering_edit.h"
#include "../resource.h"
#include "../../common/tools.h"
#include "fuel_state_edit_ctrl.h"
#include "hw_data_edit_ctrl.h"
#include "../../repgen/excel.h"
#include "../../common/json.h"

WNDPROC BunkeringWindow::defTabSwitchProc = 0;

BunkeringWindow::BunkeringWindow (HINSTANCE instance, HWND parent, config& _cfg, database& _db):
    CWindowWrapper (instance, parent, "obl_bnk_wnd"), cfg (_cfg), db (_db),
    mode (_mode::browseList), editingItem (-1), bunkerHwDataInfo (0),
    bunkerList (0), addBunker (0), removeBunker (0), editBunker (0), bunkerLoadInfo (0), bunkeringLabel (0),
    beforeLabel (0), afterLabel (0), tanksBefore (0), tanksAfter (0),
    save (0), discard (0), tabSwitch (0), editMode (false), createReport (0), exportReport (0), loadData (0), calcWeight (0),
    draftForeBeforeLabel (0), draftForeAfterLabel (0), draftAftBeforeLabel (0), draftAftAfterLabel (0),
    draftForeBefore (0), draftForeAfter (0), draftAftBefore (0), draftAftAfter (0) {}

BunkeringWindow::~BunkeringWindow () {
    delete bunkerList;
    delete addBunker, removeBunker, editBunker;
    delete tabSwitch;
    delete bunkerHwDataInfo;
    delete tanksBefore, tanksAfter;
    delete save, discard, createReport, exportReport, loadData, calcWeight;

    for (auto& ctrl: tankInfoBefore) delete ctrl;
    for (auto& ctrl: tankInfoAfter) delete ctrl;
    for (auto& ctrl: hwDataBefore) delete ctrl;
    for (auto& ctrl: hwDataAfter) delete ctrl;
}

void BunkeringWindow::OnCreate () {
    RECT client;

    GetClientRect (& client);

    bunkerList = new CListCtrlWrapper (m_hwndHandle, ID_BUNKER_LIST);

    int bunkerListWidth = min (client.right - BUTTON_WIDTH, 595);
    int buttonWidth = client.right - bunkerListWidth;

    bunkerList->CreateControl (0, 0, bunkerListWidth, BUNK_LIST_HEIGHT, LVS_REPORT | LVS_SHOWSELALWAYS | WS_VISIBLE | WS_BORDER, 0);
    bunkerList->AddColumn ("Начало закачки", 110);
    bunkerList->AddColumn ("Конец закачки", 110);
    bunkerList->AddColumn ("Закачано по ОБР, т", 120);
    bunkerList->AddColumn ("Закачано по УМ, т", 115);
    bunkerList->AddColumn ("Закачано по РМ, т", 115);
    bunkerList->SendMessage (LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    addBunker = new CButtonWrapper (m_hwndHandle, ID_NEW_BUNKERING);
    removeBunker = new CButtonWrapper (m_hwndHandle, ID_DELETE_BUNKERING);
    createReport = new CButtonWrapper (m_hwndHandle, ID_CREATE_REPORT);
    exportReport = new CButtonWrapper (m_hwndHandle, ID_EXPORT_REPORT);
    editBunker = new CButtonWrapper (m_hwndHandle, ID_EDIT_BUNKERING);
    save = new CButtonWrapper (m_hwndHandle, IDOK);
    discard = new CButtonWrapper (m_hwndHandle, IDCANCEL);
    
    addBunker->CreateControl (client.right - buttonWidth, 0, buttonWidth, BUTTON_HEIGHT, WS_VISIBLE, "Новая бункеровка");
    removeBunker->CreateControl (client.right - buttonWidth, BUTTON_HEIGHT, buttonWidth, BUTTON_HEIGHT, WS_VISIBLE, "Удалить бункеровку");
    createReport->CreateControl (client.right - buttonWidth, BUTTON_HEIGHT * 2, buttonWidth, BUTTON_HEIGHT, WS_VISIBLE, "Сгенерировать расписку");
    exportReport->CreateControl (client.right - buttonWidth, BUTTON_HEIGHT * 3, buttonWidth, BUTTON_HEIGHT, WS_VISIBLE, "Экспортировать расписку");
    editBunker->CreateControl (client.right - buttonWidth, BUTTON_HEIGHT * 4, buttonWidth, BUTTON_HEIGHT, WS_VISIBLE, "Изменить бункеровку");
    save->CreateControl (client.right - buttonWidth, BUTTON_HEIGHT * 5, buttonWidth, BUTTON_HEIGHT, WS_VISIBLE, "Сохранить");
    discard->CreateControl (client.right - buttonWidth, BUTTON_HEIGHT * 6, buttonWidth, BUTTON_HEIGHT, WS_VISIBLE, "Отмена правки");
    enableButtons (false, false);

    tabSwitch = new CTabCtrlWrapper (m_hwndHandle, ID_BUNKERING_TABS);

    tabSwitch->CreateControl (0, BUNK_LIST_HEIGHT, bunkerListWidth, client.bottom - BUNK_LIST_HEIGHT, TCS_BUTTONS);
    tabSwitch->Show (SW_HIDE);
    tabSwitch->AddItem ("Общая информация", 0);
    tabSwitch->AddItem ("До закачки", 1);
    tabSwitch->AddItem ("После закачки", 2);

    auto switchHandle = tabSwitch->GetHandle ();

    ((BaseListCtrl *) addControlToGroup (0, bunkerLoadInfo = new FuelStateEditCtrl (switchHandle, ID_BUNK_HDR_FUEL_STATE)))->CreateControl (0, 30, 262, 150);
    ((BaseListCtrl *) addControlToGroup (0, bunkerHwDataInfo = new HwDataEditCtrl (switchHandle, ID_BUNK_HDR_HW_DATA)))->CreateControl (261, 30, 162, 150);
    addControlToGroup (0, beginLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 185, 55, 20, SS_LEFT, "Начало");
    addControlToGroup (0, endLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 205, 55, 20, SS_LEFT, "Конец");
    addControlToGroup (0, portLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 225, 55, 20, SS_LEFT, "Порт");
    addControlToGroup (0, bargeLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 245, 55, 20, SS_LEFT, "Баржа");
    addControlToGroup (0, port = new CEditWrapper (switchHandle, ID_PORT))->CreateControl (60, 225, 180, 20, WS_BORDER);
    addControlToGroup (0, barge = new CEditWrapper (switchHandle, ID_BARGE))->CreateControl (60, 245, 180, 20, WS_BORDER);
    addControlToGroup (0, beginDate = new CDateTimePickerWrapper (switchHandle, ID_BEGIN_DATE))->CreateControl (60, 185, 100, 20, DTS_SHORTDATECENTURYFORMAT | DTS_UPDOWN);
    addControlToGroup (0, beginTime = new CDateTimePickerWrapper (switchHandle, ID_BEGIN_TIME))->CreateControl (160, 185, 60, 20, DTS_TIMEFORMAT | DTS_UPDOWN);
    addControlToGroup (0, endDate = new CDateTimePickerWrapper (switchHandle, ID_END_DATE))->CreateControl (60, 205, 100, 20, DTS_SHORTDATECENTURYFORMAT | DTS_UPDOWN);
    addControlToGroup (0, endTime = new CDateTimePickerWrapper (switchHandle, ID_END_TIME))->CreateControl (160, 205, 60, 20, DTS_TIMEFORMAT | DTS_UPDOWN);
    addControlToGroup (0, loadData = new CButtonWrapper (switchHandle, ID_LOAD_DATA))->CreateControl (250, 185, 220, 25, WS_VISIBLE, "Автозагрузка данных");
    addControlToGroup (0, calcWeight = new CButtonWrapper (switchHandle, ID_CALC_WEIGHT))->CreateControl (250, 210, 220, 25, WS_VISIBLE, "Расчет масс топлива");

    addControlToGroup (1, draftForeBeforeLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 30, 120, 20, SS_LEFT, "Осадка в носу");
    addControlToGroup (1, draftAftBeforeLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 50, 120, 20, SS_LEFT, "Осадка в корме");
    addControlToGroup (1, draftForeBefore = new CEditWrapper (switchHandle, ID_DRAFT_FORE_BEFORE))->CreateControl (125, 30, 80, 20, WS_BORDER);
    addControlToGroup (1, draftAftBefore = new CEditWrapper (switchHandle, ID_DRAFT_AFT_BEFORE))->CreateControl (125, 50, 80, 20, WS_BORDER);

    addControlToGroup (2, draftForeAfterLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 30, 120, 20, SS_LEFT, "Осадка в носу");
    addControlToGroup (2, draftAftAfterLabel = new CStaticWrapper (switchHandle, IDC_STATIC))->CreateControl (5, 50, 120, 20, SS_LEFT, "Осадка в корме");
    addControlToGroup (2, draftForeAfter = new CEditWrapper (switchHandle, ID_DRAFT_FORE_AFTER))->CreateControl (125, 30, 80, 20, WS_BORDER);
    addControlToGroup (2, draftAftAfter = new CEditWrapper (switchHandle, ID_DRAFT_AFT_AFTER))->CreateControl (125, 50, 80, 20, WS_BORDER);

    beginTime->SetFormat ("HH:mm");
    endTime->SetFormat ("HH:mm");

    tanksBefore = new CTabCtrlWrapper (switchHandle, ID_TANKS_BEFORE);
    tanksAfter = new CTabCtrlWrapper (switchHandle, ID_TANKS_AFTER);

    addControlToGroup (1, tanksBefore->CreateControl (0, 80, bunkerListWidth, client.bottom - 80, TCS_BUTTONS));
    addControlToGroup (2, tanksAfter->CreateControl (0, 80, bunkerListWidth, client.bottom - 80, TCS_BUTTONS));

    auto count = 0;
    FuelStateEditCtrl *tankInfoCtrl;
    HwDataEditCtrl *hwDataCtrl;

    for (auto& tankInfo: cfg.tanks) {
        tanksBefore->AddItem ((char *) tankInfo.name.c_str (), tankInfo.id);
        tanksAfter->AddItem ((char *) tankInfo.name.c_str (), tankInfo.id);

        tankInfoCtrl = new FuelStateEditCtrl (tanksBefore->GetHandle (), ID_TANK_INFO_BEFORE_FIRST + count);
        hwDataCtrl = new HwDataEditCtrl (tanksBefore->GetHandle (), ID_HW_DATA_EDIT_BEFORE_FIRST + count);
        
        tankInfoCtrl->CreateControl (10, 30, 262, 150);
        tankInfoBefore.push_back (tankInfoCtrl);

        hwDataCtrl->CreateControl (271, 30, 162, 150);
        hwDataBefore.push_back (hwDataCtrl);

        tankInfoCtrl = new FuelStateEditCtrl (tanksAfter->GetHandle (), ID_TANK_INFO_AFTER_FIRST + count);
        hwDataCtrl = new HwDataEditCtrl (tanksAfter->GetHandle (), ID_HW_DATA_EDIT_AFTER_FIRST + count);
        
        tankInfoCtrl->CreateControl (10, 30, 262, 150);
        tankInfoAfter.push_back (tankInfoCtrl);

        hwDataCtrl->CreateControl (271, 30, 162, 150);
        hwDataAfter.push_back (hwDataCtrl);

        ++ count;
    }

    showControlGroup (0);
    loadBunkeringList ();

    defTabSwitchProc = (WNDPROC) GetWindowLongPtr (tabSwitch->GetHandle (), GWLP_WNDPROC);
    SetWindowLongPtr (tabSwitch->GetHandle (), GWLP_WNDPROC, (LONG_PTR) localTabSwitchProc);
}

LRESULT BunkeringWindow::OnSize (const DWORD requestType, const WORD width, const WORD height)
{
    int bunkerListWidth = min (width - BUTTON_WIDTH, 595);
    int buttonWidth = width - bunkerListWidth;

    bunkerList->Move (0, 0, bunkerListWidth, BUNK_LIST_HEIGHT, TRUE);
    addBunker->Move (width - buttonWidth, 0, buttonWidth, BUTTON_HEIGHT, TRUE);
    removeBunker->Move (width - buttonWidth, BUTTON_HEIGHT, buttonWidth, BUTTON_HEIGHT, TRUE);
    createReport->Move (width - buttonWidth, BUTTON_HEIGHT * 2, buttonWidth, BUTTON_HEIGHT, TRUE);
    exportReport->Move (width - buttonWidth, BUTTON_HEIGHT * 3, buttonWidth, BUTTON_HEIGHT, TRUE);
    editBunker->Move (width - buttonWidth, BUTTON_HEIGHT * 4, buttonWidth, BUTTON_HEIGHT, TRUE);
    save->Move (width - buttonWidth, BUTTON_HEIGHT * 5, buttonWidth, BUTTON_HEIGHT, TRUE);
    discard->Move (width - buttonWidth, BUTTON_HEIGHT * 6, buttonWidth, BUTTON_HEIGHT, TRUE);

    return FALSE;
}

void BunkeringWindow::exportReportData (bunkeringData& data) {
    json::hashNode root, dataNode, fuelMeters, fuelMetersBefore, fuelMetersAfter, loaded, draft, draftBefore, draftAfter;
    json::arrayNode tanks;

    auto addAmounts = [] (amounts& amnts, json::hashNode *jsonNode) {
        jsonNode->add ("reported", new json::numberNode (amnts.reported));
        jsonNode->add ("byVolume", new json::numberNode (amnts.byVolume));
        jsonNode->add ("byCounter", new json::numberNode (amnts.byCounter));
    };

    auto addFuelState = [addAmounts] (fuelState& fs, json::hashNode& jsonNode) {
        json::hashNode *volume = new json::hashNode;
        json::hashNode *quantity = new json::hashNode;

        addAmounts (fs.volume, volume);
        addAmounts (fs.quantity, quantity);

        jsonNode.add ("density", new json::numberNode (fs.density));
        jsonNode.add ("viscosity", new json::numberNode (fs.viscosity));
        jsonNode.add ("sulphur", new json::numberNode (fs.sulphur));
        jsonNode.add ("temp", new json::numberNode (fs.temp));
        jsonNode.add ("fuelMeter", new json::numberNode (fs.fuelMeter));
        jsonNode.add ("vcf", new json::numberNode (fs.vcf));
    };

    fuelMetersBefore.add ("in", new json::numberNode (data.pmBefore.in));
    fuelMetersBefore.add ("out", new json::numberNode (data.pmBefore.out));
    fuelMetersAfter.add ("in", new json::numberNode (data.pmAfter.in));
    fuelMetersAfter.add ("out", new json::numberNode (data.pmAfter.out));

    fuelMeters.add ("before", & fuelMetersBefore);
    fuelMeters.add ("after", & fuelMetersAfter);

    addFuelState (data.loaded, loaded);

    draftBefore.add ("fore", new json::numberNode (data.draftBefore.fore));
    draftBefore.add ("aft", new json::numberNode (data.draftBefore.aft));
    draftAfter.add ("fore", new json::numberNode (data.draftAfter.fore));
    draftAfter.add ("aft", new json::numberNode (data.draftAfter.aft));

    draft.add ("before", & draftBefore);
    draft.add ("after", & draftAfter);

    for (auto& tankState: data.tankStates) {
        json::hashNode *tank = new json::hashNode;
        json::hashNode *before = new json::hashNode;
        json::hashNode *after = new json::hashNode;

        addFuelState (tankState.before, *before);
        addFuelState (tankState.after, *after);

        tank->add ("tank", new json::numberNode (tankState.tank));
        tank->add ("before", before);
        tank->add ("after", after);

        tanks.add (tank);
    }

    dataNode.add ("id", new json::numberNode ((double) data.id));
    dataNode.add ("begin", new json::numberNode (data.begin));
    dataNode.add ("end", new json::numberNode (data.end));
    dataNode.add ("port", new json::stringNode (data.port.c_str ()));
    dataNode.add ("barge", new json::stringNode (data.barge.c_str ()));
    dataNode.add ("fuelMeters", & fuelMeters);
    dataNode.add ("draft", & draft);
    dataNode.add ("tanks", & tanks);

    root.add ("type", new json::stringNode ("bunkering"));
    root.add ("data", & dataNode);

    exportJson (root, cfg);
}

void BunkeringWindow::calcFuelWeight () {
    auto calcFuelWeightInTank = [] (FuelStateEditCtrl *reportedDataCtl, HwDataEditCtrl *actualDataCtl) {
        fuelState reportedState, actualState;

        reportedDataCtl->readState (reportedState);
        actualDataCtl->readState (actualState);

        float vcf = (1.0f - (reportedState.temp - 15.0f) * 0.00064f);
        float density = reportedState.density * vcf;

        actualState.quantity.byVolume = actualState.volume.byVolume * density;
        actualState.quantity.byCounter = actualState.volume.byCounter * density;
        actualState.vcf = vcf;

        reportedState.vcf = vcf;

        reportedDataCtl->showState (reportedState);
        actualDataCtl->showState (actualState);
    };

    if (MessageBox (
        "Расчет приведет к потере уже введенных весов загруженного топлива и топлива в танках, а также расчитанного ранее или введенного коэффициэнта "
        "температурного расширения топлива VCF. Расчет требует предварительного ввода корректных значений температуры топлива (загруженного и в танках), "
        "а также плотности загруженного топлива и топлива в танках при 15 градусах по Цельсию. Желаете продолжить?",
        "Требуется подтверждение",
        MB_YESNO | MB_ICONQUESTION
    ) == IDYES) {
        calcFuelWeightInTank (bunkerLoadInfo, bunkerHwDataInfo);

        for (auto i = 0; i < cfg.tanks.size (); ++ i) {
            calcFuelWeightInTank (tankInfoBefore [i], hwDataBefore [i]);
            calcFuelWeightInTank (tankInfoAfter [i], hwDataAfter [i]);
        }
    }
}

void BunkeringWindow::loadAndPopulateData () {
    MessageBox (
        "Будет произведен расчет объема загруженного топлива, а также объемов топлива в танках на моменты начала и конца бункеровки, "
        "исходя из указанного времени начала и конца. После ввода значений температуры и плотности загруженного топлива и топлива в танках можно "
        "также рассчитать массу загруженного топлива и топива в танках (а также коэффициэнт температурного расширения топлива VCF) "
        "на моменты начала и конца бункеровки с помощью соответствующей кнопки.",
        "Внимание!",
        MB_ICONINFORMATION
    );

    auto beginDateTs = beginDate->GetTimestamp ();
    auto beginTimeTs = beginTime->GetTimestamp ();
    auto endDateTs = endDate->GetTimestamp ();
    auto endTimeTs = endTime->GetTimestamp ();

    auto begin = composeDateAndTime (beginDateTs, beginTimeTs);
    auto end = composeDateAndTime (endDateTs, endTimeTs);

    std::vector<tankState> states;

    for (auto& tank: cfg.tanks) {
        states.emplace_back (tank.id);
    }

    float uploadingCounterBefore = 0.0f, uploadingCounterAfter = 0.0f;
    float volumeTotalBefore = 0.0f, volumeTotalAfter = 0.0f;

    if (!db.loadTankStatesAt (begin, end, states, uploadingCounterBefore, uploadingCounterAfter)) {
        MessageBox ("Некоторые параметры бункеровки на указанные моменты времени не найдены. Автозаполнение невозможно.", "Ошибка", MB_ICONSTOP); return;
    }

    for (auto& state: states) {
        volumeTotalBefore += state.before.volume.byVolume;
        volumeTotalAfter += state.after.volume.byVolume;
    }

    fuelState loaded;

    loaded.volume.byVolume = volumeTotalAfter - volumeTotalBefore;
    loaded.volume.byCounter = uploadingCounterAfter - uploadingCounterBefore;

    bunkerHwDataInfo->showState (loaded);

    for (size_t i = 0; i < states.size (); ++ i) {
        auto& state = states [i];

        tankInfoBefore [i]->showState (state.before);
        tankInfoAfter [i]->showState (state.after);
        hwDataBefore [i]->showState (state.before);
        hwDataAfter [i]->showState (state.after);
    }
}

LRESULT BunkeringWindow::OnCommand (WPARAM wParam, LPARAM lParam) {
    LRESULT result = TRUE;

    switch (LOWORD (wParam)) {
        case ID_CALC_WEIGHT: {
            calcFuelWeight (); break;
        }
        case ID_LOAD_DATA: {
            loadAndPopulateData (); break;
        }
        case ID_CREATE_REPORT: {
            int index = bunkerList->GetSelectedItem ();
            if (index >= 0) {
                generateReport (cfg, list [index], m_hInstance, m_hwndHandle);
            }
            break;
        }
        case ID_EXPORT_REPORT: {
            int index = bunkerList->GetSelectedItem ();
            if (index >= 0) {
                exportReportData (list [index]);
                MessageBox ("Результаты букировки успешно экспортированы", "Информация", MB_ICONINFORMATION);
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
            preLoadData (data);
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

void BunkeringWindow::preLoadData (bunkeringData& data) {
    for (auto& meter: cfg.fuelMeters) {
        if (meter.type.compare ("UPL") == 0) {
            data.pmBefore.in = db.getLastMeterValue (meter.id);
            data.pmAfter.in = data.pmBefore.in;
        } else if (meter.type.compare ("CONS") == 0) {
            data.pmBefore.out = db.getLastMeterValue (meter.id);
            data.pmAfter.out = data.pmBefore.in;
        }
    }
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
        bunkerList->SetItemText (item, 2, ftoa (bunkering->loaded.quantity.reported, buffer, "%.3f"));
        bunkerList->SetItemText (item, 3, ftoa (bunkering->loaded.quantity.byVolume, buffer, "%.3f"));
        bunkerList->SetItemText (item, 4, ftoa (bunkering->loaded.quantity.byCounter, buffer, "%.3f"));
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
        hwDataBefore [i]->Show ((before && i == selection) ? SW_SHOW : SW_HIDE);
        hwDataAfter [i]->Show ((!before && i == selection) ? SW_SHOW : SW_HIDE);
    }
}

void BunkeringWindow::enableButtons (bool enableAction, bool enableSave) {
    if (editBunker) editBunker->Enable (enableAction);
    if (removeBunker) removeBunker->Enable (enableAction);
    if (createReport) createReport->Enable (enableAction);
    if (exportReport) exportReport->Enable (enableAction);
    if (save) save->Enable (enableSave);
    if (discard) discard->Enable (enableSave);
    if (tabSwitch) tabSwitch->Show ((enableAction || enableSave) ? SW_SHOW : SW_HIDE);
}

bool BunkeringWindow::isTankInfoBefore (uint32_t id) {
    return id >= ID_TANK_INFO_BEFORE_FIRST && id < (ID_TANK_INFO_BEFORE_FIRST + cfg.tanks.size ());
}

bool BunkeringWindow::isTankInfoAfter (uint32_t id) {
    return id >= ID_TANK_INFO_AFTER_FIRST && id < (ID_TANK_INFO_AFTER_FIRST + cfg.tanks.size ());
}

bool BunkeringWindow::isHwDataBefore (uint32_t id) {
    return id >= ID_HW_DATA_EDIT_BEFORE_FIRST && id < (ID_HW_DATA_EDIT_BEFORE_FIRST + cfg.tanks.size ());
}

bool BunkeringWindow::isHwDataAfter (uint32_t id) {
    return id >= ID_HW_DATA_EDIT_AFTER_FIRST && id < (ID_HW_DATA_EDIT_AFTER_FIRST + cfg.tanks.size ());
}

LRESULT BunkeringWindow::OnNotify (NMHDR *header) {
    if (header->code == NM_DBLCLK && (mode == _mode::add || mode == _mode::edit)) {
        NMITEMACTIVATE *info = (NMITEMACTIVATE *) header;
        if (isTankInfoBefore (header->idFrom)) {
            auto ctrl = tankInfoBefore [header->idFrom-ID_TANK_INFO_BEFORE_FIRST];
            ctrl->editValue (info->iItem, ctrl); return FALSE;
        } else if (isTankInfoAfter (header->idFrom)) {
            auto ctrl = tankInfoAfter [header->idFrom-ID_TANK_INFO_AFTER_FIRST];
            ctrl->editValue (info->iItem, ctrl); return FALSE;
        } else if (isHwDataBefore (header->idFrom)) {
            auto ctrl = hwDataBefore [header->idFrom-ID_HW_DATA_EDIT_BEFORE_FIRST];
            auto pairedCtrl = tankInfoBefore  [header->idFrom-ID_HW_DATA_EDIT_BEFORE_FIRST];
            ctrl->editValue (info->iItem, pairedCtrl); return FALSE;
        } else if (isHwDataAfter (header->idFrom)) {
            auto ctrl = hwDataAfter [header->idFrom-ID_HW_DATA_EDIT_AFTER_FIRST];
            auto pairedCtrl = tankInfoAfter [header->idFrom-ID_HW_DATA_EDIT_AFTER_FIRST];
            ctrl->editValue (info->iItem, pairedCtrl); return FALSE;
        }
    }
    if (header->code == NM_CLICK) {
        NMITEMACTIVATE *info = (NMITEMACTIVATE *) header;
        BaseListCtrl *pairedList = 0;
        if (isTankInfoBefore (header->idFrom)) {
            pairedList = hwDataBefore [header->idFrom-ID_TANK_INFO_BEFORE_FIRST];
        } else if (isTankInfoAfter (header->idFrom)) {
            pairedList = hwDataAfter [header->idFrom-ID_TANK_INFO_AFTER_FIRST];
        } else if (isHwDataBefore (header->idFrom)) {
            pairedList = tankInfoBefore [header->idFrom-ID_HW_DATA_EDIT_BEFORE_FIRST];
        } else if (isHwDataAfter (header->idFrom)) {
            pairedList = tankInfoAfter [header->idFrom-ID_HW_DATA_EDIT_AFTER_FIRST];
        }
        if (pairedList && pairedList->GetSelectedItem () != info->iItem) {
            pairedList->SetItemState (info->iItem, LVIS_SELECTED, LVIS_SELECTED); return FALSE;
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
                bunkerLoadInfo->editValue (info->iItem, bunkerLoadInfo);
            }
            break;
        }
        case ID_BUNK_HDR_HW_DATA: {
            if (header->code == NM_DBLCLK) {
                NMITEMACTIVATE *info = (NMITEMACTIVATE *) header;
                bunkerHwDataInfo->editValue (info->iItem, bunkerLoadInfo);
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
    bunkerHwDataInfo->showState (_data.loaded);
    beginDate->SetTimestamp (_data.begin);
    beginTime->SetTimestamp (_data.begin);
    endDate->SetTimestamp (_data.end);
    endTime->SetTimestamp (_data.end);
    port->SetText (_data.port.c_str ());
    barge->SetText (_data.barge.c_str ());

    // Before page
    draftForeBefore->SetText (ftoa (_data.draftBefore.fore, buffer, "%.1f"));
    draftAftBefore->SetText (ftoa (_data.draftBefore.aft, buffer, "%.1f"));

    for (auto i = 0; i < _data.tankStates.size () && i < cfg.tanks.size (); ++ i) {
        tankInfoBefore [i]->showState (_data.tankStates [i].before);
        hwDataBefore [i]->showState (_data.tankStates [i].before);
    }

    // After page
    draftForeAfter->SetText (ftoa (_data.draftAfter.fore, buffer, "%.1f"));
    draftAftAfter->SetText (ftoa (_data.draftAfter.aft, buffer, "%.1f"));

    for (auto i = 0; i < _data.tankStates.size () && i < cfg.tanks.size (); ++ i) {
        tankInfoAfter [i]->showState (_data.tankStates [i].after);
        hwDataAfter [i]->showState (_data.tankStates [i].after);
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

    if (!bunkerLoadInfo->readState (data.loaded) || !bunkerHwDataInfo->readState (data.loaded)) {
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
        for (auto& ctrl: hwDataBefore) ctrl->Enable (enable);
        for (auto& ctrl: hwDataAfter) ctrl->Enable (enable);
    }
}

LRESULT CALLBACK BunkeringWindow::localTabSwitchProc (HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_COMMAND && GetDlgCtrlID (wnd) == ID_BUNKERING_TABS && HIWORD (wParam) == BN_CLICKED) {
        switch (LOWORD (wParam)) {
            case ID_LOAD_DATA:
            case ID_CALC_WEIGHT:
                ::SendMessage (GetParent (wnd), msg, wParam, lParam); break;
        }
    }
    
    return CallWindowProc (defTabSwitchProc, wnd, msg, wParam, lParam);
}
