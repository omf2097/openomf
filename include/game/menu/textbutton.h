#ifndef _TEXTBUTTON_H
#define _TEXTBUTTON_H

#include "game/menu/component.h"
#include "game/text/text.h"

typedef void (*textbutton_click_cb)(component *c, void *userdata);

component* textbutton_create(font *font, const char *text, int disabled, textbutton_click_cb cb, void *userdata);
void textbutton_set_border(component *c, color col);
void textbutton_set_text(component *c, const char* text);
void textbutton_remove_border(component *c);

#endif // _TEXTBUTTON_H
