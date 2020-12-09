#include "nmea.h"
#include <iostream>
#include <sstream>

namespace nmea {
    gpsPos pos;
    float cog, sog;
    time_t dateTime;

    data _pos (& pos, sizeof (pos));
    data _cog (& cog, sizeof (cog));
    data _sog (& sog, sizeof (sog));
    data _dateTime (& dateTime, sizeof (dateTime));

    void parse_GGA (sentence&);

    bool composeLat (sentence&, int, double&);
    bool composeLon (sentence&, int, double&);
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
}

void nmea::parse_GGA (sentence& snt) {
    gpsPos position;
    bool ok = composeLat (snt, 2, position.lat) && composeLon (snt, 4, position.lon);

    _pos.update (time (0), ok, & position);
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
