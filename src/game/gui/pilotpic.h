#ifndef PILOTPIC_H
#define PILOTPIC_H

#include "game/gui/component.h"

component *pilotpic_create(int pic_id, int pilot_id);
void pilotpic_select(component *c, int pic_id, int pilot_id);
int pilotpic_get_pilot_count(component *c, int pic_id);
void pilotpic_next(component *c);
void pilotpic_prev(component *c);

#endif // PILOTPIC_H
