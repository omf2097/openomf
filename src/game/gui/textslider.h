#ifndef TEXTSLIDER_H
#define TEXTSLIDER_H

#include "game/gui/component.h"
#include "game/gui/text/enums.h"

typedef void (*textslider_slide_cb)(component *c, void *userdata, int pos);

component *textslider_create(const char *text, const char *help, unsigned int positions, int has_off,
                             textslider_slide_cb cb, void *userdata);
component *textslider_create_bind(const char *text, const char *help, unsigned int positions, int has_off,
                                  textslider_slide_cb cb, void *userdata, int *bind);
// by default, the "next" and "previous" item selection noises are panned; call this to disable that effect.
void textslider_disable_panning(component *c);

void textslider_set_font(component *c, font_size font);
void textslider_set_text_horizontal_align(component *c, text_horizontal_align align);
void textslider_set_text_vertical_align(component *c, text_vertical_align align);

#endif // TEXTSLIDER_H
