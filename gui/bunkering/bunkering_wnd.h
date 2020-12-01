#pragma once

#include <vector>

#include "../../common/defs.h"
#include "../../common/db.h"
#include "../wui/WindowWrapper.h"
#include "../wui/GenericControlWrapper.h"
#include "../wui/ListCtrlWrapper.h"
#include "../wui/TabCtrlWrapper.h"
#include "../wui/ButtonWrapper.h"
#include "../wui/EditWrapper.h"
#include "../wui/TrackbarWrapper.h"
#include "../wui/DateTimePickerWrapper.h"
#include "../wui/ListBoxWrapper.h"
#include "bunkering_header.h"

class BunkeringWindow: public CWindowWrapper {
    public:
        BunkeringWindow (HINSTANCE instance, HWND parent, config& _cfg, database& _db);
        virtual ~BunkeringWindow ();

        void setBunkeringData (bunkeringData& _data);
        
    private:
        enum _mode {
            browseList, view, edit, add
        };

        _mode mode;

        static const int BUNK_LIST_HEIGHT = 320;
        static const int BUTTON_WIDTH = 200;
        static const int BUTTON_HEIGHT = 30;

        typedef std::vector<CGenericControlWrapper *> controlGroup;

        //BunkeringHeaderWindow *bunkerHeader;
        CTabCtrlWrapper *tabSwitch, *tanksBefore, *tanksAfter;
        FuelStateEditCtrl *bunkerLoadInfo;
        std::vector <FuelStateEditCtrl *> tankInfoBefore, tankInfoAfter;

        // Page controls
        std::vector<controlGroup> controlGroups;

        CGenericControlWrapper *addControlToGroup (int group, CGenericControlWrapper *control) {
            while (controlGroups.size () <= group) controlGroups.emplace_back ();

            controlGroups.at (group).push_back (control); return control;
        }

        void showControlGroup (int groupToShow = -1);

        CStaticWrapper *bunkeringLabel, *beginLabel, *endLabel, *portLabel, *bargeLabel, *beforeLabel, *afterLabel;
        CStaticWrapper *draftForeBeforeLabel, *draftForeAfterLabel;
        CStaticWrapper *draftAftBeforeLabel, *draftAftAfterLabel;
        CStaticWrapper *fmInBeforeLabel, *fmOutBeforeLabel, *fmInAfterLabel, *fmOutAfterLabel;
        CEditWrapper *fmInBefore, *fmOutBefore, *fmInAfter, *fmOutAfter;
        CListCtrlWrapper *bunkerList;
        CListBoxWrapper *tankList;
        CEditWrapper *port, *barge, *draftForeBefore, *draftForeAfter, *draftAftBefore, *draftAftAfter;
        CButtonWrapper *addBunker, *removeBunker, *editBunker, *save, *discard;
        CDateTimePickerWrapper *beginDate, *beginTime, *endDate, *endTime;
        config& cfg;
        database& db;
        bunkeringList list;
        bool editMode;

        virtual void OnCreate ();
        virtual LRESULT OnCommand (WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnSize (const DWORD requestType, const WORD width, const WORD height);
        virtual LRESULT OnNotify (NMHDR *pHeader);

        void loadBunkeringList ();

        void showOnlySelectedTank (bool before);
        bool checkData (bunkeringData&);
        void enableButtons (bool enableAction, bool enableSave);
        void enableEditor (bool enable);
};
