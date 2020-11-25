#include <stdlib.h>
#include <Windows.h>

#define EPOCH_BIAS  116444736000000000i64

void TimetToFileTime (const time_t timeTime, FILETIME *pftTime);
void TimetToLocalFileTime (const time_t timeTime, FILETIME *pftTime);
void TimetToSystemTime (const time_t timeTime, SYSTEMTIME *pstTime);
void TimetToLocalTime (const time_t timeTime, SYSTEMTIME *pstTime);
time_t FileTimeToTimet (FILETIME *pftTime);
time_t LocalFileTimeToTimet (FILETIME *pftTime);
time_t SystemTimeToTimet (SYSTEMTIME *pstTime);
time_t LocalTimeToTimet (SYSTEMTIME *pstTime);
time_t LocalTimeToTimet2 (SYSTEMTIME *pstTime);