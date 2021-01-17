#include <stdlib.h>
#include <stdio.h>
#include "db.h"
#include "tools.h"

const char *database::dbPath = "data.db";

void log (char *msg);

char *initialQueries [] {
    "create table logbook"
    "(id integer not null primary key asc,"
    "timestamp integer not null,"
    "lat real,lon real,"
    "sog real,cog real,hdg real,"
    "draft_fore real,draft_aft real)",
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
    "create unique index idx_vol1 on volumes(tank,timestamp)",
    "create unique index idx_vol2 on volumes(timestamp,tank)",

    "create table meters(id integer not null primary key asc,meter integer not null,timestamp integer not null,value real not null)",
    "create unique index idx_mtr1 on meters(meter,timestamp)",
    "create unique index idx_mtr2 on meters(timestamp,meter)",

    "create table bunkerings("
    "id integer not null primary key asc,"
    "begin integer not null,"
    "end integer not null,"
    "port text,"
    "barge text,"
    "draft_fore_before real,"
    "draft_aft_before real,"
    "draft_fore_after real,"
    "draft_aft_after real,"
    "density_rep real not null,"
    "density_by_vol real not null,"
    "density_by_cnt real not null,"
    "viscosity real not null,"
    "sulphur real not null,"
    "temp_rep real not null,"
    "temp_by_vol real not null,"
    "temp_by_cnt real not null,"
    "volume_rep real not null,"
    "volume_by_vol real not null,"
    "volume_by_cnt real not null,"
    "quantity_rep real not null,"
    "quantity_by_vol real not null,"
    "quantity_by_cnt real not null,"
    "vcf_rep real not null,"
    "vcf_by_vol real not null,"
    "vcf_by_cnt real not null)",
    "create index idx_bunk1 on bunkerings(begin)",
    "create index idx_bunk2 on bunkerings(end)",

    "create table tank_state("
    "id integer not null primary key asc,"
    "bunkering integer not null,"
    "tank integer not null,"
    "density_before_rep real not null,"
    "density_before_by_vol real not null,"
    "density_before_by_cnt real not null,"
    "viscosity_before real not null,"
    "sulphur_before real not null,"
    "temp_before_rep real not null,"
    "temp_before_by_vol real not null,"
    "temp_before_by_cnt real not null,"
    "volume_before_rep real not null,"
    "volume_before_by_vol real not null,"
    "volume_before_by_cnt real not null,"
    "quantity_before_rep real not null,"
    "quantity_before_by_vol real not null,"
    "quantity_before_by_cnt real not null,"
    "vcf_before_rep real not null,"
    "vcf_before_by_vol real not null,"
    "vcf_before_by_cnt real not null,"
    "density_after_rep real not null,"
    "density_after_by_vol real not null,"
    "density_after_by_cnt real not null,"
    "viscosity_after real not null,"
    "sulphur_after real not null,"
    "temp_after_rep real not null,"
    "temp_after_by_vol real not null,"
    "temp_after_by_cnt real not null,"
    "volume_after_rep real not null,"
    "volume_after_by_vol real not null,"
    "volume_after_by_cnt real not null,"
    "quantity_after_rep real not null,"
    "quantity_after_by_vol real not null,"
    "quantity_after_by_cnt real not null,"
    "vcf_after_rep real not null,"
    "vcf_after_by_vol real not null,"
    "vcf_after_by_cnt real not null)",
    "create unique index idx_tankstate1 on tank_state(bunkering,tank)",
    "create unique index idx_tankstate2 on tank_state(tank,bunkering)",

    0,
};

void logString (char *string) {
    FILE *log = fopen ("data.log", "rb+");

    if (!log) log = fopen ("data.log", "wb+");

    if (log) {
        fseek (log, 0, SEEK_END);
        fwrite (string, 1, strlen (string), log);
        fwrite ("\r\n", 1, 2, log);
        fclose (log);
    }
}

