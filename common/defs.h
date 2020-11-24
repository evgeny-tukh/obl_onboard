#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <thread>

enum dataColumn {
    bunkeringReport = 0,
    sensorData = 1,
    hoppeData = 2,
    analyzeResult = 3,
};

enum fuelOperType {
    operate = 1,
    bunkering = 2,
    move = 3,

    unknown = 0,
};

enum tankSide {
    starboard = 's',
    port = 'p',
    center = 'c',
};

struct fuelMeter {
    uint16_t id;
    std::string name, type;

    fuelMeter (const uint16_t _id, const char *_name, const char *_type): name (_name), type (_type), id (_id) {}
};

struct tank {
    uint16_t id;
    std::string name, type;
    float volume;
    char side;

    tank (const uint16_t _id, const char *_name, const char *_type, float _vol, const char *_side): name (_name), type (_type), volume (_vol), id (_id), side (*_side) {}
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
    float timezone;
    std::string cfgFile;
    bool queryData;
    std::vector<tank> tanks;
    std::vector<fuelMeter> fuelMeters;
    ship shipInfo;
    time_t pollingInterval;
    std::map<uint8_t, param> params;
    std::map<uint8_t, paramGroup> paramGroups;
    std::map<char *, uint8_t> columnMap;

    tank *findTank (char *name) {
        for (auto& tank: tanks) {
            if (strcmp (tank.name.c_str (), name) == 0) return & tank;
        }

        return 0;
    }

    fuelMeter *findFuelMeter (char *name) {
        for (auto& fuelMeter: fuelMeters) {
            if (strcmp (fuelMeter.name.c_str (), name) == 0) return & fuelMeter;
        }

        return 0;
    }

    param *findParam (char *key) {
        for (auto iter = params.begin (); iter != params.end (); ++ iter) {
            if (strcmp (iter->second.key.c_str (), key) == 0) return & iter->second;
        }

        return 0;
    }

    int findColumnMap (char *key) {
        for (auto iter = columnMap.begin (); iter != columnMap.end (); ++ iter) {
            if (strcmp (iter->first, key) == 0) return (int) iter->second;
        }

        return -1;
    }

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
extern void parseCfgFile (config& cfg);
