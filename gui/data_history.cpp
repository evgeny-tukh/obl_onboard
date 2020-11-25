#include "data_history.h"

dataHistory::dataHistory (database& _db, config& _cfg): db (_db), cfg (_cfg) {
    for (auto& tank: cfg.tanks) {
        histories.emplace (tank.id, dataHistory::history ());
    }
    for (auto& fm: cfg.fuelMeters) {
        histories.emplace (fm.id, dataHistory::history ());
    }

    load ();
}

int dataHistory::loadCb (void *instance, int numOfFields, char **values, char **fields) {
    return ((dataHistory *) instance)->loadCb (numOfFields, fields, values);
}

int dataHistory::loadCb (int numOfFields, char **fields, char **values) {
    auto object = atoi (values [0]);
    auto history = histories.find (object);

    if (history != histories.end ()) {
        history->second.emplace (_atoi64 (values [1]), (float) atof (values [2]));
    }

    return 0;
}

void dataHistory::load () {
    char *errorMsg;

    // cleanup first
    for (auto& history: histories) history.second.clear ();

    // now load data
    db.executeAndGet ("select tank,timestamp,value from volumes", loadCb, this, 0);
    db.executeAndGet ("select meter,timestamp,value from meters", loadCb, this, 0);
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
