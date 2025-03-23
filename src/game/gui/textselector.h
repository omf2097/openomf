#ifndef TEXTSELECTOR_H
#define TEXTSELECTOR_H

#include "game/gui/component.h"
#include "game/gui/text/enums.h"
#include "utils/vector.h"

typedef void (*textselector_toggle_cb)(component *c, void *userdata, int pos);

component *textselector_create(const char *text, const char *help, textselector_toggle_cb toggle_cb, void *userdata);
component *textselector_create_bind(const char *text, const char *help, textselector_toggle_cb toggle_cb,
                                    void *userdata, int *bind);
component *textselector_create_bind_opts(const char *text, const char *help, textselector_toggle_cb toggle_cb,
                                         void *userdata, int *bind, const char **opts, int opt_size);
void textselector_add_option(component *c, const char *option);
void textselector_clear_options(component *c);
const char *textselector_get_current_text(const component *c);
int textselector_get_pos(const component *c);
void textselector_set_pos(component *c, int pos);

void textselector_set_font(component *c, font_size font);
void textselector_set_text_horizontal_align(component *c, text_horizontal_align align);
void textselector_set_text_vertical_align(component *c, text_vertical_align align);

#endif // TEXTSELECTOR_H
