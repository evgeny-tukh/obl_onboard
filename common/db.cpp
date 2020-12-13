#include <stdlib.h>
#include <stdio.h>
#include "db.h"
#include "tools.h"

const char *database::dbPath = "data.db";

char *initialQueries [] {
    "create table logbook"
    "(id integer not null primary key asc,"
    "timestamp integer not null,"
    "lat real,lon real,"
    "sog real,cog real,hdg real)",
    "create index idx_lb on logbook(timestamp)",

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
    "volume_rep real not null,"
    "volume_by_vol real not null,"
    "volume_by_cnt real not null,"
    "quantity_rep real not null,"
    "quantity_by_vol real not null,"
    "quantity_by_cnt real not null,"
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
    "volume_before_rep real not null,"
    "volume_before_by_vol real not null,"
    "volume_before_by_cnt real not null,"
    "quantity_before_rep real not null,"
    "quantity_before_by_vol real not null,"
    "quantity_before_by_cnt real not null,"
    "vcf_before real not null,"
    "fuelmeter_before real not null,"
    "density_after real not null,"
    "viscosity_after real not null,"
    "sulphur_after real not null,"
    "temp_after real not null,"
    "volume_after_rep real not null,"
    "volume_after_by_vol real not null,"
    "volume_after_by_cnt real not null,"
    "quantity_after_rep real not null,"
    "quantity_after_by_vol real not null,"
    "quantity_after_by_cnt real not null,"
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

    sprintf (query, "insert into operations(type,tank,timestamp) values(%d,%d,%I64d)", operationType, tank, timestamp);
    
    return executeSimple (query, & operation) ? operation : 0;
}

void database::addFuelParameter (
    uint64_t operation,
    uint16_t parameter,
    uint8_t column,
    double value
) {
    char query [100];

    sprintf (query, "insert into data(operation,parameter,column,value) values(%I64d,%d,%d,%f)", operation, parameter, column, value);
    execute (query);
}

void database::saveBunkering (bunkeringData& data) {
    char query [1000];

    sprintf (
        query, 
        "update bunkerings set "
        "begin=%I64d,end=%I64d,port='%s',barge='%s',"
        "draft_fore_before=%f,draft_aft_before=%f,fm_in_value_before=%f,fm_out_value_before=%f,"
        "draft_fore_after=%f,draft_aft_after=%f,fm_in_value_after=%f,fm_out_value_after=%f,"
        "density=%f,viscosity=%f,sulphur=%f,temp=%f,volume_rep=%f,volume_by_vol=%f,volume_by_cnt=%f,"
        "quantity_rep=%f,quantity_by_vol=%f,quantity_by_cnt=%f,vcf=%f,fuelmeter=%f "
        "where id=%d",
        data.begin, data.end, data.port.c_str (), data.barge.c_str (),
        data.draftBefore.fore, data.draftBefore.aft, data.pmBefore.in, data.pmBefore.out,
        data.draftAfter.fore, data.draftAfter.aft, data.pmAfter.in, data.pmAfter.out,
        data.loaded.density, data.loaded.viscosity, data.loaded.sulphur, data.loaded.temp, 
        data.loaded.volume.reported, data.loaded.volume.byVolume, data.loaded.volume.byCounter,
        data.loaded.quantity.reported, data.loaded.quantity.byVolume, data.loaded.quantity.byCounter,
        data.loaded.vcf, data.loaded.fuelMeter,
        data.id
    );
    executeSimple (query);

    for (auto& tankState: data.tankStates) {
        sprintf (
            query, 
            "update tank_state "
            "set density_before=%f,viscosity_before=%f,sulphur_before=%f,temp_before=%f,"
            "volume_before_rep=%f,volume_before_by_vol=%f,volume_before_by_cnt=%f,"
            "quantity_before_rep=%f,quantity_before_by_vol=%f,quantity_before_by_cnt=%f,"
            "vcf_before=%f,fuelmeter_before=%f,"
            "density_after=%f,viscosity_after=%f,sulphur_after=%f,temp_after=%f,"
            "volume_after_rep=%f,volume_after_by_vol=%f,volume_after_by_cnt=%f,"
            "quantity_after_rep=%f,quantity_after_by_vol=%f,quantity_after_by_cnt=%f,"
            "vcf_after=%f,fuelmeter_after=%f "
            "where id=%d and bunkering=%d",
            tankState.before.density, tankState.before.viscosity, tankState.before.sulphur, tankState.before.temp,
            tankState.before.volume.reported, tankState.before.volume.byVolume, tankState.before.volume.byCounter,
            tankState.before.quantity.reported, tankState.before.quantity.byVolume, tankState.before.quantity.byCounter,
            tankState.before.vcf, tankState.before.fuelMeter, tankState.after.density, tankState.after.viscosity, tankState.after.sulphur, tankState.after.temp,
            tankState.after.volume.reported, tankState.after.volume.byVolume, tankState.after.volume.byCounter,
            tankState.after.quantity.reported, tankState.after.quantity.byVolume, tankState.after.quantity.byCounter,
            tankState.after.vcf, tankState.after.fuelMeter, tankState.id, data.id
        );
        executeSimple (query);
    }
}

