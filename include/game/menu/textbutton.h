#ifndef _TEXTBUTTON_H
#define _TEXTBUTTON_H

#include "game/menu/component.h"
#include "game/text/text.h"

void textbutton_create(component *c, font *font, const char *text);
void textbutton_free(component *c);
void textbutton_render(component *c);
int textbutton_event(component *c, SDL_Event *event);
int textbutton_action(component *c, int action);
void textbutton_tick(component *c);
void textbutton_set_border(component *c, color col);
void textbutton_remove_border(component *c);

#endif // _TEXTBUTTON_H
