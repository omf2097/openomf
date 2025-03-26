#ifndef BUTTON_H
#define BUTTON_H

#include "game/gui/component.h"
#include "game/gui/text_render.h"

typedef void (*button_click_cb)(component *c, void *userdata);

component *button_create(const char *text, const char *help, bool disabled, bool border, button_click_cb cb,
                         void *userdata);
void button_set_text(component *c, const char *text);
void button_set_userdata(component *c, void *userdata);
void button_set_text_shadow(component *c, uint8_t shadow, vga_index color);

#endif // BUTTON_H
