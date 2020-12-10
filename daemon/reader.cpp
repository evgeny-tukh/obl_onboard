#include <stdlib.h>
#include <vector>
#include <thread>
#include <chrono>
#include <WinSock2.h>

#include "../common/defs.h"
#include "../nmea/nmea.h"
#include "../common/db.h"

struct readerContext: pollerContext {
    SOCKET socket;
};

std::vector<readerContext *> readerContexts;

void readerProc (readerContext *ctx, config& cfg, database& db, sensorCfg& sensor) {
    WSAData data;

    WSAStartup (0x202, & data);

    ctx->socket = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    uint32_t yes = 1;

    auto result = setsockopt (ctx->socket, SOL_SOCKET, SO_REUSEADDR, (const char *) & yes, sizeof (yes));

    sockaddr_in addr;

    addr.sin_addr.S_un.S_addr = inet_addr (sensor.nic.c_str ());
    addr.sin_family = AF_INET;
    addr.sin_port = htons (sensor.port);

    result = bind (ctx->socket, (const sockaddr *) & addr, sizeof (addr));

    time_t lastAliveCheck = time (0), lastRecord = 0;

    int32_t lastSentLat = nmea::NO_VALID_DATA;
    int32_t lastSentLon = nmea::NO_VALID_DATA;
    uint32_t lastSentSOG = nmea::NO_VALID_DATA;
    uint32_t lastSentCOG = nmea::NO_VALID_DATA;
    uint32_t lastSentHDG = nmea::NO_VALID_DATA;

    while (ctx->keepRunning) {
        u_long available;

        if (result = ioctlsocket (ctx->socket, FIONREAD, & available), result == S_OK && available > 0) {
            sockaddr_in sender;
            char buffer [2000];
            int senderSize = sizeof (sender);
            auto bytesRead = recvfrom (ctx->socket, buffer, available, 0, (sockaddr *) & sender, & senderSize);

            if (bytesRead > 0) {
                nmea::sentence sentence;

                buffer [bytesRead] = 0;
                
                if (sentence.parse (buffer)) {
                    nmea::parse (sentence);

                    int32_t lat = nmea::getEncodedLat ();
                    int32_t lon = nmea::getEncodedLon ();
                    uint32_t sog = nmea::getEncodedSOG ();
                    uint32_t cog = nmea::getEncodedCOG ();
                    uint32_t hdg = nmea::getEncodedHDG ();

                    if (lat != lastSentLat || lon != lastSentLon) {
                        PostMessage (HWND_BROADCAST, cfg.posChangedMsg, lat, lon);
                        lastSentLat = lat;
                        lastSentLon = lon;
                    }

                    if (sog != lastSentSOG) {
                        PostMessage (HWND_BROADCAST, cfg.sogChangedMsg, 0, sog);
                        lastSentSOG = sog;
                    }

                    if (cog != lastSentCOG) {
                        PostMessage (HWND_BROADCAST, cfg.cogChangedMsg, 0, cog);
                        lastSentCOG = cog;
                    }

                    if (hdg != lastSentHDG) {
                        PostMessage (HWND_BROADCAST, cfg.hdgChangedMsg, 0, hdg);
                        lastSentHDG = hdg;
                    }
                }
            }
        }

        time_t now = time (0);
        if ((now - lastAliveCheck) >= 5) {
            nmea::checkAlive (now);

            lastAliveCheck = now;
        }
        if ((now - lastRecord) >= cfg.logbookPeriod ) {
            db.addLogbookRecord (
                nmea::getTimestamp (),
                nmea::getLat (),
                nmea::getLon (),
                nmea::getCOG (),
                nmea::getSOG (),
                nmea::getHDG ()
            );

            lastRecord = now;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    closesocket (ctx->socket);
}

readerContext *startReader (config& cfg, database& db, sensorCfg& sensor) {
    readerContext *ctx = new readerContext;

    ctx->keepRunning = true;
    ctx->runner = new std::thread (readerProc, ctx, cfg, db, sensor);

    return ctx;
}

void startAllReaders (config& cfg, database& db) {
    for (auto& sensor: cfg.sensors) {
        readerContexts.push_back (startReader (cfg, db, sensor));
    }
}

void stopAllReaders () {
    for (auto ctx: readerContexts) ctx->keepRunning = false;

    for (auto ctx: readerContexts) {
        if (ctx->runner->joinable ()) ctx->runner->join ();
    }
}
