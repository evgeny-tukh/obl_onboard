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
        inline void checkAlive (time_t now) {
            if ((now - updateTime) > timeout) ok = false;
        }
    };

    struct sentence {
        char type [4];
        std::vector<std::string> fields;

        sentence (): type {0, 0, 0, 0} {}
        bool parse (char *);
        inline bool omitted (int index) {
            return fields.size () <= index || fields.at (index).length () == 0;
        }
        inline size_t fieldLength (int index) {
            return fields.size () <= index ? 0 : fields.at (index).length ();
        }
        inline char asCharAt (int index) {
            return fields.size () <= index || fields.at (index).empty () ? 0 : fields.at (index).front ();
        }
        inline char *asStringAt (int index) {
            return fields.size  () <= index ? 0 : & fields.at (index).front ();
        }
    };

    struct gpsPos {
        double lat, lon;
    };

    void parse (sentence&);
    void checkAlive (time_t now);

    time_t getTimestamp ();
    double *getLat ();
    double *getLon ();
    float *getSOG ();
    float *getCOG ();
    float *getHDG ();

    static const uint32_t NO_VALID_DATA = 0x7FFFFFFFF;

    int32_t getEncodedLat ();
    int32_t getEncodedLon ();
    uint32_t getEncodedSOG ();
    uint32_t getEncodedCOG ();
    uint32_t getEncodedHDG ();

    char *formatPos (double lat, double lon, char *buffer);
}
