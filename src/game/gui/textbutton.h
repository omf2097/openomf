#ifndef TEXTBUTTON_H
#define TEXTBUTTON_H

#include "game/gui/component.h"
#include "game/gui/text_render.h"

typedef void (*textbutton_click_cb)(component *c, void *userdata);

component *textbutton_create(const text_settings *tconf, const char *text, const char *help, int disabled,
                             textbutton_click_cb cb, void *userdata);
void textbutton_set_border(component *c, uint8_t color);
void textbutton_set_text(component *c, const char *text);

#endif // TEXTBUTTON_H
