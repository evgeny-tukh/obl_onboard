#include <stdlib.h>
#include <Windows.h>
#include <time.h>

#define EPOCH_BIAS  116444736000000000i64

void TimetToFileTime (const time_t timeTime, FILETIME *pftTime)
{
    LONGLONG llTime;
    
    _tzset ();
    
    llTime = Int32x32To64 (timeTime - _timezone, 10000000) + EPOCH_BIAS;
    
    pftTime->dwLowDateTime  = (DWORD) llTime;
    pftTime->dwHighDateTime = (DWORD) (llTime >> 32);
}

void TimetToLocalFileTime (const time_t timeTime, FILETIME *pftTime)
{
    LONGLONG llTime;
    
    llTime = Int32x32To64 (timeTime, 10000000) + EPOCH_BIAS;
    
    pftTime->dwLowDateTime  = (DWORD) llTime;
    pftTime->dwHighDateTime = (DWORD) (llTime >> 32);
}

void TimetToSystemTime (const time_t timeTime, SYSTEMTIME *pstTime)
{
    FILETIME ftTime;
    
    TimetToFileTime (timeTime, & ftTime);
    
    if (pstTime)
        FileTimeToSystemTime (& ftTime, pstTime);
}

void TimetToLocalTime (const time_t timeTime, SYSTEMTIME *pstTime)
{
    FILETIME ftTime;
    
    TimetToLocalFileTime (timeTime, & ftTime);
    
    if (pstTime)
        FileTimeToSystemTime (& ftTime, pstTime);
}

time_t FileTimeToTimet (FILETIME *pftTime)
{
    LARGE_INTEGER liTime;
    
    liTime.LowPart  = pftTime->dwLowDateTime;
    liTime.HighPart = pftTime->dwHighDateTime;

    _tzset ();
        
    return _timezone + (time_t) ((liTime.QuadPart - EPOCH_BIAS) / 10000000);
}

time_t LocalFileTimeToTimet (FILETIME *pftTime)
{
    LARGE_INTEGER liTime;
    
    liTime.LowPart  = pftTime->dwLowDateTime;
    liTime.HighPart = pftTime->dwHighDateTime;

    return (time_t) ((liTime.QuadPart - EPOCH_BIAS) / 10000000);
}

time_t SystemTimeToTimet (SYSTEMTIME *pstTime)
{
    FILETIME ftTime;
    
    if (SystemTimeToFileTime (pstTime, & ftTime))
        return FileTimeToTimet (& ftTime);
    else
        return 0;
}

time_t LocalTimeToTimet (SYSTEMTIME *pstTime)
{
    FILETIME ftTime;
    
    if (SystemTimeToFileTime (pstTime, & ftTime))
        return FileTimeToTimet (& ftTime);
    else
        return 0;
}

time_t LocalTimeToTimet2 (SYSTEMTIME *pstTime)
{
    FILETIME ftTime;
    
    if (SystemTimeToFileTime (pstTime, & ftTime))
        return LocalFileTimeToTimet (& ftTime);
    else
        return 0;
}
