#include <stdlib.h>
#include "game/ticktimer.h"
#include "utils/vector.h"

typedef struct ticktimer_unit_t {
    ticktimer_cb callback;
    int ticks;
    void *userdata;
} ticktimer_unit;

static vector _ticktimer_units;

void ticktimer_init() {
    vector_create(&_ticktimer_units, sizeof(ticktimer_unit));
}

void ticktimer_close() {
    vector_free(&_ticktimer_units);
}

void ticktimer_add(int ticks, ticktimer_cb cb, void *userdata) {
    ticktimer_unit unit;
    unit.callback = cb;
    unit.ticks = ticks;
    unit.userdata = userdata;
    vector_append(&_ticktimer_units, &unit);
}

void ticktimer_run() {
    iterator it;
    vector_iter_begin(&_ticktimer_units, &it);
    ticktimer_unit *unit;
    while((unit = iter_next(&it)) != NULL) {
        if(unit->ticks <= 0) {
            unit->callback(unit->userdata);
            vector_delete(&_ticktimer_units, &it);
        } else {
            unit->ticks--;
        }
    }
}
