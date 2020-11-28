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

    "create table volumes(id integer not null primary key asc,tank integer not null,timestamp integer not null,value real not null)",
    "create index idx_vol1 on volumes(tank,timestamp)",
    "create index idx_vol2 on volumes(timestamp,tank)",

    "create table meters(id integer not null primary key asc,meter integer not null,timestamp integer not null,value real not null)",
    "create index idx_mtr1 on meters(meter,timestamp)",
    "create index idx_mtr2 on meters(timestamp,meter)",

    "create table bunkerings("
    "id integer not null primary key asc,"
    "begin integer not null,"
    "end integer not null,"
    "port text,"
    "barge text,"
    "draft_fore_before real,"
    "draft_aft_before real,"
    "fm_in_value_before real,"
    "fm_out_value_before real,"
    "draft_fore_after real,"
    "draft_aft_after real,"
    "fm_in_value_after real,"
    "fm_out_value_after real)",
    "create index idx_bunk1 on bunkerings(begin)",
    "create index idx_bunk2 on bunkerings(end)",

    "create table tank_state("
    "id integer not null primary key asc,"
    "bunkering integer not null,"
    "tank integer not null,"
    "density_before real not null,"
    "viscosity_before real not null,"
    "sulphur_before real not null,"
    "temp_before real not null,"
    "volume_before real not null,"
    "quantity_before real not null,"
    "vcf_before real not null,"
    "density_after real not null,"
    "viscosity_after real not null,"
    "sulphur_after real not null,"
    "temp_after real not null,"
    "volume_after real not null,"
    "quantity_after real not null,"
    "vcf_after real not null)",
    "create index idx_tankstate1 on tank_state(bunkering,tank)",
    "create index idx_tankstate2 on tank_state(tank,bunkering)",

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

void database::addData (uint32_t object, time_t timestamp, dataValueType type, double value) {
    char query [100];
    const char *table;
    const char *field;

    switch (type) {
        case dataValueType::tankVolume:
            table = "volumes";
            field = "tank";
            break;
        case dataValueType::fuelMeter:
            table = "meters";
            field = "meter";
            break;
        default:
            return;
    }

    sprintf (query, "insert into %s(%s,timestamp,value) values(%d,%I64d,%f)", table, field, object, timestamp, value);
    executeSimple (query);
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

uint64_t database::createBunkering (bunkeringData& data) {
    /*char query [300];
    uint64_t result;

    sprintf (
        query, 
        "insert into bunkerings(tank,begin,end,port,barge,density,viscosity,sulphur,temp,volume,quantity) "
        "values(%d,%zd,%zd,'%s','%s',%.4f,%.2f,%.2f,%.1f,%.3f,%.3f)",
        data.tank, data.begin, data.end, data.port, data.barge, data.density, data.viscosity, data.sulphur, data.temp, data.volume, data.quantity
    );
    executeSimple (query, & result);

    return result;*/return 0;
}

int bunkeringListLoadCb (void *param, int numOfFields, char **values, char **fields) {
    /*bunkeringList *list = (bunkeringList *) param;

    list->emplace_back (
        (uint32_t) atol (values [0]),   //id (_id),
        (uint32_t) atol (values [1]) ,  //tank (0),
        (time_t) _atoi64 (values [2]),  //begin (time (0) - 5400),
        (time_t) _atoi64 (values [3]),  //end (time (0) - 1800),
        values [4],                     //port ("-"),
        values [5],                     //barge ("-"),
        (float) atof (values [6]),      //density (0.95f),
        (float) atof (values [7]),      //viscosity (380.0f),
        (float) atof (values [8]),      //sulphur (1.5f),
        (float) atof (values [9]),      //temp (45.0f),
        (float) atof (values [10]),     //volume (0.0f),
        (float) atof (values [11])      //quantity (0.0f)
    );*/

    return 0;
}

size_t database::loadBunkeringList (uint8_t tank, bunkeringList& list, time_t begin, time_t end) {
    list.clear ();

    /*char query [300];
    sprintf (
        query,
        "select id,tank,begin,end,port,barge,density,viscosity,sulphur,temp,volume,quantity from bunkerings where tank=%d and begin>=%lld and end<=%lld order by begin",
        tank,
        begin,
        end
    );
    executeAndGet (query, bunkeringListLoadCb, & list, 0);*/

    return list.size ();
}

bool database::getBunkering (uint32_t id, bunkeringData& data) {
    /*bunkeringList list;
    char query [300];
    sprintf (query, "select id,tank,begin,end,port,barge,density,viscosity,sulphur,temp,volume,quantity from bunkerings where id=%d", id);
    executeAndGet (query, bunkeringListLoadCb, & list, 0);

    bool result = list.size () > 0;

    if (result) memcpy (& data, & list.front (), sizeof (data));

    return result;*/return false;
}

void database::saveBunkering (bunkeringData& data) {
    /*char query [300];

    sprintf (
        query, 
        "update bunkerings set begin=%I64d,end=%I64d,port='%s',barge='%s',density=%.4f,viscosity=%.2f,sulphur=%.2f,temp=%.1f,volume=%.3f,quantity=%3f where id=%d",
        data.begin, data.end, data.port, data.barge, data.density, data.viscosity, data.sulphur, data.temp, data.volume, data.quantity, data.id
    );
    executeSimple (query);*/
}

void database::deleteBunkering (uint32_t id) {
    char query [100];
    sprintf (query, "delete from bunkerings where id=%d", id);
    executeSimple (query);
}