#include <stdlib.h>
#include <stdio.h>
#include "req_mgr.h"

reqManager::reqManager (uint16_t _port, char *_host): port (_port), host (_strdup (_host)), internet (0) {
}

reqManager::~reqManager () {
    if (host) free (host);

    if (internet) InternetCloseHandle (internet);
}

bool reqManager::open () {
    if (!internet) {
        internet = InternetOpen ("OblDeaemon", INTERNET_OPEN_TYPE_DIRECT, 0, 0, 0);

        if (!internet) {
            printf ("Error %d connecting to Internet\n", GetLastError ()); return false;
        }
    }
    
    return internet != 0;
}

bool reqManager::sendRequest (char *reqCmd, char *buffer, size_t size) {
    char url [500] { "/api" };

    sprintf (url, "http://%s:%d/api/%s", host, port, reqCmd);

    auto urlHandle = InternetOpenUrl (internet, url, 0, 0, 0, 0);

    if (!urlHandle) {
        printf ("Error %d opening URL\n", GetLastError ()); return false;
    }

    memset (buffer, 0, size);

    unsigned long bytesRead;
    auto result = InternetReadFile (urlHandle, buffer, size - 1, & bytesRead);

    buffer [bytesRead] = 0;

    InternetCloseHandle (urlHandle);

    return result;
}