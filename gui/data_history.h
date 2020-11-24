#pragma once

#include <time.h>
#include <map>

#include "../common/db.h"
#include "../common/defs.h"

class dataHistory {
    public:
        dataHistory (database& _db, config& _cfg);

        void load ();

        float getData (uint16_t id, time_t timestamp);
        time_t minTime ();
        time_t maxTime ();
        
    protected:
        typedef std::map<time_t, float> history;

        std::map<uint16_t, history> histories;
        database& db;
        config& cfg;

        static int loadCb (void *instance, int numOfFields, char **fields, char **values);
        int loadCb (int numOfFields, char **fields, char **values);
};