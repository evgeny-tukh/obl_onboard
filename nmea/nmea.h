#pragma once

#include <string>
#include <time.h>
#include <cstdint>
#include <map>
#include <vector>

namespace nmea {
    struct data {
        void *value;
        size_t size;
        time_t updateTime, timeout;
        bool ok;

        data (void *buffer, size_t sz, time_t to = 15): value (buffer), updateTime (0), size (sz), ok (false), timeout (to) {
            memset (value, 0, size);
        }
        inline void update (time_t timestamp, bool good = false, void *source = 0) {
            if (good) {
                memcpy (value, source, size);
                updateTime = timestamp;
                ok = true;
            } else if ((timestamp - updateTime) > timeout) {
                ok = false;
            }
        }
    };

    struct sentence {
        char type [4];
        std::vector<std::string> fields;

        sentence (): type {0, 0, 0, 0} {}
        bool parse (char *);
    };

    struct gpsPos {
        double lat, lon;
    };

    void parse (sentence&);
}