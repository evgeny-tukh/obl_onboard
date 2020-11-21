#pragma once

#include "sqlite3.h"

class database {
    public:
        database ();
        virtual ~database ();

        bool execute (char *query);
        long executeAndGet (char *query, sqlite3_callback cb, void *arg, char **errorMsg);

    protected:
        static const char *dbPath;
        sqlite3 *db;

        void initDb ();
};
