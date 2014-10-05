#ifndef _SPRITEBUTTON_H
#define _SPRITEBUTTON_H

#include "game/gui/component.h"
#include "game/text/text.h"
#include "video/surface.h"

typedef void (*spritebutton_click_cb)(component *c, void *userdata);

component* spritebutton_create(const font *font, const char *text, const surface *img, int disabled, spritebutton_click_cb cb, void *userdata);

#endif // _SPRITEBUTTON_H