database::database (config& _cfg, char *workingFolder): cfg (_cfg) {
    std::string dbFilePath (workingFolder ? workingFolder : "");

    dbFilePath += dbPath;

    if (sqlite3_open_v2 (dbFilePath.c_str (), & db, SQLITE_OPEN_READWRITE, 0) == SQLITE_OK) {
        return;
    } else if (sqlite3_open (dbFilePath.c_str (), & db) == SQLITE_OK) {
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
    char query [2000];

    sprintf (
        query, 
        "update bunkerings set "
        "begin=%I64d,end=%I64d,port='%s',barge='%s',"
        "draft_fore_before=%.4f,draft_aft_before=%.4f,"
        "draft_fore_after=%.4f,draft_aft_after=%.4f,"
        "density_rep=%.4f,density_by_vol=%.4f,density_by_cnt=%.4f,"
        "viscosity=%.4f,sulphur=%.4f,"
        "temp_rep=%.4f,temp_by_vol=%.4f,temp_by_cnt=%.4f,"
        "volume_rep=%.4f,volume_by_vol=%.4f,volume_by_cnt=%.4f,"
        "quantity_rep=%.4f,quantity_by_vol=%.4f,quantity_by_cnt=%.4f,vcf_rep=%.4f,vcf_by_vol=%.4f,vcf_by_cnt=%.4f "
        "where id=%d",
        data.begin, data.end, data.port.c_str (), data.barge.c_str (),
        data.draftBefore.fore, data.draftBefore.aft, data.draftAfter.fore, data.draftAfter.aft,
        data.loaded.density.reported, data.loaded.density.byVolume, data.loaded.density.byCounter,
        data.loaded.viscosity, data.loaded.sulphur, 
        data.loaded.temp.reported, data.loaded.temp.byVolume, data.loaded.temp.byCounter,
        data.loaded.volume.reported, data.loaded.volume.byVolume, data.loaded.volume.byCounter,
        data.loaded.quantity.reported, data.loaded.quantity.byVolume, data.loaded.quantity.byCounter,
        data.loaded.vcf.reported, data.loaded.vcf.byVolume, data.loaded.vcf.byCounter,
        data.id
    );
    executeSimple (query);

    for (auto& tankState: data.tankStates) {
        sprintf (
            query, 
            "update tank_state "
            "set density_before_rep=%.4f,density_before_by_vol=%.4f,density_before_by_cnt=%.4f,"
            "viscosity_before=%.4f,sulphur_before=%.4f,"
            "temp_before_rep=%.4f,temp_before_by_vol=%.4f,temp_before_by_cnt=%.4f,"
            "volume_before_rep=%.4f,volume_before_by_vol=%.4f,volume_before_by_cnt=%.4f,"
            "quantity_before_rep=%.4f,quantity_before_by_vol=%.4f,quantity_before_by_cnt=%.4f,"
            "vcf_before_rep=%.4f,vcf_before_by_vol=%.4f,vcf_before_by_cnt=%.4f,"
            "density_after_rep=%.4f,density_after_by_vol=%.4f,density_after_by_cnt=%.4f,"
            "viscosity_after=%.4f,sulphur_after=%.4f,"
            "temp_after_rep=%.4f,temp_after_by_vol=%.4f,temp_after_by_cnt=%.4f,"
            "volume_after_rep=%.4f,volume_after_by_vol=%.4f,volume_after_by_cnt=%.4f,"
            "quantity_after_rep=%.4f,quantity_after_by_vol=%.4f,quantity_after_by_cnt=%.4f,"
            "vcf_after_rep=%.4f,vcf_after_by_vol=%.4f,vcf_after_by_cnt=%.4f "
            "where id=%d and bunkering=%d",
            tankState.before.density.reported, tankState.before.density.byVolume, tankState.before.density.byCounter,
            tankState.before.viscosity, tankState.before.sulphur, 
            tankState.before.temp.reported, tankState.before.temp.byVolume, tankState.before.temp.byCounter,
            tankState.before.volume.reported, tankState.before.volume.byVolume, tankState.before.volume.byCounter,
            tankState.before.quantity.reported, tankState.before.quantity.byVolume, tankState.before.quantity.byCounter,
            tankState.before.vcf.reported, tankState.before.vcf.byVolume, tankState.before.vcf.byCounter,
            tankState.after.density.reported, tankState.after.density.byVolume, tankState.after.density.byCounter,
            tankState.after.viscosity, tankState.after.sulphur, 
            tankState.after.temp.reported, tankState.after.temp.byVolume, tankState.after.temp.byCounter,
            tankState.after.volume.reported, tankState.after.volume.byVolume, tankState.after.volume.byCounter,
            tankState.after.quantity.reported, tankState.after.quantity.byVolume, tankState.after.quantity.byCounter,
            tankState.after.vcf.reported, tankState.after.vcf.byVolume, tankState.after.vcf.byCounter,
            tankState.id, data.id
        );
        executeSimple (query);
    }
}

uint64_t database::createBunkering (bunkeringData& data) {
    char query [3000];
    uint64_t bunkeringID, stateID;

    memset (query, 0, sizeof (query));

    sprintf (
        query, 
        "insert into bunkerings"
        "(begin,end,port,barge,"
        "draft_fore_before,draft_aft_before,draft_fore_after,draft_aft_after,"
        "density_rep,density_by_vol,density_by_cnt,"
        "viscosity,sulphur,"
        "temp_rep,temp_by_vol,temp_by_cnt,"
        "volume_rep,volume_by_vol,volume_by_cnt,quantity_rep,quantity_by_vol,quantity_by_cnt,"
        "vcf_rep,vcf_by_vol,vcf_by_cnt) "
        "values(%I64d,%I64d,'%s','%s',%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f)",
        data.begin, data.end, data.port.c_str (), data.barge.c_str (),
        data.draftBefore.fore, data.draftBefore.aft, data.draftAfter.fore, data.draftAfter.aft,
        data.loaded.density.reported, data.loaded.density.byVolume, data.loaded.density.byCounter,
        data.loaded.viscosity, data.loaded.sulphur,
        data.loaded.temp.reported, data.loaded.temp.byVolume, data.loaded.temp.byCounter,
        data.loaded.volume.reported, data.loaded.volume.byVolume, data.loaded.volume.byCounter,
        data.loaded.quantity.reported, data.loaded.quantity.byVolume, data.loaded.quantity.byCounter,
        data.loaded.vcf.reported, data.loaded.vcf.byVolume, data.loaded.vcf.byCounter
    );
    executeSimple (query, & bunkeringID);

    data.id = bunkering;

    for (auto& tankState: data.tankStates) {
        memset (query, 0, sizeof (query));
        sprintf (
            query, 
            "insert into tank_state"
            "(bunkering,tank,"
            "density_before_rep,density_before_by_vol,density_before_by_cnt,"
            "viscosity_before,sulphur_before,"
            "temp_before_rep,temp_before_by_vol,temp_before_by_cnt,"
            "volume_before_rep,volume_before_by_vol,volume_before_by_cnt,"
            "quantity_before_rep,quantity_before_by_vol,quantity_before_by_cnt,"
            "vcf_before_rep,vcf_before_by_vol,vcf_before_by_cnt,"
            "density_after_rep,density_after_by_vol,density_after_by_cnt,"
            "viscosity_after,sulphur_after,"
            "temp_after_rep,temp_after_by_vol,temp_after_by_cnt,"
            "volume_after_rep,volume_after_by_vol,volume_after_by_cnt,"
            "quantity_after_rep,quantity_after_by_vol,quantity_after_by_cnt,"
            "vcf_after_rep,vcf_after_by_vol,vcf_after_by_cnt) "
            "values(%I64d,%d,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,"
            "%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,"
            "%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f)",
            bunkeringID, tankState.tank,
            tankState.before.density.reported, tankState.before.density.byVolume, tankState.before.density.byCounter, 
            tankState.before.viscosity, tankState.before.sulphur, 
            tankState.before.temp.reported, tankState.before.temp.byVolume, tankState.before.temp.byCounter,
            tankState.before.volume.reported, tankState.before.volume.byVolume, tankState.before.volume.byCounter,
            tankState.before.quantity.reported, tankState.before.quantity.byVolume, tankState.before.quantity.byCounter,
            tankState.before.vcf.reported, tankState.before.vcf.byVolume, tankState.before.vcf.byCounter,
            tankState.after.density.reported, tankState.after.density.byVolume, tankState.after.density.byCounter,
            tankState.after.viscosity, tankState.after.sulphur, 
            tankState.after.temp.reported, tankState.after.temp.byVolume, tankState.after.temp.byCounter,
            tankState.after.volume.reported, tankState.after.volume.byVolume, tankState.after.volume.byCounter,
            tankState.after.quantity.reported, tankState.after.quantity.byVolume, tankState.after.quantity.byCounter,
            tankState.after.vcf.reported, tankState.after.vcf.byVolume, tankState.after.vcf.byCounter
        );
        logString (query);
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
        state.density.reported = atoF (values [start]);
        state.density.byVolume = atoF (values [start+1]);
        state.density.byCounter = atoF (values [start+2]);
        state.viscosity = atoF (values [start+3]);
        state.sulphur = atoF (values [start+4]);
        state.temp.reported = atoF (values [start+5]);
        state.temp.byVolume = atoF (values [start+6]);
        state.temp.byCounter = atoF (values [start+7]);
        state.volume.reported = atoF (values [start+8]);
        state.volume.byVolume = atoF (values [start+9]);
        state.volume.byCounter = atoF (values [start+10]);
        state.quantity.reported = atoF (values [start+11]);
        state.quantity.byVolume = atoF (values [start+12]);
        state.quantity.byCounter = atoF (values [start+13]);
        state.vcf.reported = atoF (values [start+14]);
        state.vcf.byVolume = atoF (values [start+15]);
        state.vcf.byCounter = atoF (values [start+16]);
    };

    uint32_t bunkeringID = atoi (values [0]);

    if (lastBunkeringID != bunkeringID) {
        ctx->list.emplace_back (ctx->cfg, bunkeringID, values [3], values [4]);
        bunkData = & ctx->list.back ();

        bunkData->begin = atol (values [1]);
        bunkData->end = atol (values [2]);
        bunkData->draftBefore.fore = atoF (values [5]);
        bunkData->draftBefore.aft = atoF (values [6]);
        bunkData->draftAfter.fore = atoF (values [7]);
        bunkData->draftAfter.aft = atoF (values [8]);
        
        loadFuelState (bunkData->loaded, 9);
        bunkData->tankStates.clear ();
    }

    bunkData->tankStates.emplace_back (atol (values [28]), atol (values [26]));

    tankState& tank = bunkData->tankStates.back ();

    loadFuelState (tank.before, 29);
    loadFuelState (tank.after, 46);

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

    executeAndGet ("select tank,value,timestamp from volumes order by timestamp desc", volumeCollectCb, & context, 0);
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
    float *hdg,
    float *draftFore,
    float *draftAft
) {
    char query [200];
    char buffer [100];
    static const char *null = "null";
    sprintf (
        query,
        "insert into logbook(timestamp,lat,lon,sog,cog,hdg,draft_fore,draft_aft) values(%I64d,%s,%s,%s,%s,%s,%s,%s)",
        timestamp,
        lat && lon ? ftoa (*lat, buffer, "%.8f") : null,
        lat && lon ? ftoa (*lon, buffer + 20, "%.8f") : null,
        sog ? ftoa (*sog, buffer + 40, "%.1f") : null,
        cog ? ftoa (*cog, buffer + 60, "%.1f") : null,
        hdg ? ftoa (*hdg, buffer + 80, "%.1f") : null,
        draftFore ? ftoa (*draftFore, buffer + 80, "%.1f") : null,
        draftAft ? ftoa (*draftAft, buffer + 80, "%.1f") : null
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
