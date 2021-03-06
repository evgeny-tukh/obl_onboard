#include <stdlib.h>

#include <thread>
#include <chrono>

#include "../common/defs.h"
#include "req_mgr.h"
#include "../common/json.h"
#include "../common/db.h"
#include "../common/tools.h"
#include "logbook.h"

extern logbook::record logbookData;

char *getPath ();
SOCKET createTransmitter ();

class tankData {
    public:
        tankData (config& _cfg, time_t ts = 0): cfg (_cfg), timestamp (ts) {}
        inline void setTimestamp (time_t ts) { timestamp = ts; }
        void addSensorInput (const char *inputKey, const char *value, database& db, SOCKET transmitter);

    private:
        time_t timestamp;
        config& cfg;
};

int buildSendNmea (nmeaSource *nmeaSrc, const char *body, SOCKET transmitter) {
    // construct nmea sentence
    char sentence [100];
    uint8_t crc = nmeaSrc->source [0];

    for (auto chr = nmeaSrc->source + 1; *chr; ++ chr) crc ^= *chr;
    crc ^= ',';
    for (auto chr = body; *chr; ++ chr) crc ^= *chr;

    sprintf (sentence, "$%s,%s*%02X\r\n", nmeaSrc->source, body, crc);

    sockaddr_in transmitterDest;

    transmitterDest.sin_family = AF_INET;
    transmitterDest.sin_addr.S_un.S_addr = INADDR_BROADCAST;
    transmitterDest.sin_port = htons (nmeaSrc->port);

    int sent = sendto (transmitter, sentence, strlen (sentence), 0, (const sockaddr *) & transmitterDest, sizeof (transmitterDest));

    return sent;
}

void tankData::addSensorInput (const char *inputKey, const char *value, database& db, SOCKET transmitter) {
    char key [100];
    int column;

    // Nothing to do if value is "no data"
    if (*value == '*') return;

    // Does input key belong to the tank?
    tank *tankCfg = cfg.findTank ((char *) inputKey);
    nmeaSource *nmeaSrc = tankCfg ? 0 : cfg.findNmeaSource ((char *) inputKey);

    double numValue = nmeaSrc ? 0.0f : atof (value);

    if (tankCfg) {
        db.addData (tankCfg->id, timestamp, database::dataValueType::tankVolume, numValue);
    } else if (strcmp (inputKey, cfg.draftForeChannel.c_str ()) == 0) {
        // Draft channel
        logbookData.draftFore.update ((float) numValue);
    } else if (strcmp (inputKey, cfg.draftAftChannel.c_str ()) == 0) {
        logbookData.draftAft.update ((float) numValue);
    } else if (nmeaSrc) {
        buildSendNmea (nmeaSrc, value, transmitter);
    } else {
        // Does input key belong to the fuel meter?
        fuelMeter *fmCfg = cfg.findFuelMeter ((char *) inputKey);

        if (fmCfg) db.addData (fmCfg->id, timestamp, database::dataValueType::fuelMeter, numValue);
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

void parsePollResult (char *buffer, config& cfg, database& db, SOCKET transmitter) {
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
                        data.addSensorInput (fldName, fldValue, db, transmitter);
                    }
                }
            }
        }

        PostMessage (HWND_BROADCAST, cfg.newDataMsg, 0, 0);
    }
}

void poll (reqManager& reqMgr, config& cfg, database& db, SOCKET transmitter) {
    if (reqMgr.open ()) {
        static const size_t bufSize = 0xFFFF;
        char *buffer = (char *) malloc (bufSize);

        if (buffer) {
            printf ("Requesting...");

            if (reqMgr.sendRequest (buffer, bufSize)) {
                printf ("%zd bytes received\n", strlen (buffer));

                parsePollResult (buffer, cfg, db, transmitter);
            }

            free (buffer);
        } else {
            printf ("Unable to allocate buffer\n");
        }
    }
}

void pollerProc (pollerContext *ctx, config& cfg) {
    database db (cfg, getPath ());
    reqManager reqMgr (cfg.port, (char *) cfg.host.c_str (), (char *) cfg.path.c_str ());
    WSAData data;
    SOCKET transmitter;

    WSAStartup (0x202, & data);

    transmitter = createTransmitter ();

    ctx->lastCheck = time (0);
    poll (reqMgr, cfg, db, transmitter);

    while (ctx->keepRunning) {
        time_t now = time (0);

        if ((now - ctx->lastCheck) >= cfg.pollingInterval) {
            ctx->lastCheck = now;
            poll (reqMgr, cfg, db, transmitter);
        }

        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    closesocket (transmitter);
}

pollerContext *startPoller (config& cfg) {
    pollerContext *ctx = new pollerContext;

    ctx->keepRunning = true;
    ctx->runner = new std::thread (pollerProc, ctx, cfg);

    return ctx;
}