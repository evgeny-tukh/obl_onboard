#include <stdlib.h>
#include <stdio.h>
#include <time.h>

char *formatTimestamp (time_t timestamp, char *buffer) {
    tm *dateTime = localtime (& timestamp);

    sprintf (buffer, "%02d.%02d.%04d %02d:%02d", dateTime->tm_mday, dateTime->tm_mon + 1, dateTime->tm_year + 1900, dateTime->tm_hour, dateTime->tm_min);

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