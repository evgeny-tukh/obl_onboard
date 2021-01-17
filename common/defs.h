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

enum layoutElementType {
    TANK = 1,
    IMAGE = 2,
    FUELMETER = 3,
    PIPE = 4,
};

enum layoutOrientation {
    HORIZONTAL = 1,
    VERTICAL = 2,
    UNKNOWN = 0,
};

enum layoutUnit {
    PERCENT = 1,
    PIXELS = 2,
};

enum layoutLabelPos {
    ABOVE = 1,
    BELOW = 2,
    LEFT = 3,
    RIGHT = 4,
};

enum layoutImage {
    NONE = 0,
    ENGINE = 100,
};

#define rgb(r,g,b)  ((b<<16)+(g<<8)+r)

enum penColor {
    BLACK = 0,
    RED = rgb (255, 0, 0),
    GREEN = rgb (0, 255, 0),
    BLUE = rgb (0, 0, 255),
    GRAY = rgb (180, 180, 180),
};

struct layoutNode {
    int x, y, offsetX, offsetY, width, height;

    layoutNode (int _x, int _y, int _offsX, int _offsY, int _width, int _height): x (_x), y (_y), offsetX (_offsX), offsetY (_offsY), width (_width), height (_height) {}
};

struct layoutElement: layoutNode {
    layoutElementType type;
    layoutOrientation orientation;
    layoutLabelPos labelPos;
    layoutUnit unit;
    std::string labelText;
    int id;
    std::vector<layoutNode> nodes;
    penColor color;

    layoutElement (
        layoutElementType _type,
        layoutUnit _unit,
        layoutLabelPos _lblPos,
        const char *_lblText,
        int _id,
        int _x,
        int _y,
        int _width,
        int _height,
        layoutOrientation _orient,
        int _offsX,
        int _offsY,
        penColor _color
    ):
        layoutNode (_x, _y, _offsX, _offsY, _width, _height),
        type (_type),
        unit (_unit),
        id (_id),
        labelPos (_lblPos),
        labelText (_lblText),
        orientation (_orient),
        color (_color) {
    }

    inline void addNode (int _x, int _y, int _offsX, int _offsY) {
        nodes.emplace_back (_x, _y, _offsX, _offsY, 0, 0);
    }
};

struct tank {
    uint16_t id;
    std::string name, type;
    float volume;
    char side;

    tank (const uint16_t _id, const char *_name, const char *_type, float _vol, const char *_side):
        name (_name), type (_type), volume (_vol), id (_id), side (*_side) {}
};

struct ship {
    std::string master, cheng, name;
    uint32_t mmsi, imo, mtID;
    float normalDraft;
};

struct reporting {
    std::string templatePath;
    std::string exportPath;
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

struct amounts {
    float reported, byVolume, byCounter;

    amounts (float rep, float byVol, float byCnt): reported (rep), byVolume (byVol), byCounter (byCnt) {}
    amounts (float value): reported (value), byVolume (value), byCounter (value) {}

    void copyFrom (amounts& from) {
        reported = from.reported;
        byVolume = from.byVolume;
        byCounter = from.byCounter;
    }
};

struct fuelState {
    float viscosity, sulphur;
    amounts volume, temp, density, quantity, vcf;

    fuelState ():
        density (0.95f),
        viscosity (380.0f),
        sulphur (1.5f),
        temp (45.0f),
        volume (500.0f),
        quantity (490.0f),
        vcf (0.95f) {}

    void copyFrom (fuelState& from) {
        viscosity = from.viscosity;
        sulphur = from.sulphur;

        volume.copyFrom (from.volume);
        quantity.copyFrom (from.quantity);
        vcf.copyFrom (from.vcf);
        temp.copyFrom (from.temp);
        density.copyFrom (from.density);
    }
};

struct tankState {
    uint32_t id, tank;
    fuelState before, after;

    tankState (uint32_t _tank, uint32_t _id = 0): tank (_tank), id (_id), before (), after () {}

