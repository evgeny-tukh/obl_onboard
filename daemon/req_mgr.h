#pragma once

#include <stdlib.h>
#include <cstdint>
#include <Windows.h>
#include <WinInet.h>

class reqManager {
    public:
        reqManager (uint16_t port, char *host);
        virtual ~reqManager ();

        bool open ();
        bool sendRequest (char *reqCmd, char *buffer, size_t size);

    protected:
        HINTERNET internet;
        uint16_t port;
        char *host;
};

