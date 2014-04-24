#include <stdlib.h>
#include "game/utils/ticktimer.h"
#include "utils/vector.h"

typedef struct ticktimer_unit_t {
    ticktimer_cb callback;
    int ticks;
    void *userdata;
} ticktimer_unit;

void ticktimer_init(ticktimer *tt) {
    vector_create(&tt->units, sizeof(ticktimer_unit));
}

void ticktimer_close(ticktimer *tt) {
    vector_free(&tt->units);
}

void ticktimer_add(ticktimer *tt, int ticks, ticktimer_cb cb, void *userdata) {
    ticktimer_unit unit;
    unit.callback = cb;
    unit.ticks = ticks;
    unit.userdata = userdata;
    vector_append(&tt->units, &unit);
}

void ticktimer_run(ticktimer *tt) {
    iterator it;
    vector_iter_begin(&tt->units, &it);
    ticktimer_unit *unit;
    while((unit = iter_next(&it)) != NULL) {
        if(unit->ticks <= 0) {
            unit->callback(unit->userdata);
            vector_delete(&tt->units, &it);
        } else {
            unit->ticks--;
        }
    }
}
