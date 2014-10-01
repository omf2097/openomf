#ifndef _LABEL_H
#define _LABEL_H

#include "game/menu/component.h"
#include "game/text/text.h"

component* label_create(const font *font, const char *text);
void label_set_text(component *label, const char *text);

#endif // _LABEL_H
