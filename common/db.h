#pragma once

#include <cstdint>
#include <vector>
#include "sqlite3.h"
#include "defs.h"

typedef std::vector<bunkeringData> bunkeringList;

struct bunkeringContext {
    bunkeringList& list;
    config& cfg;
};

class database {
    public:
        database (config&);
        virtual ~database ();

        bool execute (char *query, uint64_t *insertID = 0);
        bool executeSimple (char *query, uint64_t *insertID = 0);
        long executeAndGet (char *query, sqlite3_callback cb, void *arg, char **errorMsg);

        enum dataValueType {
            tankVolume = 1,
            fuelMeter,
        };

        void addData (uint32_t object, time_t timestamp, dataValueType type, double value);

        double getSingleValue (char *query);

        typedef std::map<uint32_t, double> valueMap;
        void collectCurrentVolumes (valueMap& volumes);
        void collectCurrentMeters (valueMap& volumes);

        uint64_t createBunkering (bunkeringData& data);
        void saveBunkering (bunkeringData& data);
        size_t loadBunkeringList (uint8_t tank, bunkeringList& list, time_t begin = 0, time_t end = 3000000000);
        bool getBunkering (uint32_t id, bunkeringData& data);
        void deleteBunkering (uint32_t id);
        double getLastMeterValue (uint32_t id);

        uint32_t addFuelOperation (
            uint32_t tank,
            uint8_t operationType,
            time_t timestamp
        );

        void addFuelParameter (
            uint64_t operation,
            uint16_t parameter,
            uint8_t column,
            double value
        );
    protected:
        config& cfg;
        static const char *dbPath;
        sqlite3 *db;

        void initDb ();
};
