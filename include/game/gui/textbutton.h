#ifndef TEXTBUTTON_H
#define TEXTBUTTON_H

#include "game/gui/component.h"
#include "game/gui/text_render.h"

typedef void (*textbutton_click_cb)(component *c, void *userdata);

component *textbutton_create(const text_settings *tconf, const char *text, int disabled, textbutton_click_cb cb,
                             void *userdata);
void textbutton_set_border(component *c, color col);
void textbutton_set_text(component *c, const char *text);
void textbutton_remove_border(component *c);

#endif // TEXTBUTTON_H
