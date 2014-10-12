#ifndef _LABEL_H
#define _LABEL_H

#include "game/gui/component.h"
#include "game/gui/text_render.h"

component* label_create(const font *font, const char *text);
void label_set_text(component *label, const char *text);

#endif // _LABEL_H
