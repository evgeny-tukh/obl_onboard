#pragma once

#include <Windows.h>
#include <Shlwapi.h>
#include <commctrl.h>
#include "wui/WindowWrapper.h"
#include "wui/ListCtrlWrapper.h"
#include "wui/TreeCtrlWrapper.h"
#include "wui/ListBoxWrapper.h"
#include "wui/StaticWrapper.h"
#include "wui/InputBox.h"

class CMainWnd : public CWindowWrapper
{
    public:
        CMainWnd (HINSTANCE instance);
        virtual ~CMainWnd ();

    private:
        virtual LRESULT OnMessage (UINT message, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnCommand (WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnSysCommand (WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnSize (const DWORD requestType, const WORD width, const WORD height);
        virtual LRESULT OnTimer (unsigned int timerID);
        virtual LRESULT OnNotify (NMHDR *pHeader);

        void RequestAppQuit ();

        void Initialize ();
};
