#include <assert.h>

#include "game/gui/text/text.h"
#include "game/gui/text/text_layout.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/vector.h"
#include "video/surface.h"
#include "video/video.h"

enum cache_flags
{
    INVALIDATE_NONE = 0,
    INVALIDATE_LAYOUT = 0x1,
    INVALIDATE_STYLE = 0x2,
    INVALIDATE_ALL = 0xFF,
};

struct text {
    str buf;             // Copy of text
    font_size font;      // Font size to use
    uint16_t w;          // Bounding box width
    uint16_t h;          // Bounding box height
    text_layout layout;  // Each glyph position in a nice list, easy to render.
    uint8_t cache_flags; // Cache invalidation flags

    // Text rendering options
    vga_index text_color;
    vga_index shadow_color;
    text_vertical_align vertical_align;
    text_horizontal_align horizontal_align;
    text_margin margin;
    text_row_direction direction;
    uint8_t line_spacing;
    uint8_t letter_spacing;
    uint8_t shadow;
    uint8_t glyph_margin;
    uint8_t word_wrap;
};

static void defaults(text *t) {
    t->text_color = 0xFD;
    t->shadow_color = 0xC0;
    t->vertical_align = ALIGN_TEXT_TOP;
    t->horizontal_align = ALIGN_TEXT_LEFT;
    t->margin.left = 0;
    t->margin.right = 0;
    t->margin.top = 0;
    t->margin.bottom = 0;
    t->line_spacing = 1;
    t->letter_spacing = 0;
    t->direction = TEXT_ROW_HORIZONTAL;
    t->shadow = GLYPH_SHADOW_NONE;
    t->glyph_margin = 0;
    t->word_wrap = true;
}

text *text_create(font_size font) {
    text *t = omf_calloc(1, sizeof(text));
    t->font = font;
    t->w = t->h = 0;
    t->cache_flags = INVALIDATE_NONE;
    text_layout_create(&t->layout);
    str_create(&t->buf);
    defaults(t);
    return t;
}

text *text_create_with_size(font_size font, uint16_t w, uint16_t h) {
    text *t = text_create(font);
    t->w = w;
    t->h = h;
    return t;
}

text *text_create_from_c(font_size font, uint16_t w, uint16_t h, const char *src) {
    text *t = text_create_with_size(font, w, h);
    text_set_from_c(t, src);
    return t;
}

text *text_create_from_str(font_size font, uint16_t w, uint16_t h, const str *src) {
    text *t = text_create_with_size(font, w, h);
    text_set_from_str(t, src);
    return t;
}

void text_free(text **t) {
    if(*t != NULL) {
        str_free(&(*t)->buf);
        text_layout_free(&(*t)->layout);
        omf_free(*t);
    }
}

void text_set_from_c(text *t, const char *src) {
    if(src == NULL) {
        str_truncate(&t->buf, 0);
    } else {
        str_set_c(&t->buf, src);
    }
    t->cache_flags |= INVALIDATE_ALL;
}

void text_set_from_str(text *t, const str *src) {
    assert(src);
    str_set(&t->buf, src);
    t->cache_flags |= INVALIDATE_ALL;
}

void text_set_font(text *t, font_size font) {
    if(t->font != font) {
        t->font = font;
        t->cache_flags |= INVALIDATE_LAYOUT;
    }
}

void text_set_bounding_box(text *t, uint16_t w, uint16_t h) {
    if(t->w != w || t->h != h) {
        t->w = w;
        t->h = h;
        t->cache_flags |= INVALIDATE_LAYOUT;
    }
}

void text_set_color(text *t, vga_index color) {
    if(t->text_color != color) {
        t->text_color = color;
        t->cache_flags |= INVALIDATE_STYLE;
    }
}

void text_set_shadow_color(text *t, vga_index color) {
    if(t->shadow_color != color) {
        t->shadow_color = color;
        t->cache_flags |= INVALIDATE_STYLE;
    }
}

void text_set_vertical_align(text *t, text_vertical_align align) {
    if(t->vertical_align != align) {
        t->vertical_align = align;
        t->cache_flags |= INVALIDATE_LAYOUT;
    }
}

void text_set_horizontal_align(text *t, text_horizontal_align align) {
    if(t->horizontal_align != align) {
        t->horizontal_align = align;
        t->cache_flags |= INVALIDATE_LAYOUT;
    }
}

void text_set_margin(text *t, const text_margin m) {
    if(t->margin.left != m.left || t->margin.right != m.right || t->margin.top != m.top ||
       t->margin.bottom != m.bottom) {
        t->margin = m;
        t->cache_flags |= INVALIDATE_LAYOUT;
    }
}

void text_set_direction(text *t, text_row_direction direction) {
    if(t->direction != direction) {
        t->direction = direction;
        t->cache_flags |= INVALIDATE_LAYOUT;
    }
}

