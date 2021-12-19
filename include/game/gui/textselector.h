#ifndef TEXTSELECTOR_H
#define TEXTSELECTOR_H

#include "game/gui/component.h"
#include "game/gui/text_render.h"
#include "utils/vector.h"

typedef void (*textselector_toggle_cb)(component *c, void *userdata, int pos);

component* textselector_create(const text_settings *tconf, const char *text, textselector_toggle_cb toggle_cb, void *userdata);
component* textselector_create_bind(const text_settings *tconf, const char *text, textselector_toggle_cb toggle_cb, void *userdata, int *bind);
component* textselector_create_bind_opts(const text_settings *tconf, const char *text, textselector_toggle_cb toggle_cb, void *userdata, int *bind, const char **opts, int opt_size);
void textselector_add_option(component *c, const char *option);
void textselector_clear_options(component *c);
const char* textselector_get_current_text(const component *c);
int textselector_get_pos(const component *c);
void textselector_set_pos(component *c, int pos);

#endif // TEXTSELECTOR_H
