#include "nmea.h"
#include <iostream>
#include <sstream>
#include "../common/tools.h"

namespace nmea {
    gpsPos pos;
    float cog, sog, hdg;
    time_t dateTime;
    char qualInd, psModeInd;
    tm lastDate { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    data _pos (& pos, sizeof (pos));
    data _hdg (& hdg, sizeof (hdg));
    data _cog (& cog, sizeof (cog));
    data _sog (& sog, sizeof (sog));
    data _dateTime (& dateTime, sizeof (dateTime));
    data _qualInd (& qualInd, sizeof (qualInd));
    data _psModeInd (& psModeInd, sizeof (psModeInd));

    void parse_GGA (sentence&);
    void parse_GLL (sentence&);
    void parse_ZDA (sentence&);
    void parse_VTG (sentence&);
    void parse_HDT (sentence&);
    void parse_RMC (sentence&);

    bool composeLat (sentence&, int, double&);
    bool composeLon (sentence&, int, double&);
    bool extratcUTC (sentence&, int, time_t&);
}

inline int digit2Int (char digit) {
    return (digit >= '0' && digit <= '9') ? digit - '0' : 0;
}

inline int twoDigits2Int (char *source) {
    return digit2Int (source [0]) * 10 + digit2Int (source [1]);
}

inline int threeDigits2Int (char *source) {
    return digit2Int (source [0]) * 100 + digit2Int (source [1]) * 10 + digit2Int (source [2]);
}

bool nmea::sentence::parse (char *source) {
    memset (type, 0, sizeof (type));
    fields.clear ();

    // general check
    if (*source != '$' && *source != '!') return false;

    auto asteriskPos = strchr (source, '*');

    if (!asteriskPos) return false;

    // look for crc
    #if 0
    auto hex2int = [] (char digit) {
        if (digit >= '0' && digit <= '9')
            return digit - '0';
        else if (digit >= 'a' && digit <= 'f')
            return digit - 'a' + 10;
        else if (digit >= 'A' && digit <= 'F')
            return digit - 'A' + 10;
        else
            return 0;
    };
    #endif
    
    uint8_t crc = source [1], presentCrc = hex2int (asteriskPos [1]) * 16 + hex2int (asteriskPos [2]);

    for (auto chr = source + 2; chr < asteriskPos; ++ chr) crc ^= *chr;

    if (crc != presentCrc) return false;

    // parse itself
    fields.emplace_back ("");

    for (auto chr = source; chr < asteriskPos; ++ chr) {
        if (*chr == ',')
            fields.push_back ("");
        else
            fields.back () += *chr;
    }

    if (fields.front ().length () > 3)
        memcpy (type, & fields.front ().back () - 2, 3);
    else
        memset (type, 0, sizeof (type));

    return true;
}

void nmea::parse (nmea::sentence& snt) {
    if (memcmp (snt.type, "GGA", 3) == 0)
        parse_GGA (snt);
    else if (memcmp (snt.type, "ZDA", 3) == 0)
        parse_ZDA (snt);
    else if (memcmp (snt.type, "VTG", 3) == 0)
        parse_VTG (snt);
    else if (memcmp (snt.type, "HDT", 3) == 0)
        parse_HDT (snt);
    else if (memcmp (snt.type, "GLL", 3) == 0)
        parse_GLL (snt);
    else if (memcmp (snt.type, "RMC", 3) == 0)
        parse_RMC (snt);
}

bool nmea::composeLat (sentence& snt, int start, double& value) {
    if (snt.fields.size () < (start + 2)) return false;
    if (snt.fields.at (start).empty () || snt.fields.at (start + 1).empty ()) return false;

    auto textValue = snt.fields.at (start).c_str ();
    
    value = (double) twoDigits2Int ((char *) textValue);
    value += atof (textValue + 2) / 60.0;
    
    if (snt.asCharAt (start + 1) == 'S') value = - value;

    return true;
}

bool nmea::composeLon (sentence& snt, int start, double& value) {
    if (snt.fields.size () < (start + 2)) return false;
    if (snt.fields.at (start).empty () || snt.fields.at (start + 1).empty ()) return false;

    auto textValue = snt.fields.at (start).c_str ();
    
    value = (double) threeDigits2Int ((char *) textValue);
    value += atof (textValue + 3) / 60.0;
    
    if (snt.asCharAt (start + 1) == 'W') value = - value;

    return true;
}

bool nmea::extratcUTC (sentence& snt, int field, time_t& timestamp) {
    if (snt.fields.size () <= field) return false;
    if (!lastDate.tm_year) return false;
    if (snt.fieldLength (field) < 6) return false;

    tm result;
    char *textValue = snt.asStringAt (field);

    result.tm_year = lastDate.tm_year;
    result.tm_mon = lastDate.tm_mon;
    result.tm_mday = lastDate.tm_mday;
    result.tm_hour = twoDigits2Int (textValue);
    result.tm_min = twoDigits2Int (textValue + 2);
    result.tm_sec = twoDigits2Int (textValue + 4);

    timestamp = mktime (& result) - *__timezone ();

    return true;
}

void nmea::parse_RMC (sentence& snt) {
    if (snt.fields.size () < 8 || snt.asCharAt (2) != 'A') return;

    if (!snt.omitted (9)) {
        char *dateString = snt.asStringAt (9);
        char *utc = snt.asStringAt (1);

        lastDate.tm_year = twoDigits2Int (dateString + 4) - 1900;
        lastDate.tm_mon = twoDigits2Int (dateString + 2) - 1;
        lastDate.tm_mday = twoDigits2Int (dateString);
        lastDate.tm_hour = twoDigits2Int (utc);
        lastDate.tm_min = twoDigits2Int (utc + 2);
        lastDate.tm_sec = twoDigits2Int (utc + 4);
    }

    gpsPos position;
    time_t timestamp, now = time (0);
    char indicator = snt.omitted (12) ? 'N' : snt.asCharAt (12);
    bool ok = composeLat (snt, 3, position.lat) && composeLon (snt, 5, position.lon) && extratcUTC (snt, 1, timestamp);

    switch (indicator) {
        case 'A':
        case 'D':
        case 'P':
            break;
        default:
            ok = false;
    }

    _psModeInd.update (now, true, & indicator);
    _pos.update (now, ok, & position);
    _dateTime.update (now, ok, & timestamp);

    if (!snt.omitted (7)) {
        float value = atof (snt.asStringAt (7));
        _sog.update (now, true, & value);
    }

    if (!snt.omitted (8)) {
        float value = atof (snt.asStringAt (8));
        _cog.update (now, true, & value);
    }
}

void nmea::parse_GLL (sentence& snt) {
    if (snt.fields.size () < 8 || snt.asCharAt (6) != 'A') return;

    gpsPos position;
    time_t timestamp, now = time (0);
    char indicator = snt.omitted (7) ? 'V' : snt.asCharAt (7);
    bool ok = composeLat (snt, 1, position.lat) && composeLon (snt, 3, position.lon) && extratcUTC (snt, 5, timestamp);

    switch (indicator) {
        case 'A':
        case 'D':
        case 'P':
            break;
        default:
            ok = false;
    }

    _psModeInd.update (now, true, & indicator);
    _pos.update (now, ok, & position);
    _dateTime.update (now, ok, & timestamp);
}

void nmea::parse_GGA (sentence& snt) {
    if (snt.fields.size () < 7) return;

    gpsPos position;
    time_t timestamp, now = time (0);
    char indicator = snt.omitted (6) ? 'V' : snt.asCharAt (6);
    bool ok = composeLat (snt, 2, position.lat) && composeLon (snt, 4, position.lon) && extratcUTC (snt, 1, timestamp);

    ok &= (indicator >= '1' && indicator <= '5');

    _psModeInd.update (now, true, & indicator);
    _pos.update (now, ok, & position);
    _dateTime.update (now, ok, & timestamp);
}

void nmea::parse_ZDA (sentence& snt) {
    if (snt.fields.size () < 5) return;
    if (snt.fieldLength (1) < 6) return;

    for (auto i = 2; i < 5; ++ i) {
        if (snt.fieldLength (i) < 2) return;
    }

    char *utc = snt.asStringAt (1);

    lastDate.tm_hour = twoDigits2Int (utc);
    lastDate.tm_min = twoDigits2Int (utc + 2);
    lastDate.tm_sec = twoDigits2Int (utc + 4);
    lastDate.tm_year = atoi (snt.asStringAt (4)) - 1900;
    lastDate.tm_mon = atoi (snt.asStringAt (3)) - 1;
    lastDate.tm_mday = atoi (snt.asStringAt (2));

    time_t timestamp = mktime (& lastDate) - *__timezone ();

    _dateTime.update (time (0), true, & timestamp);
}

void nmea::parse_HDT (sentence& snt) {
    if (!snt.omitted (1) && !snt.omitted (2) && snt.asCharAt (2) == 'T') {
        float heading = atof (snt.asStringAt (1));

        if (heading >= 0.0f && heading < 360.0f) _hdg.update (time (0), true, & heading);
    }
}

void nmea::parse_VTG (sentence& snt) {
    if (snt.omitted (9)) return;

    char modeInd = snt.asCharAt (9);
    bool ok = true;

    switch (modeInd) {
        case 'A':
        case 'D':
        case 'P':
            break;
        default:
            ok = false;
    }

    time_t now = time (0);
    _psModeInd.update (now, true, & modeInd);

    if (!ok) return;

    if (snt.fields.size () > 2 && !snt.omitted (1) && !snt.omitted (2) && snt.asCharAt (2) == 'T') {
        float courseOG = atof (snt.fields.at (1).c_str ());

        _cog.update (now, courseOG >= 0.0f && courseOG < 360.0f, & courseOG);
    }

    if (snt.fields.size () > 6 && !snt.omitted (5) && !snt.omitted (6) && snt.asCharAt (6) == 'N') {
        float speedOG = atof (snt.fields.at (5).c_str ());

        _sog.update (now, speedOG > 0.0f && speedOG < 100.0f, & speedOG);
    } else if (snt.fields.size () > 8 && !snt.omitted (7) && !snt.omitted (8) && snt.asCharAt (8) == 'K') {
        float speedOG = atof (snt.fields.at (7).c_str ()) / 1.852f;

        _sog.update (now, speedOG > 0.0f && speedOG < 100.0f, & speedOG);
    }
}

void nmea::checkAlive (time_t now) {
    _pos.checkAlive (now);
    _hdg.checkAlive (now);
    _cog.checkAlive (now);
    _sog.checkAlive (now);
    _dateTime.checkAlive (now);
    _qualInd.checkAlive (now);
    _psModeInd.checkAlive (now);
}

time_t nmea::getTimestamp () {
    return dateTime;
}

double *nmea::getLat () {
    return _pos.ok ? & pos.lat : 0;
}

double *nmea::getLon () {
    return _pos.ok ? & pos.lon : 0;
}

float *nmea::getSOG () {
    return _sog.ok ? & sog : 0;
}

float *nmea::getCOG () {
    return _cog.ok ? & cog : 0;
}

float *nmea::getHDG () {
    return _hdg.ok ? & hdg : 0;
}

int32_t nmea::getEncodedLat () {
    return _pos.ok ? (int32_t) (pos.lat * 60000.0) : NO_VALID_DATA;
}

int32_t nmea::getEncodedLon () {
    return _pos.ok ? (int32_t) (pos.lon * 60000.0) : NO_VALID_DATA;
}

uint32_t nmea::getEncodedSOG () {
    return _sog.ok ? (uint32_t) (sog * 10.0) : NO_VALID_DATA;
}

uint32_t nmea::getEncodedCOG () {
    return _cog.ok ? (uint32_t) (cog * 10.0) : NO_VALID_DATA;
}

uint32_t nmea::getEncodedHDG () {
    return _hdg.ok ? (uint32_t) (hdg * 10.0) : NO_VALID_DATA;
}

char *nmea::formatPos (double lat, double lon, char *buffer) {
    if (buffer) {
        double absLat = fabs (lat);
        double absLon = fabs (lon);
        int latDeg = (int) absLat;
        int lonDeg = (int) absLon;
        double latMin = (absLat - (double) latDeg) * 60.0;
        double lonMin = (absLon - (double) lonDeg) * 60.0;

        sprintf (
            buffer,
            "%02d %06.3f%c %03d %06.3f%c",
            latDeg,
            latMin,
            lat >= 0.0 ? 'N' : 'S',
            lonDeg,
            lonMin,
            lon >= 0.0 ? 'E' : 'W'
        );
    }

    return buffer;
}
