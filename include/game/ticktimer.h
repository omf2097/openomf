#ifndef _TICKTIMER_H
#define _TICKTIMER_H

typedef void (*ticktimer_cb)(void *userdata);

void ticktimer_init();
void ticktimer_add(int ticks, ticktimer_cb cb, void *userdata);
void ticktimer_run();
void ticktimer_close();

#endif // _TICKTIMER_H