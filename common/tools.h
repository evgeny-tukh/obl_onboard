#pragma once

#include <time.h>
#include <Windows.h>

#include "defs.h"
#include "json.h"

enum timiestampFormatFlags {
    showDate = 1,
    showTime = 2,
    showSeconds = 4,
};

char *formatTimestamp (time_t timestamp, char *buffer);
char *formatTimestampEx (time_t timestamp, char *buffer, uint8_t flags);

inline char *ftoa (double value, char *buffer, char *format) {
    sprintf (buffer, format, value);
    return buffer;
}

time_t composeDateAndTime (time_t dateTS, time_t timeTS);

void paintRectangleGradient (HDC paintCtx, int x1, int y1, int x2, int y2, uint32_t beginColor, uint32_t endColor, bool horizontal);
void paintEllipseGradient (HDC paintCtx, int x1, int y1, int x2, int y2, uint32_t beginColor, uint32_t endColor, bool horizontal);

void replaceSlashes (char *path);

typedef void (*walkCb) (char *path, void *param, WIN32_FIND_DATAA *findData);
void walkThroughFolder (char *path, walkCb cb, void *param);

bool exportJson (json::hashNode& root, config& cfg);

inline auto hex2int (char digit) {
    if (digit >= '0' && digit <= '9')
        return digit - '0';
    else if (digit >= 'a' && digit <= 'f')
        return digit - 'a' + 10;
    else if (digit >= 'A' && digit <= 'F')
        return digit - 'A' + 10;
    else
        return 0;
}



