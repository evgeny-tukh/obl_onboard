#pragma once

#include <time.h>

char *formatTimestamp (time_t timestamp, char *buffer);

inline char *ftoa (double value, char *buffer, char *format) {
    sprintf (buffer, format, value);
    return buffer;
}

time_t composeDateAndTime (time_t dateTS, time_t timeTS);