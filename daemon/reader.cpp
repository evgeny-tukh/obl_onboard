#include <stdlib.h>
#include <vector>
#include <thread>
#include <chrono>
#include <WinSock2.h>

#include "../common/defs.h"
#include "../nmea/nmea.h"

struct readerContext: pollerContext {
    SOCKET socket;
};

std::vector<readerContext *> readerContexts;

void readerProc (readerContext *ctx, config& cfg, sensorCfg& sensor) {
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
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    closesocket (ctx->socket);
}

readerContext *startReader (config& cfg, sensorCfg& sensor) {
    readerContext *ctx = new readerContext;

    ctx->keepRunning = true;
    ctx->runner = new std::thread (readerProc, ctx, cfg, sensor);

    return ctx;
}

void startAllReaders (config& cfg) {
    for (auto& sensor: cfg.sensors) {
        readerContexts.push_back (startReader (cfg, sensor));
    }
}

void stopAllReaders () {
    for (auto ctx: readerContexts) ctx->keepRunning = false;

    for (auto ctx: readerContexts) {
        if (ctx->runner->joinable ()) ctx->runner->join ();
    }
}
