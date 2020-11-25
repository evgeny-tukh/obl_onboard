#include <stdlib.h>
#include <stdio.h>
#include <time.h>

char *formatTimestamp (time_t timestamp, char *buffer) {
    tm *dateTime = localtime (& timestamp);

    sprintf (buffer, "%02d.%02d.%04d %02d:%02d", dateTime->tm_mday, dateTime->tm_mon + 1, dateTime->tm_year + 1900, dateTime->tm_hour, dateTime->tm_min);

    return buffer;
}