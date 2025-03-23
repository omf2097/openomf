#ifndef TEXTINPUT_H
#define TEXTINPUT_H

#include "game/gui/component.h"
#include "game/gui/text/enums.h"

typedef void (*textinput_done_cb)(component *c, void *userdata);

component *textinput_create(int max_chars, const char *help, const char *initial_value);
const char *textinput_value(const component *c);
void textinput_clear(component *c);
void textinput_enable_background(component *c, int enabled);
void textinput_set_done_cb(component *c, textinput_done_cb done_cb, void *userdata);
void textinput_set_text(component *c, char const *value);

void textinput_set_font(component *c, font_size font);
void textinput_set_horizontal_align(component *c, text_horizontal_align align);
void textinput_set_text_shadow(component *c, uint8_t shadow, vga_index color);

#endif // TEXTINPUT_H
