#include <assert.h>

#include "game/gui/text/text.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/vector.h"
#include "video/surface.h"

enum cache_flags
{
    INVALIDATE_NONE = 0,
    INVALIDATE_LAYOUT = 0x1,
    INVALIDATE_SURFACE = 0x2,
    INVALIDATE_ALL = 0xFF,
};

typedef struct layout_item {
    uint16_t x;
    uint16_t y;
    surface *glyph;
} layout_item;

struct text {
    str buf;              // Copy of text
    font_size font;       // Font size to use
    uint16_t w;           // Bounding box width
    uint16_t h;           // Bounding box height
    vector cached_layout; // Each glyph position in a nice list, easy to render.
    uint8_t cache_flags;  // Cache invalidation flags

    // Text rendering options
    vga_index text_color;
    vga_index shadow_color;
    text_vertical_align vertical_align;
    text_horizontal_align horizontal_align;
    text_padding padding;
    text_direction direction;
    uint8_t shadow;
    uint8_t glyph_margin;
    uint8_t max_lines;
};

static void defaults(text *t) {
    t->text_color = 0xFD;
    t->shadow_color = 0xC0;
    t->vertical_align = TEXT_TOP;
    t->horizontal_align = TEXT_LEFT;
    t->padding.left = 0;
    t->padding.right = 0;
    t->padding.top = 0;
    t->padding.bottom = 0;
    t->direction = TEXT_HORIZONTAL;
    t->shadow = TEXT_SHADOW_NONE;
    t->glyph_margin = 0;
    t->max_lines = UINT8_MAX;
}

text *text_create(font_size font, uint16_t w, uint16_t h) {
    text *t = omf_calloc(1, sizeof(text));
    t->font = font;
    t->w = w;
    t->h = h;
    t->cache_flags |= INVALIDATE_ALL;
    str_create(&t->buf);
    defaults(t);
    return t;
}

text *text_create_from_c(font_size font, uint16_t w, uint16_t h, const char *src) {
    text *t = text_create(font, w, h);
    text_set_from_c(t, src);
    return t;
}

text *text_create_from_str(font_size font, uint16_t w, uint16_t h, const str *src) {
    text *t = text_create(font, w, h);
    text_set_from_str(t, src);
    return t;
}

void text_free(text **t) {
    str_free(&(*t)->buf);
    omf_free(*t);
}

void text_set_from_c(text *t, const char *src) {
    assert(src);
    str_set_c(&t->buf, src);
    t->cache_flags |= INVALIDATE_ALL;
}

void text_set_from_str(text *t, const str *src) {
    assert(src);
    str_set(&t->buf, src);
    t->cache_flags |= INVALIDATE_ALL;
}

void text_set_font(text *t, font_size font) {
    assert(font);
    t->font = font;
    t->cache_flags |= INVALIDATE_ALL;
}

void text_set_bounding_box(text *t, uint16_t w, uint16_t h) {
    t->w = w;
    t->h = h;
    t->cache_flags |= INVALIDATE_ALL;
}

void text_set_color(text *t, vga_index color) {
    t->text_color = color;
    t->cache_flags |= INVALIDATE_SURFACE;
}

void text_set_shadow_color(text *t, vga_index color) {
    t->shadow_color = color;
    t->cache_flags |= INVALIDATE_SURFACE;
}

void text_set_vertical_align(text *t, text_vertical_align align) {
    t->vertical_align = align;
    t->cache_flags |= INVALIDATE_ALL;
}

void text_set_horizontal_align(text *t, text_horizontal_align align) {
    t->horizontal_align = align;
    t->cache_flags |= INVALIDATE_ALL;
}

void text_set_padding(text *t, uint8_t left, uint8_t right, uint8_t top, uint8_t bottom) {
    t->padding.left = left;
    t->padding.right = right;
    t->padding.top = top;
    t->padding.bottom = bottom;
    t->cache_flags |= INVALIDATE_ALL;
}

void text_set_direction(text *t, text_direction direction) {
    t->direction = direction;
    t->cache_flags |= INVALIDATE_ALL;
}

void text_set_shadow_style(text *t, uint8_t shadow) {
    t->shadow = shadow & TEXT_SHADOW_ALL;
    t->cache_flags |= INVALIDATE_SURFACE;
}

void text_set_glyph_margin(text *t, uint8_t glyph_margin) {
    t->glyph_margin = glyph_margin;
    t->cache_flags |= INVALIDATE_ALL;
}

void text_set_max_lines(text *t, uint8_t max) {
    t->max_lines = max;
    t->cache_flags |= INVALIDATE_ALL;
}

font_size text_get_font(const text *t) {
    return t->font;
}

void text_get_bounding_box(const text *t, uint16_t *w, uint16_t *h) {
    if(w != NULL)
        *w = t->w;
    if(h != NULL)
        *h = t->h;
}

vga_index text_get_color(const text *t) {
    return t->text_color;
}

vga_index text_get_shadow_color(const text *t) {
    return t->shadow_color;
}

text_vertical_align text_get_vertical_align(const text *t) {
    return t->vertical_align;
}

text_horizontal_align text_get_horizontal_align(const text *t) {
    return t->horizontal_align;
}

void text_get_padding(const text *t, uint8_t *left, uint8_t *right, uint8_t *top, uint8_t *bottom) {
    if(left != NULL)
        *left = t->padding.left;
    if(right != NULL)
        *right = t->padding.right;
    if(top != NULL)
        *top = t->padding.top;
    if(bottom != NULL)
        *bottom = t->padding.bottom;
}

text_direction text_get_direction(const text *t) {
    return t->direction;
}

uint8_t text_get_shadow_style(const text *t) {
    return t->shadow;
}

uint8_t text_get_glyph_margin(const text *t) {
    return t->glyph_margin;
}

uint8_t text_get_max_lines(const text *t) {
    return t->max_lines;
}

void text_generate_layout(text *t) {
    // TBD
}
