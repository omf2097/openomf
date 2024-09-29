#ifndef TEXTINPUT_H
#define TEXTINPUT_H

#include "game/gui/component.h"
#include "game/gui/text_render.h"

component *textinput_create(const text_settings *tconf, const char *text, const char *help, const char *initialoption);
char *textinput_value(const component *c);
void textinput_enable_background(component *c, int enabled);
void textinput_set_max_chars(component *c, int max_chars);

#endif // TEXTINPUT_H
