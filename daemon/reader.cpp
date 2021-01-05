#include <stdlib.h>
#include <vector>
#include <thread>
#include <chrono>
#include <WinSock2.h>

#include "../common/defs.h"
#include "../nmea/nmea.h"
#include "../common/db.h"
#include "logbook.h"

struct readerContext: pollerContext {
    SOCKET socket;
};

logbook::record logbookData;

std::vector<readerContext *> readerContexts;

void readerProc (readerContext *ctx, config& cfg, database& db, sensorCfg& sensor) {
    WSAData data;
    SOCKET transmitter;

    WSAStartup (0x202, & data);

    ctx->socket = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    transmitter = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    uint32_t yes = 1;

    setsockopt (ctx->socket, SOL_SOCKET, SO_REUSEADDR, (const char *) & yes, sizeof (yes));
    
    setsockopt (transmitter, SOL_SOCKET, SO_REUSEADDR, (const char *) & yes, sizeof (yes));
    setsockopt (transmitter, SOL_SOCKET, SO_BROADCAST, (const char *) & yes, sizeof (yes));

    sockaddr_in addr;

    addr.sin_addr.S_un.S_addr = inet_addr (sensor.nic.c_str ());
    addr.sin_family = AF_INET;
    addr.sin_port = htons (sensor.port);

    bind (ctx->socket, (const sockaddr *) & addr, sizeof (addr));

    addr.sin_addr.S_un.S_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = 0;

    bind (transmitter, (const sockaddr *) & addr, sizeof (addr));

    time_t lastAliveCheck = time (0), lastRecord = 0;
    time_t lastSentPosTime = 0;
    time_t lastSentSOGTime = 0;
    time_t lastSentCOGTime = 0;
    time_t lastSentHDGTime = 0;

    int32_t lastSentLat = nmea::NO_VALID_DATA;
    int32_t lastSentLon = nmea::NO_VALID_DATA;
    uint32_t lastSentSOG = nmea::NO_VALID_DATA;
    uint32_t lastSentCOG = nmea::NO_VALID_DATA;
    uint32_t lastSentHDG = nmea::NO_VALID_DATA;

    sockaddr_in transmitterDest;

    transmitterDest.sin_family = AF_INET;
    transmitterDest.sin_addr.S_un.S_addr = INADDR_BROADCAST;
    transmitterDest.sin_port = htons (7000);

    changedData packet { sizeof (packet) };

    auto transmit = [transmitter, & packet, transmitterDest] (changedDataType type) {
        packet.type = type;

        sendto (transmitter, (const char *) & packet, packet.size, 0, (sockaddr *) & transmitterDest, sizeof (transmitterDest));
    };

    while (ctx->keepRunning) {
        u_long available;
        time_t now = time (0);

        if (ioctlsocket (ctx->socket, FIONREAD, & available) == S_OK && available > 0) {
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

                    if (lat != lastSentLat || lon != lastSentLon || (now - lastSentPosTime) > 10) {
                        PostMessage (HWND_BROADCAST, cfg.posChangedMsg, lat, lon);
                        lastSentLat = lat;
                        lastSentLon = lon;
                        lastSentPosTime = now;

                        packet.lat = lat;
                        packet.lon = lon;
                        transmit (changedDataType::POS);
                    }

                    if (sog != lastSentSOG || (now - lastSentSOGTime) > 10) {
                        PostMessage (HWND_BROADCAST, cfg.sogChangedMsg, 0, sog);
                        lastSentSOG = sog;
                        lastSentSOGTime = now;

                        packet.value = sog;
                        transmit (changedDataType::SOG);
                    }

                    if (cog != lastSentCOG || (now - lastSentCOGTime) > 10) {
                        PostMessage (HWND_BROADCAST, cfg.cogChangedMsg, 0, cog);
                        lastSentCOG = cog;
                        lastSentCOGTime = now;

                        packet.value = cog;
                        transmit (changedDataType::COG);
                    }

                    if (hdg != lastSentHDG || (now - lastSentHDGTime) > 10) {
                        PostMessage (HWND_BROADCAST, cfg.hdgChangedMsg, 0, hdg);
                        lastSentHDG = hdg;
                        lastSentHDGTime = now;

                        packet.value = hdg;
                        transmit (changedDataType::HDG);
                    }
                }
            }
        }

        if ((now - lastAliveCheck) >= 5) {
            nmea::checkAlive (now);

            lastAliveCheck = now;

            logbookData.position.update (nmea::getLat (), nmea::getLon ());
            logbookData.sog.update (nmea::getSOG ());
            logbookData.cog.update (nmea::getCOG ());
            logbookData.hdg.update (nmea::getHDG ());
            logbookData.checkAlive (now);
        }

        if ((now - lastRecord) >= cfg.logbookPeriod ) {
            time_t timestamp = nmea::getTimestamp ();

            if (!timestamp) timestamp = time (0);
            
            db.addLogbookRecord (
                timestamp,
                logbookData.getLat (),
                logbookData.getLon (),
                logbookData.getCOG (),
                logbookData.getSOG (),
                logbookData.getHDG (),
                logbookData.getDraftFore (),
                logbookData.getDraftAft ()
            );

            lastRecord = now;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds (5));
    }

    closesocket (ctx->socket);
    closesocket (transmitter);
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
