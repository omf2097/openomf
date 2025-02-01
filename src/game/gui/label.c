#include "game/gui/label.h"
#include "game/gui/widget.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"

typedef struct {
    char *text;
    text_settings tconf;
    text_object text_cache[1];
} label;

static void label_render(component *c) {
    label *local = widget_get_obj(c);
    text_render(&(local->text_cache[0]), &local->tconf, TEXT_DEFAULT, c->x, c->y, c->w, c->h, local->text);
}

static void label_free(component *c) {
    label *local = widget_get_obj(c);
    text_objects_free(local->text_cache, 1);
    omf_free(local->text);
    omf_free(local);
}

void label_set_text(component *c, const char *text) {
    label *local = widget_get_obj(c);
    if(local->text) {
        omf_free(local->text);
    }
    local->text = omf_strdup(text);
    local->text_cache[0].dirty = true;
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
    local->text = omf_strdup(text);

    int tsize = text_char_width(tconf);
    int longest = 0;
    int h = text_find_line_count(tconf, max_width / tsize, 0, text, &longest);

    // fonts are all 8 high?
    component_set_size_hints(c, longest * tsize, h * 8);

    // local->tconf.cforeground = color_create(0, 255, 0, 255);

    widget_set_obj(c, local);
    widget_set_render_cb(c, label_render);
    widget_set_free_cb(c, label_free);
    // local->text_cache[0].dynamic = true;
    return c;
}

component *label_create(const text_settings *tconf, const char *text) {
    return label_create_with_width(tconf, text, 300);
}
