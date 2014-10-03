#include "game/gui/label.h"
#include "game/gui/widget.h"
#include "utils/compat.h"

typedef struct {
    char *text;
    const font *font;
} label;

static void label_render(component *c) {
    label *local = widget_get_obj(c);
    int chars = strlen(local->text);
    int width = chars * local->font->w;
    int xoff = (c->w - width) / 2;
    font_render(local->font, local->text, c->x + xoff, c->y, color_create(121, 121, 121, 255));
}

static void label_free(component *c) {
    label *local = widget_get_obj(c);
    free(local->text);
    free(local);
}

void label_set_text(component *c, const char* text) {
    label *local = widget_get_obj(c);
    if(local->text) {
        free(local->text);
    }
    local->text = strdup(text);
}

component* label_create(const font *font, const char *text) {
    component *c = widget_create();
    component_disable(c, 1);
    c->supports_disable = 0;
    c->supports_select = 0;
    c->supports_focus = 0;

    label *local = malloc(sizeof(label));
    memset(local, 0, sizeof(label));
    local->font = font;
    local->text = strdup(text);

    widget_set_obj(c, local);
    widget_set_render_cb(c, label_render);
    widget_set_free_cb(c, label_free);

    return c;
}
