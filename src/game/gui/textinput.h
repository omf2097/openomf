#ifndef TEXTINPUT_H
#define TEXTINPUT_H

#include "game/gui/component.h"
#include "game/gui/text_render.h"

typedef void (*textinput_done_cb)(component *c, void *userdata);

component *textinput_create(const text_settings *tconf, int max_chars, const char *help, const char *initialvalue);
const char *textinput_value(const component *c);
void textinput_clear(component *c);
void textinput_enable_background(component *c, int enabled);
void textinput_set_done_cb(component *c, textinput_done_cb done_cb, void *userdata);

#endif // TEXTINPUT_H
