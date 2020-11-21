#include <stdlib.h>
#include <stdio.h>
#include "db.h"

const char *database::dbPath = "data.db";

char *initialQueries [] {
    "create table operations"
    "(id integer not null primary key asc,type integer not null,tank integer not null,timestamp integer not null)",
    "create index idx_tank on operations(tank,timestamp,type)",
    "create index idx_tank2 on operations(tank,type,timestamp)",
    "create index idx_timestamp on operations(timestamp,tank,type)",
    "create index idx_type on operations(type,timestamp,tank)",
    "create index idx_type2 on operations(type,tank,timestamp)",
    "create table data(id integer not null primary key asc,"
    "operation integer not null,parameter integer not null,column integer not null,value real not null)",
    "create index idx_order on data(operation,parameter,column)",

    0,
};

database::database () {
    if (sqlite3_open_v2 (dbPath, & db, SQLITE_OPEN_READWRITE, 0) == SQLITE_OK) {
        return;
    } else if (sqlite3_open (dbPath, & db) == SQLITE_OK) {
        initDb (); return;
    }

    printf ("Unable to open database file\n");
    exit (0);
}

database::~database () {
    sqlite3_close (db);
}


void database::initDb () {
    for (auto i = 0; initialQueries [i]; execute (initialQueries [i++]));
}

bool database::execute (char *query, uint64_t *insertID) {
    sqlite3_stmt *statement;
    const char *queryTail;
    bool result = sqlite3_prepare (db, query, -1, & statement, & queryTail) == SQLITE_OK;

    if (result) {
        result &= (sqlite3_step (statement) == SQLITE_DONE);

        if (insertID) *insertID = sqlite3_last_insert_rowid (db);

        result &= (sqlite3_finalize (statement) == SQLITE_OK);
    }

    return result;
}

bool database::executeSimple (char *query, uint64_t *insertID) {
    auto result = sqlite3_exec (db, query, 0, 0, 0) == SQLITE_OK ;

    if (result && insertID) *insertID = sqlite3_last_insert_rowid (db);

    return result;
}

long database::executeAndGet (char *query, sqlite3_callback cb, void *arg, char **errorMsg) {
    long result = 0L;

    if (sqlite3_exec (db, query, cb, arg, errorMsg) == SQLITE_OK) {
        result = (long) sqlite3_last_insert_rowid (db);
    }

    return result;
}

uint32_t database::addFuelOperation (
    uint32_t tank,
    uint8_t operationType,
    time_t timestamp
) {
    char query [100];
    uint64_t operation = 0;

    sprintf (query, "insert into operations(type,tank,timestamp) values(%d,%d,%zd)", operationType, tank, timestamp);
    
    return executeSimple (query, & operation) ? operation : 0;
}

void database::addFuelParameter (
    uint64_t operation,
    uint16_t parameter,
    uint8_t column,
    double value
) {
    char query [100];

    sprintf (query, "insert into data(operation,parameter,column,value) values(%zd,%d,%d,%f)", operation, parameter, column, value);
    execute (query);
}
