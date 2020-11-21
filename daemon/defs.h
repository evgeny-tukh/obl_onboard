#pragma once

#include <string>
#include <vector>
#include <map>
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

struct paramGroup {
    uint8_t id;
    std::string key, name;

    paramGroup (
        uint8_t _id,
        const char *_key,
        const char *_name
    ): id (_id), key (_key), name (_name) {}
};

struct param {
    std::string key, name;
    uint8_t id, multiplier, group;
    bool isNumber;

    param (): key (""), name (""), id (0), multiplier (0), group (0), isNumber (false) {}

    param (
        uint8_t _id,
        const char *_key,
        const char *_name,
        uint8_t _mult,
        uint8_t _isNum,
        uint8_t _grp
    ): id (_id), key (_key), name (_name), multiplier (_mult), isNumber (_isNum != 0), group (_grp) {}
};

struct config {
    std::string host, path;
    uint16_t port;
    uint64_t begin, end;
    std::string cfgFile;
    bool queryData;
    std::vector<tank> tanks;
    ship shipInfo;
    time_t pollingInterval;
    std::map<uint8_t, param> params;
    std::map<uint8_t, paramGroup> paramGroups;

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
