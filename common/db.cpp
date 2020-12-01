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
    "fm_out_value_after real,"
    "density real not null,"
    "viscosity real not null,"
    "sulphur real not null,"
    "temp real not null,"
    "volume real not null,"
    "quantity real not null,"
    "vcf real not null,"
    "fuelmeter real not null)",
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
    "fuelmeter_before real not null,"
    "density_after real not null,"
    "viscosity_after real not null,"
    "sulphur_after real not null,"
    "temp_after real not null,"
    "volume_after real not null,"
    "quantity_after real not null,"
    "vcf_after real not null,"
    "fuelmeter_after real not null)",
    "create index idx_tankstate1 on tank_state(bunkering,tank)",
    "create index idx_tankstate2 on tank_state(tank,bunkering)",

    0,
};

database::database (config& _cfg): cfg (_cfg) {
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

void database::saveBunkering (bunkeringData& data) {
    char query [500];

    sprintf (
        query, 
        "update bunkerings set "
        "begin=%zd,end=%zd,port='%s',barge='%s',"
        "draft_fore_before=%f,draft_aft_before=%f,fm_in_value_before=%f,fm_out_value_before=%f,"
        "draft_fore_after=%f,draft_aft_after=%f,fm_in_value_after=%f,fm_out_value_after=%f,"
        "density=%f,viscosity=%f,sulphur=%f,temp=%f,volume=%f,quantity=%f,vcf=%f,fuelmeter=%f "
        "where id=%d",
        data.begin, data.end, data.port.c_str (), data.barge.c_str (),
        data.draftBefore.fore, data.draftBefore.aft, data.pmBefore.in, data.pmBefore.out,
        data.draftAfter.fore, data.draftAfter.aft, data.pmAfter.in, data.pmAfter.out,
        data.loaded.density, data.loaded.viscosity, data.loaded.sulphur, data.loaded.temp, data.loaded.volume, data.loaded.quantity, data.loaded.vcf, data.loaded.fuelMeter,
        data.id
    );
    executeSimple (query);

    for (auto& tankState: data.tankStates) {
        sprintf (
            query, 
            "update tank_state "
            "set density_before=%f,viscosity_before=%f,sulphur_before=%f,temp_before=%f,volume_before=%f,quantity_before=%f,vcf_before=%f,fuelmeter_before=%f,"
            "density_after=%f,viscosity_after=%f,sulphur_after=%f,temp_after=%f,volume_after=%f,quantity_after=%f,vcf_after=%f,fuelmeter_after=%f "
            "where id=%d and bunkering=%d",
            tankState.before.density, tankState.before.viscosity, tankState.before.sulphur, tankState.before.temp, tankState.before.volume, tankState.before.quantity,
            tankState.before.vcf, tankState.before.fuelMeter, tankState.after.density, tankState.after.viscosity, tankState.after.sulphur, tankState.after.temp,
            tankState.after.volume, tankState.after.quantity, tankState.after.vcf, tankState.after.fuelMeter, tankState.id, data.id
        );
        executeSimple (query);
    }
}

uint64_t database::createBunkering (bunkeringData& data) {
    char query [500];
    uint64_t bunkeringID, stateID;

    sprintf (
        query, 
        "insert into bunkerings"
        "(begin,end,port,barge,"
        "draft_fore_before,draft_aft_before,fm_in_value_before,fm_out_value_before,"
        "draft_fore_after,draft_aft_after,fm_in_value_after,fm_out_value_after,"
        "density,viscosity,sulphur,temp,volume,quantity,vcf,fuelmeter) "
        "values(%zd,%zd,'%s','%s',%.1f,%.1f,%.3f,%.3f,%.1f,%.1f,%.3f,%.3f,%.4f,%.2f,%.2f,%.1f,%.3f,%.3f,%.4f,%.4f)",
        data.begin, data.end, data.port.c_str (), data.barge.c_str (),
        data.draftBefore.fore, data.draftBefore.aft, data.pmBefore.in, data.pmBefore.out,
        data.draftAfter.fore, data.draftAfter.aft, data.pmAfter.in, data.pmAfter.out,
        data.loaded.density, data.loaded.viscosity, data.loaded.sulphur, data.loaded.temp, data.loaded.volume, data.loaded.quantity, data.loaded.vcf, data.loaded.fuelMeter
    );
    executeSimple (query, & bunkeringID);

    data.id = bunkering;

    for (auto& tankState: data.tankStates) {
        sprintf (
            query, 
            "insert into tank_state"
            "(bunkering,tank,"
            "density_before,viscosity_before,sulphur_before,temp_before,volume_before,quantity_before,vcf_before,fuelmeter_before,"
            "density_after,viscosity_after,sulphur_after,temp_after,volume_after,quantity_after,vcf_after,fuelmeter_after) "
            "values(%I64d,%d,%.4f,%.2f,%.2f,%.1f,%.3f,%.3f,%.4f,%.4f,%.4f,%.2f,%.2f,%.1f,%.3f,%.3f,%.4f,%.4f)",
            bunkeringID, tankState.tank,
            tankState.before.density, tankState.before.viscosity, tankState.before.sulphur, tankState.before.temp, tankState.before.volume, tankState.before.quantity,
            tankState.before.vcf, tankState.before.fuelMeter,
            tankState.after.density, tankState.after.viscosity, tankState.after.sulphur, tankState.after.temp, tankState.after.volume, tankState.after.quantity,
            tankState.after.vcf, tankState.after.fuelMeter
        );
        executeSimple (query, & stateID);

        tankState.id = stateID;
    }

    return bunkeringID;
}

int bunkeringListLoadCb (void *param, int numOfFields, char **values, char **fields) {
    bunkeringContext *ctx = (bunkeringContext *) param;
    bunkeringData *bunkData = ctx->list.size () > 0 ? & ctx->list.back () : 0;
    uint32_t lastBunkeringID = ctx->list.size () > 0 ? ctx->list.back ().id : 0;

    auto atoF = [] (char *strValue) {
        return strValue ? (float) atof (strValue) : 0.0f;
    };
    auto loadFuelState = [values, atoF] (fuelState& state, int start) {
        state.density = atoF (values [start]);
        state.viscosity = atoF (values [start+1]);
        state.sulphur = atoF (values [start+2]);
        state.temp = atoF (values [start+3]);
        state.volume = atoF (values [start+4]);
        state.quantity = atoF (values [start+5]);
        state.vcf = atoF (values [start+6]);
        state.fuelMeter = atoF (values [start+7]);
    };

    uint32_t bunkeringID = atoi (values [0]);

    if (lastBunkeringID != bunkeringID) {
        ctx->list.emplace_back (ctx->cfg, bunkeringID, values [3], values [4]);
        bunkData = & ctx->list.back ();

        bunkData->begin = atol (values [1]);
        bunkData->end = atol (values [2]);
        bunkData->draftBefore.fore = atoF (values [5]);
        bunkData->draftBefore.aft = atoF (values [6]);
        bunkData->pmBefore.in = atoF (values [7]);
        bunkData->pmBefore.out = atoF (values [8]);
        bunkData->draftAfter.fore = atoF (values [9]);
        bunkData->draftAfter.aft = atoF (values [10]);
        bunkData->pmAfter.in = atoF (values [11]);
        bunkData->pmAfter.out = atoF (values [12]);
        
        loadFuelState (bunkData->loaded, 13);
        bunkData->tankStates.clear ();
    }

    bunkData->tankStates.emplace_back (atol (values [23]), atol (values [21]));

    tankState& tank = bunkData->tankStates.back ();

    loadFuelState (tank.before, 24);
    loadFuelState (tank.after, 32);

    return 0;
}

size_t database::loadBunkeringList (uint8_t tank, bunkeringList& list, time_t begin, time_t end) {
    bunkeringContext ctx = { list, cfg };
    list.clear ();

    char query [500];
    sprintf (
        query,
        "select * from bunkerings b left join tank_state s on b.id=s.bunkering where begin>=%lld and end<=%lld order by begin,tank",
        begin,
        end
    );
    executeAndGet (query, bunkeringListLoadCb, & ctx, 0);

    return list.size ();
}

bool database::getBunkering (uint32_t id, bunkeringData& data) {
    bunkeringList list;
    char query [300];
    sprintf (query, "select * from bunkerings b left join tank_state s on b.id=s.bunkering where b.id=%d", id);
    executeAndGet (query, bunkeringListLoadCb, & list, 0);

    bool result = list.size () > 0;

    if (result) data.copyFrom (list [0]);

    return result;
}

void database::deleteBunkering (uint32_t id) {
    char query [100];
    sprintf (query, "delete from bunkerings where id=%d", id);
    executeSimple (query);
}