#include "nmea.h"
#include <iostream>
#include <sstream>

namespace nmea {
    gpsPos pos;
    float cog, sog;
    time_t dateTime;
    tm lastDate { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    data _pos (& pos, sizeof (pos));
    data _cog (& cog, sizeof (cog));
    data _sog (& sog, sizeof (sog));
    data _dateTime (& dateTime, sizeof (dateTime));

    void parse_GGA (sentence&);
    void parse_ZDA (sentence&);

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
}

bool nmea::composeLat (sentence& snt, int start, double& value) {
    if (snt.fields.size () < (start + 2)) return false;
    if (snt.fields.at (start).empty () || snt.fields.at (start + 1).empty ()) return false;

    auto textValue = snt.fields.at (start).c_str ();
    
    value = (double) twoDigits2Int ((char *) textValue);
    value += atof (textValue + 2) / 60.0;
    
    if (snt.fields.at (start + 1).at (0) == 'S') value = - value;

    return true;
}

bool nmea::composeLon (sentence& snt, int start, double& value) {
    if (snt.fields.size () < (start + 2)) return false;
    if (snt.fields.at (start).empty () || snt.fields.at (start + 1).empty ()) return false;

    auto textValue = snt.fields.at (start).c_str ();
    
    value = (double) threeDigits2Int ((char *) textValue);
    value += atof (textValue + 3) / 60.0;
    
    if (snt.fields.at (start + 1).at (0) == 'W') value = - value;

    return true;
}

bool nmea::extratcUTC (sentence& snt, int field, time_t& timestamp) {
    if (snt.fields.size () <= field) return false;
    if (!lastDate.tm_year) return false;
    if (snt.fields.at (field).length () < 6) return false;

    tm result;
    char *textValue = & snt.fields.at (field).front ();

    result.tm_year = lastDate.tm_year;
    result.tm_mon = lastDate.tm_mon;
    result.tm_mday = lastDate.tm_mday;
    result.tm_hour = twoDigits2Int (textValue);
    result.tm_min = twoDigits2Int (textValue + 2);
    result.tm_sec = twoDigits2Int (textValue + 4);

    timestamp = mktime (& result) - *__timezone ();
}

void nmea::parse_GGA (sentence& snt) {
    gpsPos position;
    time_t timestamp, now = time (0);
    bool ok = composeLat (snt, 2, position.lat) && composeLon (snt, 4, position.lon) && extratcUTC (snt, 1, timestamp);

    _pos.update (now, ok, & position);
    _dateTime.update (now, ok, & timestamp);
}

void nmea::parse_ZDA (sentence& snt) {
    if (snt.fields.size () < 5) return;
    if (snt.fields.at (1).length () < 6) return;

    for (auto i = 2; i < 5; ++ i) {
        if (snt.fields.at (i).length () < 2) return;
    }

    char *utc = & snt.fields.at (1).front ();

    lastDate.tm_hour = twoDigits2Int (utc);
    lastDate.tm_min = twoDigits2Int (utc + 2);
    lastDate.tm_sec = twoDigits2Int (utc + 4);
    lastDate.tm_year = atoi (snt.fields.at (4).c_str ()) - 1900;
    lastDate.tm_mon = atoi (snt.fields.at (3).c_str ()) - 1;
    lastDate.tm_mday = atoi (snt.fields.at (2).c_str ());

    time_t timestamp = mktime (& lastDate) - *__timezone ();

    _dateTime.update (time (0), true, & timestamp);
}
