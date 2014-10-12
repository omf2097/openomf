#ifndef _TEXTINPUT_H
#define _TEXTINPUT_H

#include "game/gui/component.h"
#include "game/gui/text_render.h"

component* textinput_create(const font *font, const char *text, const char *initialoption);
char* textinput_value(const component *c);

#endif // _TEXTINPUT_H
