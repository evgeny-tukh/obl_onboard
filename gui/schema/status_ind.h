#pragma once

#include "../wui/WindowWrapper.h"

class StatusIndicator: public CWindowWrapper {
    public:
        StatusIndicator (HINSTANCE instance, HWND parent, UINT_PTR id);
        virtual ~StatusIndicator ();

        BOOL Create (const int nX, const int nY, const int nWidth, const int nHeight) {
            return CWindowWrapper::Create (0, nX, nY, nWidth, nHeight, WS_CHILD | WS_VISIBLE);
        }

        enum dataStatus {
            ok = 1,
            warning,
            failure,
        };

        inline dataStatus getStatus () {
            return status;
        }

        inline void setStatus (dataStatus _status) {
            status = _status;

            InvalidateRect (m_hwndHandle, 0, TRUE);
        }

        virtual LRESULT OnPaint ();

    protected:
        HBRUSH red, green, yellow;
        dataStatus status;
};