uint64_t database::createBunkering (bunkeringData& data) {
    char query [1000];
    uint64_t bunkeringID, stateID;

    sprintf (
        query, 
        "insert into bunkerings"
        "(begin,end,port,barge,"
        "draft_fore_before,draft_aft_before,fm_in_value_before,fm_out_value_before,"
        "draft_fore_after,draft_aft_after,fm_in_value_after,fm_out_value_after,"
        "density,viscosity,sulphur,temp,volume_rep,volume_by_vol,volume_by_cnt,quantity_rep,quantity_by_vol,quantity_by_cnt,vcf,fuelmeter) "
        "values(%I64d,%I64d,'%s','%s',%.1f,%.1f,%.3f,%.3f,%.1f,%.1f,%.3f,%.3f,%.4f,%.2f,%.2f,%.1f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.4f,%.4f)",
        data.begin, data.end, data.port.c_str (), data.barge.c_str (),
        data.draftBefore.fore, data.draftBefore.aft, data.pmBefore.in, data.pmBefore.out,
        data.draftAfter.fore, data.draftAfter.aft, data.pmAfter.in, data.pmAfter.out,
        data.loaded.density, data.loaded.viscosity, data.loaded.sulphur, data.loaded.temp,
        data.loaded.volume.reported, data.loaded.volume.byVolume, data.loaded.volume.byCounter,
        data.loaded.quantity.reported, data.loaded.quantity.byVolume, data.loaded.quantity.byCounter,
        data.loaded.vcf, data.loaded.fuelMeter
    );
    executeSimple (query, & bunkeringID);

    data.id = bunkering;

    for (auto& tankState: data.tankStates) {
        sprintf (
            query, 
            "insert into tank_state"
            "(bunkering,tank,"
            "density_before,viscosity_before,sulphur_before,temp_before,"
            "volume_before_rep,volume_before_by_vol,volume_before_by_cnt,"
            "quantity_before_rep,quantity_before_by_vol,quantity_before_by_cnt,"
            "vcf_before,fuelmeter_before,"
            "density_after,viscosity_after,sulphur_after,temp_after,"
            "volume_after_rep,volume_after_by_vol,volume_after_by_cnt,"
            "quantity_after_rep,quantity_after_by_vol,quantity_after_by_cnt,"
            "vcf_after,fuelmeter_after) "
            "values(%I64d,%d,%.4f,%.2f,%.2f,%.1f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.4f,%.4f,%.4f,%.2f,%.2f,%.1f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.4f,%.4f)",
            bunkeringID, tankState.tank,
            tankState.before.density, tankState.before.viscosity, tankState.before.sulphur, tankState.before.temp,
            tankState.before.volume.reported, tankState.before.volume.byVolume, tankState.before.volume.byCounter,
            tankState.before.quantity.reported, tankState.before.quantity.byVolume, tankState.before.quantity.byCounter,
            tankState.before.vcf, tankState.before.fuelMeter,
            tankState.after.density, tankState.after.viscosity, tankState.after.sulphur, tankState.after.temp,
            tankState.after.volume.reported, tankState.after.volume.byVolume, tankState.after.volume.byCounter,
            tankState.after.quantity.reported, tankState.after.quantity.byVolume, tankState.after.quantity.byCounter,
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
        state.volume.reported = atoF (values [start+4]);
        state.volume.byVolume = atoF (values [start+5]);
        state.volume.byCounter = atoF (values [start+6]);
        state.quantity.reported = atoF (values [start+7]);
        state.quantity.byVolume = atoF (values [start+8]);
        state.quantity.byCounter = atoF (values [start+9]);
        state.vcf = atoF (values [start+10]);
        state.fuelMeter = atoF (values [start+11]);
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

    bunkData->tankStates.emplace_back (atol (values [27]), atol (values [25]));

    tankState& tank = bunkData->tankStates.back ();

    loadFuelState (tank.before, 28);
    loadFuelState (tank.after, 40);

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

int singleValueGetCb (void *param, int numOfFields, char **values, char **fields) {
    *((double *) param) = values [0] ? atof (values [0]) : 0.0;

    // do not continue
    return 1;
}

double database::getSingleValue (char *query) {
    double result;

    executeAndGet (query, singleValueGetCb, & result, 0);

    return result;
}

double database::getLastMeterValue (uint32_t id) {
    char query [100];

    sprintf (query, "select value from meters where meter=%d order by timestamp desc", id);

    return getSingleValue (query);
}

struct valueCollectCtx {
    database::valueMap& map;
    size_t numOfIDs;
    bool found;
};

int volumeCollectCb (void *param, int numOfFields, char **values, char **fields) {
    valueCollectCtx *context = (valueCollectCtx *) param;

    if (numOfFields > 1 && values [0] && values [1]) {
        uint32_t tank = atol (values [0]);
        double level = atof (values [1]);
        time_t timestamp = _atoi64 (values [2]);

        auto pos = context->map.find (tank);

        if (pos == context->map.end ()) {
            context->map.emplace (tank, database::timedValue (level, timestamp));

            if (context->map.size () >= context->numOfIDs) return 1;
        }

        context->found = true;
    }

    return 0;
}

void database::collectCurrentVolumes (valueMap& volumes) {
    valueCollectCtx context { volumes, cfg.tanks.size (), false };

    executeAndGet ("select tank,value,stimestamp from volumes order by timestamp desc", volumeCollectCb, & context, 0);
}

void database::collectCurrentMeters (valueMap& volumes) {
    valueCollectCtx context { volumes, cfg.tanks.size (), false };

    executeAndGet ("select meter,value,timestamp from meters order by timestamp desc", volumeCollectCb, & context, 0);
}

uint64_t database::addLogbookRecord (
    time_t timestamp,
    double *lat,
    double *lon,
    float *cog,
    float *sog,
    float *hdg
) {
    char query [200];
    char buffer [100];
    static const char *null = "null";
    sprintf (
        query,
        "insert into logbook(timestamp,lat,lon,sog,cog,hdg) values(%I64d,%s,%s,%s,%s,%s)",
        timestamp,
        lat && lon ? ftoa (*lat, buffer, "%.8f") : null,
        lat && lon ? ftoa (*lon, buffer + 20, "%.8f") : null,
        sog ? ftoa (*sog, buffer + 40, "%.1f") : null,
        cog ? ftoa (*cog, buffer + 60, "%.1f") : null,
        hdg ? ftoa (*hdg, buffer + 80, "%.1f") : null
    );

    uint64_t result;
    executeSimple (query, & result);

    return result;
}

int logbookRecPickCb (void *param, int numOfFields, char **values, char **fields) {
    logbookRecord *rec = (logbookRecord *) param;

    auto assignDblField = [] (std::pair<double, bool>& data, char *stringVal) {
        if (stringVal) {
            data.first = atof (stringVal);
            data.second = true;
        } else {
            data.second = false;
        }
    };
    auto assignFltField = [] (std::pair<float, bool>& data, char *stringVal) {
        if (stringVal) {
            data.first = (float) atof (stringVal);
            data.second = true;
        } else {
            data.second = false;
        }
    };

    rec->timestamp = _atoi64 (values [1]);

    assignDblField (rec->lat, values [2]);
    assignDblField (rec->lon, values [3]);
    assignFltField (rec->sog, values [4]);
    assignFltField (rec->cog, values [5]);
    assignFltField (rec->hdg, values [6]);

    return 0;
}

void database::getRecentLogbookRecord (logbookRecord& rec) {
    executeAndGet ("select * from logbook order by timestamp desc limit 1", logbookRecPickCb, & rec, 0);
}

bool database::loadTankStatesAt (time_t begin, time_t end, std::vector<tankState>& states, float& counterBefore, float& counterAfter) {
    valueMap volumesAtBegin, metersAtBegin, volumesAtEnd, metersAtEnd;
    valueCollectCtx volumesAtBeginCtx { volumesAtBegin, cfg.tanks.size (), false };
    valueCollectCtx metersAtBeginCtx { metersAtBegin, cfg.fuelMeters.size (), false };
    valueCollectCtx volumesAtEndCtx { volumesAtEnd, cfg.tanks.size (), false };
    valueCollectCtx metersAtEndCtx { metersAtEnd, cfg.fuelMeters.size (), false };

    auto getStateAt = [this, &states, &counterBefore, &counterAfter] (time_t timestamp, valueCollectCtx *volumeCtx, valueCollectCtx *meterCtx, bool isBegin) {
        char query [200];
        sprintf (query, "select tank,value,timestamp from volumes where timestamp<=%I64d order by timestamp desc", timestamp);
        executeAndGet (query, volumeCollectCb, volumeCtx, 0);
        sprintf (query, "select meter,value,timestamp from meters where timestamp<=%I64d order by timestamp desc", timestamp);
        executeAndGet (query, volumeCollectCb, meterCtx, 0);

        auto uploadingMeter = cfg.findUploadingMeter ();

        for (auto& meter: meterCtx->map) {
            if (meter.first == uploadingMeter->id) {
                if (isBegin) {
                    counterBefore = meter.second.value;
                } else {
                    counterAfter = meter.second.value;
                }
                break;
            }
        }

        for (auto& volume: volumeCtx->map) {
            for (auto& state: states) {
                if (volume.first == state.tank) {
                    if (isBegin) {
                        state.before.volume.byVolume = volume.second.value;
                        state.before.volume.byCounter = counterBefore;
                    } else {
                        state.after.volume.byVolume = volume.second.value;
                        state.after.volume.byCounter = counterAfter;
                    }
                    break;
                }
            }
        }
    };

    getStateAt (begin, & volumesAtBeginCtx, & metersAtBeginCtx, true);
    getStateAt (end, & volumesAtEndCtx, & metersAtEndCtx, false);

    return volumesAtBeginCtx.found && metersAtBeginCtx.found && volumesAtEndCtx.found && metersAtEndCtx.found;
}

