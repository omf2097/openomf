#ifndef TICKTIMER_H
#define TICKTIMER_H

#include "utils/vector.h"

typedef struct ticktimer_t {
    vector units;
} ticktimer;

typedef void (*ticktimer_cb)(void *scenedata, void *userdata);

void ticktimer_init(ticktimer *tt);
void ticktimer_add(ticktimer *tt, int ticks, ticktimer_cb cb, void *userdata);
void ticktimer_run(ticktimer *tt, void *scenedata);
void ticktimer_close(ticktimer *tt);

#endif // TICKTIMER_H
