#include <Windows.h>
#include "ship_schema.h"
#include "../resource.h"
#include "../../common/tools.h"

ShipSchema::ShipSchema (HINSTANCE instance, HWND parent, database& _db, config& _cfg, dataHistory *hist) :
    history (hist),
    objects (),
    cfg (_cfg),
    db (_db),
    selectedTank (-1),
    timeline (0),
    dateTime (0),
    historyMode (0), onlineMode (0), statusIndicator (0),
    isHistoryMode (false),
    CWindowWrapper (instance, parent, "obl_ship_schema") {
}

ShipSchema::~ShipSchema () {
    delete timeline;
    delete historyMode, onlineMode;
    delete statusIndicator;
}

void ShipSchema::setTimestamp (time_t ts)
{
    timestamp = ts;

    //InvalidateRect (m_hwndHandle, NULL, FALSE);
    redrawTanks ();
}

LRESULT ShipSchema::OnSize (const DWORD requestType, const WORD width, const WORD height) {
    if (timeline) timeline->Move (0, height - 25, width - DATE_TIME_WIDTH - 200, 25, TRUE);
    if (dateTime) dateTime->Move (width - DATE_TIME_WIDTH - 200, height - 25, DATE_TIME_WIDTH, 25, TRUE);
    if (historyMode) historyMode->Move (width - 200, height - 25, 100, 25, TRUE);
    if (onlineMode) onlineMode->Move (width - 100, height - 25, 100, 25, TRUE);
    if (statusIndicator) statusIndicator->Create (width - 35, 5, 30, 30);

    return FALSE;
}

void ShipSchema::OnCreate () {
    RECT parent, client;
    char dateTimeString [100];

    ::GetClientRect (m_hwndParent, & parent);
    GetClientRect (& client);
    ::MoveWindow (m_hwndHandle, 0, 0, 100, parent.bottom, TRUE);

    statusIndicator = new StatusIndicator (m_hInstance, m_hwndHandle, ID_STATUS_INDICATOR);

    statusIndicator->Create (client.right - 35, 5, 30, 30);

    timeline = new CTrackbarWrapper (m_hwndHandle, ID_TIME_SELECTOR);
    timestamp = history->maxTime ();

    timeline->CreateControl (0, client.bottom - 25, client.right - DATE_TIME_WIDTH - 200, 25, TBS_AUTOTICKS);
    timeline->SetRange (history->minTime (), timestamp);
    timeline->SetValue (timestamp);

    dateTime = new CStaticWrapper (m_hwndHandle, IDC_DATE_TIME);

    dateTime->CreateControl (client.right - DATE_TIME_WIDTH - 200, client.bottom - 25, DATE_TIME_WIDTH, 25, SS_CENTER);
    dateTime->SetText (formatTimestamp (timestamp, dateTimeString));

    historyMode = new CButtonWrapper (m_hwndHandle, ID_HISTORY_MODE);
    onlineMode = new CButtonWrapper (m_hwndHandle, ID_ONLINE_MODE);

    historyMode->CreateControl (client.right - 200, client.bottom - 25, 100, 25, BS_PUSHLIKE | BS_AUTORADIOBUTTON | WS_TABSTOP | WS_GROUP, "История");
    onlineMode->CreateControl (client.right - 100, client.bottom - 25, 100, 25, BS_PUSHLIKE | BS_AUTORADIOBUTTON | WS_TABSTOP, "Online");
    onlineMode->SendMessage (BM_SETCHECK, BST_CHECKED);
    
    updateTimer = SetTimer (1000, 60000);

    updateStatus ();
}

