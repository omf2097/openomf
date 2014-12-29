#ifndef _LABEL_H
#define _LABEL_H

#include "game/gui/component.h"
#include "game/gui/text_render.h"

component* label_create(const text_settings *tconf, const char *text);
void label_set_text(component *label, const char *text);
text_settings* label_get_text_settings(component *c);

#endif // _LABEL_H