    void copyFrom (tankState& from) {
        id = from.id;
        tank = from.tank;

        before.copyFrom (from.before);
        after.copyFrom (from.after);
    }
};

struct pipeMeters {
    float in, out;

    pipeMeters (): in (0.0f), out (0.0f) {}

    void copyFrom (pipeMeters& from) {
        in = from.in;
        out = from.out;
    }
};

struct draftData {
    float fore, aft;

    draftData (): fore (0.0f), aft (0.0) {}
    draftData (float _fore, float _aft): fore (_fore), aft (_aft) {}
    draftData (struct config&);

    void copyFrom (draftData& from) {
        fore = from.fore;
        aft = from.aft;
    }
};

struct sensorCfg {
    uint32_t port;
    std::string type;
    std::string nic;

    sensorCfg (const char *_type, const char *_nic, uint32_t _port): type (_type), nic (_nic), port (_port) {}
};

struct nmeaSource {
    char source [10];
    uint16_t port;

    nmeaSource (char *_source, uint16_t _port): port (_port) {
        strcpy (source, _source);
    }
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
    std::vector<nmeaSource> nmeaSources;
    ship shipInfo;
    time_t pollingInterval, timeout, logbookPeriod;
    uint32_t newDataMsg, posChangedMsg, sogChangedMsg, cogChangedMsg, hdgChangedMsg;
    std::map<uint8_t, param> params;
    std::map<uint8_t, paramGroup> paramGroups;
    std::map<char *, uint8_t> columnMap;
    reporting repCfg;
    std::vector<sensorCfg> sensors;
    std::string draftForeChannel, draftAftChannel;
    std::map<int, layoutElement> layout;

    nmeaSource *findNmeaSource (char *name) {
        for (auto& src: nmeaSources) {
            if (strcmp (src.source, name) == 0) return & src;
        }
        
        return 0;
    }

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

    fuelMeter *findUploadingMeter () {
        for (auto& fuelMeter: fuelMeters) {
            if (_stricmp (fuelMeter.type.c_str (), "upl") == 0) return & fuelMeter;
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

struct bunkeringData {
    uint32_t id;
    time_t begin, end;
    std::string port, barge;
    fuelState loaded;
    draftData draftBefore, draftAfter;
    std::vector<tankState> tankStates;

    bunkeringData (config& cfg, uint32_t _id = 0, char *_port = 0, char *_barge = 0):
        id (_id),
        draftBefore (cfg), draftAfter (cfg),
        begin (time (0) - 5400),
        end (time (0) - 1800),
        port (_port ? _port : "*"),
        barge (_barge ? _barge : "*") {
        for (auto& tank: cfg.tanks) {
            tankStates.emplace_back (tank.id);
        }
    }

    void copyFrom (bunkeringData& from) {
        id = from.id;
        begin = from.begin;
        end = from.end;
        port = from.port.c_str ();
        barge = from.barge.c_str ();

        loaded.copyFrom (from.loaded);
        draftBefore.copyFrom (from.draftBefore);
        draftAfter.copyFrom (from.draftAfter);

        for (auto& tank: from.tankStates) {
            tankStates.emplace_back (0);
            tankStates.back ().copyFrom (tank);
        }
    }
};

struct pollerContext {
    time_t lastCheck;
    std::thread *runner;
    bool keepRunning;

    pollerContext (): runner (0), keepRunning (false), lastCheck (0) {}
};

struct logbookRecord {
    time_t timestamp;
    std::pair<double, bool> lat, lon;
    std::pair<float, bool> cog, sog, hdg;
};

enum changedDataType {
    POS = 1,
    SOG = 2,
    COG = 3,
    STW = 4,
    HDG = 5,
};

#pragma pack(1)
struct changedData {
    uint16_t size;
    uint8_t type;

    union {
        struct { int32_t lat, lon; };
        uint32_t value; 
    };
};
#pragma pack()

extern pollerContext *startPoller (config& cfg);
extern void parseCfgFile (config& cfg);
