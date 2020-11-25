#pragma once

#include <Windows.h>
#include "wui/WindowWrapper.h"
#include "wui/ButtonWrapper.h"
#include "wui/StaticWrapper.h"

class BunkeringEditor: public CWindowWrapper {
    public:
        BunkeringEditor (HINSTANCE instance, HWND parent);
        virtual ~BunkeringEditor ();

    private:
        CButtonWrapper *ok, *cancel;
        CStaticWrapper *densityLabel;

        virtual LRESULT OnMessage (UINT message, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnCommand (WPARAM wParam, LPARAM lParam);
        virtual void OnCreate ();
};
