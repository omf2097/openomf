#ifndef SPRITEBUTTON_H
#define SPRITEBUTTON_H

#include "game/gui/component.h"
#include "game/gui/text_render.h"
#include "video/surface.h"

typedef void (*spritebutton_click_cb)(component *c, void *userdata);
typedef void (*spritebutton_tick_cb)(component *c, void *userdata);
typedef void (*spritebutton_focus_cb)(component *c, bool focused, void *userdata);

component *spritebutton_create(const text_settings *tconf, const char *text, surface *img, int disabled,
                               spritebutton_click_cb cb, void *userdata);

void spritebutton_set_tick_cb(component *c, spritebutton_tick_cb);
void spritebutton_set_focus_cb(component *c, spritebutton_focus_cb);

void spritebutton_set_always_display(component *c);
void spritebutton_set_free_userdata(component *c, bool free_userdata);

#endif // SPRITEBUTTON_H
