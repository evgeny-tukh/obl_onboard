#pragma once

#include <stdlib.h>
#include <memory.h>
#include <time.h>

namespace logbook {
    struct data {
        time_t lastUpdate, timeout;
        bool good;
        void *buffer;
        size_t size;

        data (time_t _timeout): timeout (_timeout), lastUpdate (0), good (false), buffer (0), size (0) {}
        data (void *_buf, size_t _size, time_t _timeout): timeout (_timeout), lastUpdate (0), good (false), buffer (_buf), size (_size) {}

        inline void update (void *source) {
            if (source && buffer && size > 0) {
                memcpy (buffer, source, size);
                lastUpdate = time (0);
                good = true;
            }
        }

        inline bool get (void *dest) {
            if (good) memcpy (dest, buffer, size);
            return good;
        }
        inline void checkAlive (time_t now) {
            if (!now) now = time (0);
            if ((now - lastUpdate) > timeout) good = false;
        }
    };

    struct pos {
        double lat, lon;

        pos (): lat (0.0), lon (0.0) {}
        pos (double _lat, double _lon): lat (_lat), lon (_lon) {}
    };

    struct posData: data {
        posData (): data ((void *) new pos, sizeof (pos), 15) {}

        inline void update (double lat, double lon) {
            pos position (lat, lon);
            data::update (& position);
        }
        inline void update (double *lat, double *lon) {
            if (lat && lon) {
                pos position (*lat, *lon);
                data::update (& position);
            }
        }
    };

    struct simpleData: data {
        simpleData (time_t timeout): data ((void *) new float, sizeof (float), timeout) {}

        inline void update (float value) {
            data::update (& value);
        }
        inline void update (float *value) {
            data::update (value);
        }
    };
    
    struct record {
        posData position;
        simpleData sog, cog, hdg, draftAft, draftFore;

        record (): position (), sog (15), cog (15), hdg (15), draftAft (100), draftFore (100) {}

        inline double *getLat () {
            return position.good ? & ((pos *) position.buffer)->lat : 0;
        }
        inline double *getLon () {
            return position.good ? & ((pos *) position.buffer)->lon : 0;
        }
        inline float *getSOG () {
            return sog.good ? (float *) sog.buffer : 0;
        }
        inline float *getCOG () {
            return cog.good ? (float *) cog.buffer : 0;
        }
        inline float *getHDG () {
            return hdg.good ? (float *) hdg.buffer : 0;
        }
        inline float *getDraftFore () {
            return draftFore.good ? (float *) draftFore.buffer : 0;
        }
        inline float *getDraftAft () {
            return draftAft.good ? (float *) draftAft.buffer : 0;
        }

        inline void checkAlive (time_t now) {
            if (!now) now = time (0);
            position.checkAlive (now);
            sog.checkAlive (now);
            cog.checkAlive (now);
            hdg.checkAlive (now);
            draftFore.checkAlive (now);
            draftAft.checkAlive (now);
        }
    };
}
