#include <stdlib.h>
#include <thread>
#include <WinSock2.h>
#include "../common/defs.h"

void agentProc (config& cfg, pollerContext *context) {
    SOCKET receiver;
    uint32_t yes = 1;
    sockaddr_in local;
    WSAData data;

    WSAStartup (0x202, & data);

    receiver = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    local.sin_addr.S_un.S_addr = INADDR_ANY;
    local.sin_port = htons (7000);
    local.sin_family = AF_INET;

    setsockopt (receiver, SOL_SOCKET, SO_REUSEADDR, (const char *) & yes, sizeof (yes));
    bind (receiver, (const sockaddr *) & local, sizeof (local));

    while (context->keepRunning) {
        u_long available;

        if (ioctlsocket (receiver, FIONREAD, & available) == S_OK && available > 0) {
            sockaddr_in sender;
            char buffer [300];
            changedData *data = (changedData *) buffer;
            int senderSize = sizeof (sender);
            auto bytesRead = recvfrom (receiver, buffer, available, 0, (sockaddr *) & sender, & senderSize);

            if (bytesRead > 0) {
                switch (data->type) {
                    case changedDataType::POS:
                        PostMessage (HWND_BROADCAST, cfg.posChangedMsg, data->lat, data->lon); break;
                    case changedDataType::COG:
                        PostMessage (HWND_BROADCAST, cfg.cogChangedMsg, data->value, 0); break;
                    case changedDataType::SOG:
                        PostMessage (HWND_BROADCAST, cfg.sogChangedMsg, data->value, 0); break;
                    case changedDataType::HDG:
                        PostMessage (HWND_BROADCAST, cfg.hdgChangedMsg, data->value, 0); break;
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds (5));
    }

    closesocket (receiver);
}

pollerContext *startAgent (config& cfg) {
    pollerContext *context = new pollerContext;

    context->keepRunning = true;
    context->runner = new std::thread (agentProc, cfg, context);

    return context;
}

void stopAgent (pollerContext *context) {
    context->keepRunning = false;

    if (context->runner->joinable ()) context->runner->join ();

    delete context;
}