void ShipSchema::redrawTanks () {
    RECT client;
    tankDisplay::metrics tankMetrics;
    HDC paintCtx = GetDC (m_hwndHandle);

    GetClientRect (& client);
    recalc (tankMetrics);

    for (auto tank = cfg.tanks.begin (); tank != cfg.tanks.end (); ++ tank) {
        tankDisplay tankDisp (*tank);

        tankDisp.draw (paintCtx, m_hwndHandle, tankMetrics, objects, history->getData (tank->id, timestamp), tank->id, tank->type.c_str (), selectedTank == tank->id);
    }

    int fmCount = 0;
    int middle = client.right >> 1;

    for (auto fuelMeter = cfg.fuelMeters.begin (); fuelMeter != cfg.fuelMeters.end (); ++ fuelMeter, ++ fmCount) {
        int x;
        int placeFromCenter = fmCount >> 1;
        int offsetFromCenter = placeFromCenter * (fuelMeterDisplay::WIDTH + fuelMeterDisplay::GAP) + (fuelMeterDisplay::GAP >> 1);

        if (fmCount & 1) {
            // left side
            x = middle - offsetFromCenter - fuelMeterDisplay::WIDTH;
        } else {
            // right side
            x = middle + offsetFromCenter;
        }

        fuelMeterDisplay fmDisplay (*fuelMeter);

        fmDisplay.draw (paintCtx, m_hwndHandle, x, objects, history->getData (fuelMeter->id, timestamp), fuelMeter->name.c_str ());
    }

    ReleaseDC (m_hwndHandle, paintCtx);
}

LRESULT ShipSchema::OnPaint () {
    PAINTSTRUCT data;
    RECT client;
    HDC paintCtx = BeginPaint (m_hwndHandle, & data);

    GetClientRect (& client);
    FillRect (paintCtx, & client, (HBRUSH) GetStockObject (GRAY_BRUSH));

    SelectObject (paintCtx, (HBRUSH) GetStockObject (WHITE_BRUSH));
    SelectObject (paintCtx, (HPEN) GetStockObject (BLACK_PEN));

    POINT vesselShape [12];

    vesselShape [0].x = client.right >> 1;
    vesselShape [0].y = 10;
    vesselShape [1].x = client.right - 30;
    vesselShape [1].y = 40;
    vesselShape [2].x = client.right - 10;
    vesselShape [2].y = 60;
    vesselShape [3].x = client.right - 10;
    vesselShape [3].y = client.bottom - 80;
    vesselShape [4].x = client.right - 30;
    vesselShape [4].y = client.bottom - 60;
    vesselShape [5].x = (client.right >> 2) * 3;
    vesselShape [5].y = client.bottom - 46;

    vesselShape [6].x = client.right >> 1;
    vesselShape [6].y = client.bottom - 40;

    vesselShape [7].x = client.right >> 2;
    vesselShape [7].y = client.bottom - 46;

    vesselShape [8].x = 30;
    vesselShape [8].y = client.bottom - 60;

    vesselShape [9].x = 10;
    vesselShape [9].y = client.bottom - 80;
    vesselShape [10].x = 10;
    vesselShape [10].y = 60;
    vesselShape [11].x = 30;
    vesselShape [11].y = 40;

    Polygon (paintCtx, vesselShape, 12);

    tankDisplay::metrics tankMetrics;

    recalc (tankMetrics);

    for (auto tank = cfg.tanks.begin (); tank != cfg.tanks.end (); ++ tank) {
        tankDisplay tankDisp (*tank);

        tankDisp.draw (paintCtx, m_hwndHandle, tankMetrics, objects, history->getData (tank->id, timestamp), tank->id, tank->type.c_str (), selectedTank == tank->id);
    }

    int fmCount = 0;
    int middle = client.right >> 1;

    for (auto fuelMeter = cfg.fuelMeters.begin (); fuelMeter != cfg.fuelMeters.end (); ++ fuelMeter, ++ fmCount) {
        int x;
        int placeFromCenter = fmCount >> 1;
        int offsetFromCenter = placeFromCenter * (fuelMeterDisplay::WIDTH + fuelMeterDisplay::GAP) + (fuelMeterDisplay::GAP >> 1);

        if (fmCount & 1) {
            // left side
            x = middle - offsetFromCenter - fuelMeterDisplay::WIDTH;
        } else {
            // right side
            x = middle + offsetFromCenter;
        }

        fuelMeterDisplay fmDisplay (*fuelMeter);

        fmDisplay.draw (paintCtx, m_hwndHandle, x, objects, history->getData (fuelMeter->id, timestamp), fuelMeter->name.c_str ());
    }

    EndPaint (m_hwndHandle, & data);

    return 0;
}

