#ifndef _TEXTSLIDER_H
#define _TEXTSLIDER_H

#include "game/menu/component.h"
#include "game/text/text.h"

typedef void (*textslider_slide_cb)(component *c, void *userdata, int pos);

component* textslider_create(font *font, const char *text, unsigned int positions, int has_off, textslider_slide_cb cb, void *userdata);
component* textslider_create_bind(font *font, const char *text, unsigned int positions, int has_off, textslider_slide_cb cb, void *userdata, int *bind);

#endif // _TEXTSLIDER_H
