#include <stdlib.h>

#include <thread>
#include <chrono>

#include "defs.h"
#include "req_mgr.h"
#include "json.h"

time_t makeTimestamp (const char *value) {
    tm dateTime;

    memset (& dateTime, 0, sizeof (dateTime));

    dateTime.tm_year = atoi (value) - 1900;
    dateTime.tm_mon = atoi (value + 5) - 1;
    dateTime.tm_mday = atoi (value + 8);
    dateTime.tm_hour = atoi (value + 11);
    dateTime.tm_min = atoi (value + 14);
    dateTime.tm_sec = atoi (value + 17);

    return mktime (& dateTime) - _timezone;
}

void parsePollResult (char *buffer) {
    int next = 0;
    auto *json = (json::hashNode *) json::parse (buffer, next);
    json::arrayNode *result = (json::arrayNode *) (*json) ["result"];

    if (result) {
        time_t timestamp;

        for (auto i = 0; i < result->size (); ++ i) {
            json::hashNode *field = (json::hashNode *) (*result) [i];

            if (field) {
                json::stringNode *fieldName = (json::stringNode *) (*field) ["name"];
                json::stringNode *fieldValue = (json::stringNode *) (*field) ["value"];

                if (fieldName && fieldValue) {
                    if (strcmp (fieldName->getValue (), "Poll.TimeStamp") == 0) {
                        printf ("Data at %s LT\n", fieldValue->getValue ());
                        timestamp = makeTimestamp (fieldValue->getValue ());
                    }
                }
            }
        }
    }
}

void poll (reqManager& reqMgr) {
    if (reqMgr.open ()) {
        static const size_t bufSize = 0xFFFF;
        char *buffer = (char *) malloc (bufSize);

        if (buffer) {
            printf ("Requesting...");

            if (reqMgr.sendRequest (buffer, bufSize)) {
                printf ("%zd bytes received\n", strlen (buffer));

                parsePollResult (buffer);
            }

            free (buffer);
        } else {
            printf ("Unable to allocate buffer\n");
        }
    }
}

void pollerProc (pollerContext *ctx, config& cfg) {
    reqManager reqMgr (cfg.port, (char *) cfg.host.c_str (), (char *) cfg.path.c_str ());

    while (ctx->keepRunning) {
        time_t now = time (0);

        if ((now - ctx->lastCheck) >= cfg.pollingInterval) {
            ctx->lastCheck = now;
            poll (reqMgr);
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