#ifndef PILOTPIC_H
#define PILOTPIC_H

#include "formats/sprite.h"
#include "game/gui/component.h"

component *pilotpic_create(int pic_id, int pilot_id);
void pilotpic_select(component *c, int pic_id, int pilot_id);
int pilotpic_get_pilot_count(component *c, int pic_id);
int pilotpic_load(sd_sprite *s, int pic_id, int pilot_id);
void pilotpic_next(component *c);
void pilotpic_prev(component *c);
int pilotpic_selected(component *c);

#endif // PILOTPIC_H
