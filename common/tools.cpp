#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <cstdint>
#include <Windows.h>

#include "tools.h"

char *formatTimestamp (time_t timestamp, char *buffer) {
    tm *dateTime = localtime (& timestamp);

    sprintf (buffer, "%02d.%02d.%04d %02d:%02d", dateTime->tm_mday, dateTime->tm_mon + 1, dateTime->tm_year + 1900, dateTime->tm_hour, dateTime->tm_min);

    return buffer;
}

char *formatTimestampEx (time_t timestamp, char *buffer, uint8_t flags) {
    tm *dateTime = localtime (& timestamp);

    *buffer = '\0';

    if (flags & timiestampFormatFlags::showDate) {
        char date [50];
        sprintf (date, "%02d.%02d.%04d", dateTime->tm_mday, dateTime->tm_mon + 1, dateTime->tm_year + 1900);
        strcat (buffer, date);
    }

    if (flags & timiestampFormatFlags::showTime) {
        char time [50];

        if (flags & timiestampFormatFlags::showSeconds)
            sprintf (time, "%02d:%02d:%02d", dateTime->tm_hour, dateTime->tm_min, dateTime->tm_sec);
        else
            sprintf (time, "%02d:%02d", dateTime->tm_hour, dateTime->tm_min);

        if (*buffer) strcat (buffer, " ");

        strcat (buffer, time);
    }

    return buffer;
}

time_t composeDateAndTime (time_t dateTS, time_t timeTS) {
    tm *date = localtime (& dateTS);
    tm *time = localtime (& timeTS);

    time->tm_year = date->tm_year;
    time->tm_mon = date->tm_mon;
    time->tm_mday = date->tm_mday;

    return mktime (time);
}

void paintRectangleGradient (HDC paintCtx, int x1, int y1, int x2, int y2, uint32_t beginColor, uint32_t endColor, bool horizontal) {
    TRIVERTEX vertices [2];

    vertices [0].x = x1;
    vertices [0].y = y1;
    vertices [0].Red = GetRValue (beginColor) << 8;
    vertices [0].Green = GetGValue (beginColor) << 8;
    vertices [0].Blue = GetBValue (beginColor) << 8;
    vertices [0].Alpha = 0;
    vertices [1].x = x2;
    vertices [1].y = y2;
    vertices [1].Red = GetRValue (endColor) << 8;
    vertices [1].Green = GetGValue (endColor) << 8;
    vertices [1].Blue = GetBValue (endColor) << 8;
    vertices [1].Alpha = 0;

    GRADIENT_RECT rect { 0, 1 };

    GradientFill (paintCtx, vertices, 2, & rect, 1, horizontal ? GRADIENT_FILL_RECT_H : GRADIENT_FILL_RECT_V);
}

void paintEllipseGradient (HDC paintCtx, int x1, int y1, int x2, int y2, uint32_t beginColor, uint32_t endColor, bool horizontal) {
    HRGN clipper = CreateEllipticRgn (x1, y1, x2, y2);

    SelectClipRgn (paintCtx, clipper);
    paintRectangleGradient (paintCtx, x1, y1, x2, y2, beginColor, endColor, horizontal);
    SelectClipRgn (paintCtx, 0);
    DeleteObject (clipper);
}
