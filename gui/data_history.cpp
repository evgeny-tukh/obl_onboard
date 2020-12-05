#include "data_history.h"

dataHistory::dataHistory (database& _db, config& _cfg): db (_db), cfg (_cfg), lastMax (0) {
    for (auto& tank: cfg.tanks) {
        histories.emplace (tank.id, dataHistory::history ());
    }
    for (auto& fm: cfg.fuelMeters) {
        histories.emplace (fm.id, dataHistory::history ());
    }

    load ();
}

int dataHistory::loadCb (void *instance, int numOfFields, char **values, char **fields) {
    return ((dataHistory *) instance)->loadCbInt (numOfFields, fields, values);
}

int dataHistory::loadCbInt (int numOfFields, char **fields, char **values) {
    auto object = atoi (values [0]);
    auto history = histories.find (object);

    if (history != histories.end ()) {
        time_t timestamp = _atoi64 (values [1]);

        // remember last processed time
        if (timestamp > lastMax) lastMax = timestamp;

        history->second.emplace (timestamp, (float) atof (values [2]));
    }

    return 0;
}

void dataHistory::load () {
    char *errorMsg, query [200];

    // cleanup first
    //for (auto& history: histories) history.second.clear ();

    // now load data
    auto max = lastMax;

    sprintf (query, "select tank,timestamp,value from volumes where timestamp>%I64d", max);
    db.executeAndGet (query, loadCb, this, 0);

    sprintf (query, "select meter,timestamp,value from meters where timestamp>%I64d", max);
    db.executeAndGet (query, loadCb, this, 0);
}

float dataHistory::getData (uint16_t id, time_t timestamp) {
    auto& history = histories.find (id);

    if (history == histories.end ()) return 0.0;

    for (auto state = history->second.rbegin (); state != history->second.rend (); ++ state) {
        if (state->first < timestamp) return state->second;
    }
    
    return 0.0f;
    /*auto& last = history->second.rbegin ();

    return last->second;*/
}

time_t dataHistory::minTime () {
    time_t min = time (0);

    for (auto& history: histories) {
        auto& first = history.second.begin ();

        if (first != history.second.end () && first->first < min) min = first->first;
    }

    return min;
}

time_t dataHistory::maxTime () {
    time_t max =0;

    for (auto& history: histories) {
        auto& first = history.second.rbegin ();

        if (first != history.second.rend () && first->first > max) max = first->first;
    }

    return max;
}
