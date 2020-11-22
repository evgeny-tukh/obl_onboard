#pragma once

#include <cstdint>
#include "../common/sqlite3.h"

class database {
    public:
        database ();
        virtual ~database ();

        bool execute (char *query, uint64_t *insertID = 0);
        bool executeSimple (char *query, uint64_t *insertID = 0);
        long executeAndGet (char *query, sqlite3_callback cb, void *arg, char **errorMsg);

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
        static const char *dbPath;
        sqlite3 *db;

        void initDb ();
};
