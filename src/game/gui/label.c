#include "game/gui/label.h"
#include "game/gui/widget.h"
#include "utils/allocator.h"
#include "utils/compat.h"

typedef struct {
    char *text;
    text_settings tconf;
} label;

static void label_render(component *c) {
    label *local = widget_get_obj(c);
    text_render(&local->tconf, TEXT_DEFAULT, c->x, c->y, c->w, c->h, local->text);
}

static void label_free(component *c) {
    label *local = widget_get_obj(c);
    omf_free(local->text);
    omf_free(local);
}

void label_set_text(component *c, const char *text) {
    label *local = widget_get_obj(c);
    if(local->text) {
        omf_free(local->text);
    }
    local->text = strdup(text);
}

text_settings *label_get_text_settings(component *c) {
    label *local = widget_get_obj(c);
    return &local->tconf;
}

component *label_create_with_width(const text_settings *tconf, const char *text, int max_width) {
    component *c = widget_create();
    component_disable(c, 1);
    c->supports_disable = 1;
    c->supports_select = 0;
    c->supports_focus = 0;

    label *local = omf_calloc(1, sizeof(label));
    memcpy(&local->tconf, tconf, sizeof(text_settings));
    local->text = strdup(text);

    int tsize = text_char_width(tconf);
    int w, h;
    w = text_find_max_strlen(tconf, max_width / tsize, text);
    h = text_find_line_count(tconf, w, 0, strlen(text), text) * 8; // fonts are all 8 high?

    component_set_size_hints(c, w, h);

    // local->tconf.cforeground = color_create(0, 255, 0, 255);

    widget_set_obj(c, local);
    widget_set_render_cb(c, label_render);
    widget_set_free_cb(c, label_free);

    return c;
}

component *label_create(const text_settings *tconf, const char *text) {
    return label_create_with_width(tconf, text, 300);
}
