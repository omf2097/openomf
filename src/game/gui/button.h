#ifndef BUTTON_H
#define BUTTON_H

#include "game/gui/component.h"
#include "game/gui/text_render.h"

typedef void (*button_click_cb)(component *c, void *userdata);

component *button_create(const text_settings *tconf, const char *text, const char *help, int disabled,
                         button_click_cb cb, void *userdata);
void button_set_border(component *c, vga_index border_color);
void button_set_text(component *c, const char *text);
void button_set_userdata(component *c, void *userdata);

#endif // BUTTON_H
