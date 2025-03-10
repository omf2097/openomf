#include "game/gui/label.h"
#include "game/gui/widget.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"

typedef struct label {
    char *text;
    text_settings tconf;
} label;

static void label_render(component *c) {
    label *local = widget_get_obj(c);
    const gui_theme *theme = component_get_theme(c);
    text_render(&local->tconf, theme->text.primary_color, c->x, c->y, c->w, c->h, local->text);
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
    local->text = omf_strdup(text);
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
    int h = text_find_line_count(tconf, max_width / tsize, 0, strlen(text), text, &longest);

    // fonts are all 8 high?
    component_set_size_hints(c, longest * tsize, h * 8);

    widget_set_obj(c, local);
    widget_set_render_cb(c, label_render);
    widget_set_free_cb(c, label_free);

    return c;
}

component *label_create(const text_settings *tconf, const char *text) {
    return label_create_with_width(tconf, text, 300);
}
