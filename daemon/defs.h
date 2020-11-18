#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <thread>

struct tank {
    std::string name, type;
    float volume;

    tank (const char *_name, const char *_type, float _vol): name (_name), type (_type), volume (_vol) {}
};

struct ship {
    std::string master, cheng, name;
    uint32_t mmsi, imo, mtID;
};

struct config {
    std::string host;
    uint16_t port;
    uint64_t begin, end;
    std::string cfgFile;
    bool queryData;
    std::vector<tank> tanks;
    ship shipInfo;
    time_t pollingInterval;

    config () : port (3500), queryData (false), begin (0), end (0), pollingInterval (120) {
    }
};

struct pollerContext {
    time_t lastCheck;
    std::thread *runner;
    bool keepRunning;

    pollerContext (): runner (0), keepRunning (false), lastCheck (0) {}
};

extern pollerContext *startPoller (config& cfg);
