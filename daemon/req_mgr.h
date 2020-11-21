#pragma once

#include <stdlib.h>
#include <cstdint>
#include <Windows.h>
#include <WinInet.h>

class reqManager {
    public:
        reqManager (uint16_t port, char *host, char *path);
        virtual ~reqManager ();

        bool open ();
        bool sendRequest (char *buffer, size_t size);

    protected:
        HINTERNET internet;
        uint16_t port;
        char *host, *path;
};

