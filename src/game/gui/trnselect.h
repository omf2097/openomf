#ifndef TRNSELECT_H
#define TRNSELECT_H

#include "formats/tournament.h"
#include "game/gui/component.h"

component *trnselect_create(void);
int trnselect_get_pilot_count(component *c, int pic_id);
void trnselect_next(component *c);
void trnselect_prev(component *c);
sd_tournament_file *trnselect_selected(component *c);

#endif // TRNSELECT_H
