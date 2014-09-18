#ifndef _TEXTSELECTOR_H
#define _TEXTSELECTOR_H

#include "game/menu/component.h"
#include "game/text/text.h"
#include "utils/vector.h"

typedef void (*textselector_toggle_cb)(component *c, void *userdata, int pos);

component* textselector_create(font *font, const char *text, textselector_toggle_cb toggle_cb, void *userdata);
component* textselector_create_bind(font *font, const char *text, textselector_toggle_cb toggle_cb, void *userdata, int *bind);
component* textselector_create_bind_opts(font *font, const char *text, textselector_toggle_cb toggle_cb, void *userdata, int *bind, const char **opts, int opt_size);
void textselector_add_option(component *c, const char *option);
void textselector_clear_options(component *c);
const char* textselector_get_current_text(component *c);
int textselector_get_pos(component *c);
void textselector_set_pos(component *c, int pos);

#endif // _TEXTSELECTOR_H
