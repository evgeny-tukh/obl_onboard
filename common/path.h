#pragma once

#include <Windows.h>
#include <Shlwapi.h>

char *getPath () {
    static char path [MAX_PATH] { "" };

    if (!*path) {
        GetModuleFileName (0, path, sizeof (path));
        PathRemoveFileSpec (path);
        PathAddBackslash (path);
    }

    return path;
}

