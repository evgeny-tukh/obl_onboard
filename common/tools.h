#pragma once

#include <time.h>
#include <Windows.h>

char *formatTimestamp (time_t timestamp, char *buffer);

inline char *ftoa (double value, char *buffer, char *format) {
    sprintf (buffer, format, value);
    return buffer;
}

time_t composeDateAndTime (time_t dateTS, time_t timeTS);

void paintRectangleGradient (HDC paintCtx, int x1, int y1, int x2, int y2, uint32_t beginColor, uint32_t endColor, bool horizontal);
void paintEllipseGradient (HDC paintCtx, int x1, int y1, int x2, int y2, uint32_t beginColor, uint32_t endColor, bool horizontal);
