#ifndef _PILOTPIC_H
#define _PILOTPIC_H

#include "game/gui/component.h"

component* pilotpic_create(int pic_id, int pilot_id);
void pilotpic_select(component *c, int pic_id, int pilot_id);

#endif // _PILOTPIC_H