void text_set_line_spacing(text *t, uint8_t line_spacing) {
    if(t->line_spacing != line_spacing) {
        t->line_spacing = line_spacing;
        t->cache_flags |= INVALIDATE_LAYOUT;
    }
}

void text_set_letter_spacing(text *t, uint8_t letter_spacing) {
    if(t->letter_spacing != letter_spacing) {
        t->letter_spacing = letter_spacing;
        t->cache_flags |= INVALIDATE_LAYOUT;
    }
}

void text_set_shadow_style(text *t, uint8_t shadow) {
    if(t->shadow != shadow) {
        t->shadow = shadow & GLYPH_SHADOW_ALL;
        t->cache_flags |= INVALIDATE_STYLE;
    }
}

void text_set_glyph_margin(text *t, uint8_t glyph_margin) {
    if(t->glyph_margin != glyph_margin) {
        t->glyph_margin = glyph_margin;
        t->cache_flags |= INVALIDATE_LAYOUT;
    }
}

void text_set_word_wrap(text *t, bool word_wrap) {
    if(t->word_wrap != word_wrap) {
        t->word_wrap = word_wrap;
        t->cache_flags |= INVALIDATE_LAYOUT;
    }
}

void text_get_str(const text *t, str *dst) {
    str_from(dst, &t->buf);
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

void text_get_margin(const text *t, uint8_t *left, uint8_t *right, uint8_t *top, uint8_t *bottom) {
    if(left != NULL)
        *left = t->margin.left;
    if(right != NULL)
        *right = t->margin.right;
    if(top != NULL)
        *top = t->margin.top;
    if(bottom != NULL)
        *bottom = t->margin.bottom;
}

text_row_direction text_get_direction(const text *t) {
    return t->direction;
}

uint8_t text_get_line_spacing(const text *t) {
    return t->line_spacing;
}

uint8_t text_get_letter_spacing(const text *t) {
    return t->letter_spacing;
}

uint8_t text_get_shadow_style(const text *t) {
    return t->shadow;
}

uint8_t text_get_glyph_margin(const text *t) {
    return t->glyph_margin;
}

bool text_get_word_wrap(const text *t) {
    return t->word_wrap;
}

uint16_t text_get_layout_width(const text *t) {
    assert(!(t->cache_flags & INVALIDATE_LAYOUT));
    return t->layout.w;
}

uint16_t text_get_layout_height(const text *t) {
    assert(!(t->cache_flags & INVALIDATE_LAYOUT));
    return t->layout.h;
}

size_t text_get_layout_rows(const text *t) {
    assert(!(t->cache_flags & INVALIDATE_LAYOUT));
    return t->layout.rows;
}

void text_generate_layout(text *t) {
    if(t->cache_flags & INVALIDATE_LAYOUT) {
        const font *font = fonts_get_font(t->font);
        text_layout_compute(&t->layout, &t->buf, font, t->w, t->h, t->vertical_align, t->horizontal_align, t->margin,
                            t->direction, t->line_spacing, t->letter_spacing, t->word_wrap);
        t->cache_flags &= ~INVALIDATE_LAYOUT;
    }
}

static inline void draw_shadow(const text_layout_item *item, uint16_t offset_x, uint16_t offset_y, uint8_t shadow,
                               vga_index color) {
    int palette_offset = (int)color - 1;
    int x = item->x + offset_x;
    int y = item->y + offset_y;
    if(shadow & GLYPH_SHADOW_RIGHT)
        video_draw_offset(item->glyph, x + 1, y, palette_offset, 255);
    if(shadow & GLYPH_SHADOW_LEFT)
        video_draw_offset(item->glyph, x - 1, y, palette_offset, 255);
    if(shadow & GLYPH_SHADOW_BOTTOM)
        video_draw_offset(item->glyph, x, y + 1, palette_offset, 255);
    if(shadow & GLYPH_SHADOW_TOP)
        video_draw_offset(item->glyph, x, y - 1, palette_offset, 255);
}

static inline void draw_foreground(const text_layout_item *item, uint16_t offset_x, uint16_t offset_y,
                                   vga_index color) {
    int palette_offset = (int)color - 1;
    int x = item->x + offset_x;
    int y = item->y + offset_y;
    video_draw_offset(item->glyph, x, y, palette_offset, 255);
}

void text_draw(text *t, uint16_t offset_x, uint16_t offset_y) {
    assert(t != NULL);
    text_layout_item *item;
    iterator it;
    text_generate_layout(t); // Ensure we have a layout

    // First the shadows for all letters.
    vector_iter_begin(&t->layout.items, &it);
    foreach(it, item) {
        draw_shadow(item, offset_x, offset_y, t->shadow, t->shadow_color);
    }

    // Then the actual letter foregrounds.
    vector_iter_begin(&t->layout.items, &it);
    foreach(it, item) {
        draw_foreground(item, offset_x, offset_y, t->text_color);
    }
}
