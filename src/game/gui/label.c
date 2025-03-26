#include "game/gui/label.h"
#include "game/gui/text/text.h"
#include "game/gui/widget.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/log.h"
#include "video/video.h"

typedef struct label {
    text *text;
    int16_t override_color;
    font_size override_font;
    text_horizontal_align text_horizontal_align;
    text_vertical_align text_vertical_align;
    uint8_t letter_spacing;
    uint8_t text_shadow;
    text_margin text_margin;
    vga_index text_shadow_color;
    int color_theme;
} label;

static void label_render(component *c) {
    label *local = widget_get_obj(c);
    const gui_theme *theme = component_get_theme(c);
    vga_index color = theme->text.primary_color;
    if(local->override_color > -1) {
        color = local->override_color;
    } else if(local->color_theme == 1) {
        color = theme->text.secondary_color;
    }
    text_set_color(local->text, color);
    text_draw(local->text, c->x, c->y);
}

static void label_free(component *c) {
    label *local = widget_get_obj(c);
    text_free(&local->text);
    omf_free(local);
}

void label_set_text(component *c, const char *text) {
    label *local = widget_get_obj(c);
    text_set_from_c(local->text, text);
}

void label_set_text_color(component *c, vga_index color) {
    label *local = widget_get_obj(c);
    local->override_color = color;
}

void label_set_margin(component *c, text_margin margin) {
    label *local = widget_get_obj(c);
    local->text_margin = margin;
}

void label_set_font(component *c, font_size font) {
    label *local = widget_get_obj(c);
    local->override_font = font;
}

void label_set_text_horizontal_align(component *c, text_horizontal_align align) {
    label *local = widget_get_obj(c);
    local->text_horizontal_align = align;
}

void label_set_text_vertical_align(component *c, text_vertical_align align) {
    label *local = widget_get_obj(c);
    local->text_vertical_align = align;
}

void label_set_text_letter_spacing(component *c, uint8_t spacing) {
    label *local = widget_get_obj(c);
    local->letter_spacing = spacing;
}

void label_set_text_shadow(component *c, uint8_t shadow, vga_index color) {
    label *local = widget_get_obj(c);
    local->text_shadow = shadow;
    local->text_shadow_color = color;
}

void label_set_color_theme(component *c, int theme) {
    label *local = widget_get_obj(c);
    local->color_theme = theme;
}

static void label_init(component *c, const gui_theme *theme) {
    label *local = widget_get_obj(c);
    text_set_font(local->text, local->override_font != FONT_NONE ? local->override_font : theme->text.font);
    text_set_line_spacing(local->text, local->letter_spacing);
    text_set_horizontal_align(local->text, TEXT_ALIGN_LEFT);
    text_set_shadow_style(local->text, local->text_shadow);
    text_set_shadow_color(local->text, local->text_shadow_color);
    text_set_margin(local->text, local->text_margin);

    int bb_w = c->w_hint < 0 ? TEXT_BBOX_MAX : c->w_hint;
    int bb_h = c->h_hint < 0 ? TEXT_BBOX_MAX : c->h_hint;
    text_set_bounding_box(local->text, bb_w, bb_h);

    if(c->w_hint < 0) {
        text_generate_layout(local->text);
        int text_width = text_get_layout_width(local->text);
        component_set_size_hints(c, text_width + 6, c->h_hint);
    }
    if(c->h_hint < 0) {
        text_generate_layout(local->text);
        int text_height = text_get_layout_height(local->text);
        component_set_size_hints(c, c->w_hint, text_height + 3);
    }
}

static void label_layout(component *c, int x, int y, int w, int h) {
    label *local = widget_get_obj(c);
    text_set_bounding_box(local->text, w, h);
    text_set_horizontal_align(local->text, local->text_horizontal_align);
    text_set_vertical_align(local->text, local->text_vertical_align);
    text_generate_layout(local->text);
}

component *label_create_with_width(const char *text, uint16_t max_width) {
    component *c = widget_create();
    component_disable(c, true);
    component_set_supports(c, true, false, false);

    label *local = omf_calloc(1, sizeof(label));
    local->text = text_create_with_size(FONT_BIG, max_width, TEXT_BBOX_MAX);
    local->override_color = -1;
    local->color_theme = 0; // 0 = primary color, 1 = secondary color.
    local->override_font = FONT_NONE;
    local->text_horizontal_align = TEXT_ALIGN_LEFT;
    local->text_vertical_align = TEXT_ALIGN_TOP;
    local->letter_spacing = 0;
    local->text_shadow = GLYPH_SHADOW_NONE;
    local->text_shadow_color = 0;
    local->text_margin = (text_margin){0, 0, 0, 0};
    text_set_from_c(local->text, text);
    widget_set_obj(c, local);

    widget_set_render_cb(c, label_render);
    widget_set_free_cb(c, label_free);
    widget_set_init_cb(c, label_init);
    widget_set_layout_cb(c, label_layout);
    return c;
}

component *label_create(const char *text) {
    return label_create_with_width(text, TEXT_BBOX_MAX);
}

component *label_create_title(const char *text) {
    component *c = label_create_with_width(text, TEXT_BBOX_MAX);
    label_set_text_horizontal_align(c, TEXT_ALIGN_CENTER);
    label_set_color_theme(c, 1); // Secondary color
    return c;
}