void ShipSchema::recalc (tankDisplay::metrics& tankMetrics) {
    RECT client;

    GetClientRect (& client);

    int stbdCount = 0, portCount = 0, middleCount = 0;

    for (auto tank = cfg.tanks.begin (); tank != cfg.tanks.end (); ++ tank) {
        switch (tank->side) {
            case tankSide::starboard: stbdCount ++; break;
            case tankSide::port: portCount ++; break;
            case tankSide::center: middleCount ++; break;
        }
    }

    int maxCount = max (max (stbdCount, middleCount), portCount);

    tankMetrics.width = (client.right - tankDisplay::HOR_EDGE * 6) / 3;
    tankMetrics.height = (client.bottom - (maxCount - 1) * tankDisplay::HOR_EDGE - 2 * tankDisplay::VER_EDGE) / maxCount;
}

LRESULT ShipSchema::OnMessage (UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_HSCROLL: {
            if ((HWND) lParam == timeline->GetHandle ()) {
                time_t curTimestamp = timeline->GetPos ();
                char dateTimeString [100];

                dateTime->SetText (formatTimestamp (curTimestamp, dateTimeString));
                setTimestamp (curTimestamp);
            }

            break;
        }
    }

    return CWindowWrapper::OnMessage (message, wParam, lParam);
}

void ShipSchema::updateStatus () {
    time_t now = time (0);

    database::valueMap tankLevels, meterValues;

    db.collectCurrentVolumes (tankLevels);
    db.collectCurrentMeters (meterValues);

    time_t minDelay = 2000000000, maxDelay = 0, maxTimestamp = 0;

    auto checkValues = [&minDelay, &maxDelay, now, & maxTimestamp] (database::valueMap& values) {
        for (auto& value: values) {
            auto delay = now - value.second.timestamp;
            if (delay > maxDelay) maxDelay = delay;
            if (delay < minDelay) minDelay = delay;
            if (value.second.timestamp > maxTimestamp) maxTimestamp = value.second.timestamp;
        }
    };

    checkValues (tankLevels);
    checkValues (meterValues);

    timestamp = maxTimestamp;

    if (maxDelay < cfg.timeout)
        statusIndicator->setStatus (StatusIndicator::dataStatus::ok);
    else if (maxDelay < cfg.timeout)
        statusIndicator->setStatus (StatusIndicator::dataStatus::warning);
    else
        statusIndicator->setStatus (StatusIndicator::dataStatus::failure);

    char dateTimeString [100];

    dateTime->SetText (formatTimestamp (timestamp, dateTimeString));
    timeline->SetRange (history->minTime (), timestamp);
    timeline->SetValue (timestamp);
}

LRESULT ShipSchema::OnTimer (UINT timerID) {
    if (timerID == 1000) {
        history->load ();

        updateStatus ();

        //InvalidateRect (m_hwndHandle, 0, FALSE);
        redrawTanks ();
    }

    return 0;
}

LRESULT ShipSchema::OnCommand (WPARAM wParam, LPARAM lParam) {
    switch (WORD id = LOWORD (wParam)) {
        case ID_ONLINE_MODE:
        case ID_HISTORY_MODE: {
            isHistoryMode = id == ID_HISTORY_MODE;
            timeline->Enable (isHistoryMode);
            if (!isHistoryMode) {
                char dateTimeString [100];
                timestamp = history->maxTime ();
                timeline->SetValue (timestamp);
                dateTime->SetText (formatTimestamp (timestamp, dateTimeString));
                //InvalidateRect (m_hwndHandle, 0, FALSE);
                redrawTanks ();
            }
            break;
        }
    }

    return 1L;
}

void ShipSchema::onNewData () {
    if (statusIndicator && statusIndicator->getStatus () != StatusIndicator::dataStatus::ok) updateStatus ();
}
