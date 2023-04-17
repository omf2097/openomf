#ifndef TRNSELECT_H
#define TRNSELECT_H

#include "game/gui/component.h"

component *trnselect_create();
int trnselect_get_pilot_count(component *c, int pic_id);
void trnselect_next(component *c);
void trnselect_prev(component *c);

#endif // TRNSELECT_H
