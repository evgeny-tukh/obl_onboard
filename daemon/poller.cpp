#include <stdlib.h>

#include <thread>
#include <chrono>

#include "defs.h"
#include "req_mgr.h"
#include "json.h"
#include "db.h"

class tankData {
    public:
        tankData (config& _cfg, time_t ts = 0): cfg (_cfg), timestamp (ts) {}
        inline void setTimestamp (time_t ts) { timestamp = ts; }
        void addSensorInput (const char *inputKey, const char *value);
        void saveData (database& db);

    private:
        // first is column, second is input value
        typedef std::map<uint8_t, double> sensorInputs;

        // first is tank id, second is a map of input volumes
        typedef std::map<uint16_t, sensorInputs> tankVolumes;

        time_t timestamp;
        tankVolumes volumes;
        config& cfg;
};

void tankData::addSensorInput (const char *inputKey, const char *value) {
    char key [100];
    int column;

    // Nothing to do if value is "no data"
    if (*value == '*') return;

    // Field name key contains parameter identifier in the last part
    // We are interesting into ".vol" (volume) and ".hop" (volume measured by Hoppe sensor)
    // Both of them should be recorded as "volumeAfter" but into different columns (sensorData and hoppeData, respectively)
    strcpy (key, inputKey);

    char *lastDot = strrchr (key, '.');

    // Nothing to do if we can't find the last field
    if (!*lastDot) return;

    // Map the last field to the column
    column = cfg.findColumnMap (lastDot);

    // Nothing to do if this key is nott mapped by the config
    if (column < 0) return;

    // Remove last part from tank key
    *lastDot = 0;

    // Find tank configuration by the key except the last part
    tank *curTank = cfg.findTank (key);

    // Nothing no do if tank is now known
    if (!curTank) return;

    // Find tank volumes map for this key
    auto item = volumes.find (curTank->id);

    if (item == volumes.end ()) {
        // No tank info yet, add new one
        item = volumes.emplace (curTank->id, sensorInputs ()).first;
    }

    // Add new value to tank volumes map
    item->second.emplace (column, atof (value));
}

void tankData::saveData (database& db) {
    param *volParam = cfg.findParam ("volumeAfter");

    for (auto tank = volumes.begin (); tank != volumes.end (); ++ tank) {
        uint64_t operation = db.addFuelOperation (tank->first, fuelOperType::operate, timestamp);

        for (auto input = tank->second.begin (); input != tank->second.end (); ++ input) {
            db.addFuelParameter (operation, volParam->id, input->first, input->second);
        }
    }
}

time_t makeTimestamp (const char *value, float timezone) {
    tm dateTime;

    memset (& dateTime, 0, sizeof (dateTime));

    dateTime.tm_year = atoi (value) - 1900;
    dateTime.tm_mon = atoi (value + 5) - 1;
    dateTime.tm_mday = atoi (value + 8);
    dateTime.tm_hour = atoi (value + 11);
    dateTime.tm_min = atoi (value + 14);
    dateTime.tm_sec = atoi (value + 17);

    return mktime (& dateTime) /*- (time_t) (timezone * 3600.0f)*/;
}

void parsePollResult (char *buffer, config& cfg, database& db) {
    int next = 0;
    auto *json = (json::hashNode *) json::parse (buffer, next);
    json::arrayNode *result = (json::arrayNode *) (*json) ["result"];

    if (result) {
        tankData data (cfg);
        time_t timestamp;

        for (auto i = 0; i < result->size (); ++ i) {
            json::hashNode *field = (json::hashNode *) (*result) [i];

            if (field) {
                json::stringNode *fieldName = (json::stringNode *) (*field) ["dbname"];
                json::stringNode *fieldValue = (json::stringNode *) (*field) ["value"];

                if (fieldName && fieldValue) {
                    const char *fldName = fieldName->getValue ();
                    const char *fldValue = fieldValue->getValue ();

                    if (strcmp (fldName, "Poll.TimeStamp") == 0) {
                        printf ("Data at %s LT\n", fldValue);
                        
                        timestamp = makeTimestamp (fldValue, cfg.timezone);

                        data.setTimestamp (timestamp);
                    } else {
                        data.addSensorInput (fldName, fldValue);
                        #if 0
                        char key [100];
                        int column;

                        // Field name key contains parameter identifier in the last part
                        // We are interesting into ".vol" (volume) and ".hop" (volume measured by Hoppe sensor)
                        // Both of them should be recorded as "volumeAfter" but into different columns (sensorData and hoppeData, respectively)
                        strcpy (key, fldName);

                        char *lastDot = strrchr (key, '.');

                        if (!*lastDot) continue;

                        column = cfg.findColumnMap (lastDot);

                        if (column < 0) continue;

                        // Remove last part from tank key
                        *lastDot = 0;

                        tank *curTank = cfg.findTank (key);

                        if (curTank) {
                            uint64_t operation = db.addFuelOperation (curTank->id, fuelOperType::operate, timestamp);

                            if (*fldValue != '*') db.addFuelParameter (operation, volParam->id, column, atof (fldValue));
                        }
                        #endif
                    }
                }
            }
        }

        // Store operations
        data.saveData (db);
    }
}

void poll (reqManager& reqMgr, config& cfg, database& db) {
    if (reqMgr.open ()) {
        static const size_t bufSize = 0xFFFF;
        char *buffer = (char *) malloc (bufSize);

        if (buffer) {
            printf ("Requesting...");

            if (reqMgr.sendRequest (buffer, bufSize)) {
                printf ("%zd bytes received\n", strlen (buffer));

                parsePollResult (buffer, cfg, db);
            }

            free (buffer);
        } else {
            printf ("Unable to allocate buffer\n");
        }
    }
}

void pollerProc (pollerContext *ctx, config& cfg) {
    database db;
    reqManager reqMgr (cfg.port, (char *) cfg.host.c_str (), (char *) cfg.path.c_str ());

    while (ctx->keepRunning) {
        time_t now = time (0);

        if ((now - ctx->lastCheck) >= cfg.pollingInterval) {
            ctx->lastCheck = now;
            poll (reqMgr, cfg, db);
        }

        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

pollerContext *startPoller (config& cfg) {
    pollerContext *ctx = new pollerContext;

    ctx->keepRunning = true;
    ctx->runner = new std::thread (pollerProc, ctx, cfg);

    return ctx;
}