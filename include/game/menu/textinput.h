#ifndef _TEXTINPUT_H
#define _TEXTINPUT_H

#include "game/menu/component.h"
#include "game/text/text.h"

component* textinput_create(font *font, const char *text, const char *initialoption);
void textinput_free(component *c);
void textinput_render(component *c);
int textinput_event(component *c, SDL_Event *event);
void textinput_tick(component *c);
char* textinput_value(component *c);

#endif // _TEXTINPUT_H
