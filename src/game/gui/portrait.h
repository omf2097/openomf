#ifndef PORTRAIT_H
#define PORTRAIT_H

#include "formats/sprite.h"
#include "game/gui/component.h"

component *portrait_create(int pic_id, int pilot_id);
void portrait_select(component *c, int pic_id, int pilot_id);
int portrait_get_pilot_count(component *c, int pic_id);
int portrait_load(sd_sprite *s, vga_palette *pal, int pic_id, int pilot_id);
void portrait_next(component *c);
void portrait_prev(component *c);
int portrait_selected(component *c);
void portrait_set_from_sprite(component *c, sd_sprite *spr);

#endif // PORTRAIT_H
