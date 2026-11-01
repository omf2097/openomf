#include "game/gui/label.h"
#include "game/gui/component.h"
#include "game/gui/text/text.h"
#include "utils/allocator.h"

typedef struct label {
    text *text;
    int16_t override_color;
    font_size override_font;
    text_horizontal_align text_horizontal_align;
    text_vertical_align text_vertical_align;
    uint8_t line_spacing;
    uint8_t text_shadow;
    text_margin text_margin;
    vga_index text_shadow_color;
    int color_theme;
} label;

static void label_render(component *c) {
    const label *local = component_get_obj(c);
    const gui_theme *theme = component_get_theme(c);
    vga_index color = theme->text.primary_color;
    if(local->override_color > -1) {
        color = local->override_color;
    } else if(local->color_theme == 1) {
        color = theme->text.secondary_color;
    }
    text_set_color(local->text, color);
    const vec2i pos = component_get_pos(c);
    text_draw(local->text, pos.x, pos.y);
}

static void label_free(component *c) {
    label *local = component_get_obj(c);
    text_free(&local->text);
    omf_free(local);
}

static void label_measure(component *c) {
    const label *local = component_get_obj(c);
    uint16_t bb_w, bb_h;
    text_get_bounding_box(local->text, &bb_w, &bb_h);

    const vec2i hint = component_get_size_hint(c);
    text_set_bounding_box(local->text, hint.x < 0 ? TEXT_BBOX_MAX : hint.x, hint.y < 0 ? TEXT_BBOX_MAX : hint.y);
    text_generate_layout(local->text);
    component_set_required_size(c, text_get_layout_width(local->text) + 6, text_get_layout_height(local->text) + 3);

    text_set_bounding_box(local->text, bb_w, bb_h);
}

void label_set_text(component *c, const char *text) {
    const label *local = component_get_obj(c);
    text_set_from_c(local->text, text);
    label_measure(c);
}

void label_set_text_color(component *c, vga_index color) {
    label *local = component_get_obj(c);
    local->override_color = color;
}

void label_set_margin(component *c, text_margin margin) {
    label *local = component_get_obj(c);
    local->text_margin = margin;
}

void label_set_font(component *c, font_size font) {
    label *local = component_get_obj(c);
    local->override_font = font;
}

void label_set_text_horizontal_align(component *c, text_horizontal_align align) {
    label *local = component_get_obj(c);
    local->text_horizontal_align = align;
}

void label_set_text_vertical_align(component *c, text_vertical_align align) {
    label *local = component_get_obj(c);
    local->text_vertical_align = align;
}

void label_set_text_line_spacing(component *c, uint8_t spacing) {
    label *local = component_get_obj(c);
    local->line_spacing = spacing;
}

void label_set_text_shadow(component *c, uint8_t shadow, vga_index color) {
    label *local = component_get_obj(c);
    local->text_shadow = shadow;
    local->text_shadow_color = color;
}

void label_set_color_theme(component *c, int theme) {
    label *local = component_get_obj(c);
    local->color_theme = theme;
}

static void label_init(component *c, const gui_theme *theme) {
    const label *local = component_get_obj(c);
    text_set_font(local->text, local->override_font != FONT_NONE ? local->override_font : theme->text.font);
    text_set_line_spacing(local->text, local->line_spacing);
    text_set_horizontal_align(local->text, TEXT_ALIGN_LEFT);
    text_set_shadow_style(local->text, local->text_shadow);
    text_set_shadow_color(local->text, local->text_shadow_color);
    text_set_margin(local->text, local->text_margin);

    label_measure(c);
}

static void label_layout(component *c, int x, int y, int w, int h) {
    const label *local = component_get_obj(c);
    text_set_bounding_box(local->text, w, h);
    text_set_horizontal_align(local->text, local->text_horizontal_align);
    text_set_vertical_align(local->text, local->text_vertical_align);
    text_generate_layout(local->text);
}

component *label_create(const char *text) {
    component *c = component_create();
    component_disable(c, true);
    component_set_supported(c, true, false, false);

    label *local = omf_calloc(1, sizeof(label));
    local->text = text_create_with_font_and_size(FONT_BIG, TEXT_BBOX_MAX, TEXT_BBOX_MAX);
    local->override_color = -1;
    local->color_theme = 0; // 0 = primary color, 1 = secondary color.
    local->override_font = FONT_NONE;
    local->text_horizontal_align = TEXT_ALIGN_LEFT;
    local->text_vertical_align = TEXT_ALIGN_TOP;
    local->line_spacing = 0;
    local->text_shadow = GLYPH_SHADOW_NONE;
    local->text_shadow_color = 0;
    local->text_margin = (text_margin){0, 0, 0, 0};
    text_set_from_c(local->text, text);
    component_set_obj(c, local);

    component_set_render_cb(c, label_render);
    component_set_free_cb(c, label_free);
    component_set_init_cb(c, label_init);
    component_set_layout_cb(c, label_layout);
    return c;
}

component *label_create_title(const char *text) {
    component *c = label_create(text);
    label_set_text_horizontal_align(c, TEXT_ALIGN_CENTER);
    label_set_color_theme(c, 1); // Secondary color
    return c;
}
