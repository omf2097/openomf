#include <assert.h>

#include "game/gui/text/text.h"
#include "game/gui/text/text_layout.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/miscmath.h"
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
    int16_t x_off;       // X offset relative to render coordinate
    int16_t y_off;       // Y offset relative to render coordinate
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

struct text_document {
    vector text_objects;
};

static void defaults(text *t) {
    t->text_color = 0xFD;
    t->shadow_color = 0xC0;
    t->vertical_align = TEXT_ALIGN_TOP;
    t->horizontal_align = TEXT_ALIGN_LEFT;
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

void text_free_direct(void *p) {
    text *t = (text *)p;
    str_free(&t->buf);
    text_layout_free(&t->layout);
}

text_document *text_document_create(void) {
    text_document *d = omf_calloc(1, sizeof(text_document));
    vector_create_with_size_cb(&d->text_objects, sizeof(text), 0, text_free_direct);
    return d;
}

void text_document_free(text_document **d) {
    if(*d != NULL) {
        vector_free(&(*d)->text_objects);
        omf_free(*d);
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

void text_generate_document(text_document *td, str *buf0, font_size font_sz, uint16_t w, uint16_t h,
                            vga_index text_color, vga_index shadow_color, text_vertical_align vertical_align,
                            text_horizontal_align horizontal_align, text_margin margin, uint8_t line_spacing,
                            uint8_t letter_spacing, uint8_t shadow, uint8_t glyph_margin) {
    size_t start = 0;
    size_t len = str_size(buf0);

    const char *buf = str_c(buf0);

    uint16_t current_width = w;
    uint16_t current_height = w;
    text_vertical_align current_vertical_align = vertical_align;
    text_horizontal_align current_horizontal_align = horizontal_align;
    font_size current_font_size = font_sz;
    uint8_t current_shadow = shadow;
    uint16_t current_x_off = 0;
    uint16_t current_y_off = 0;
    uint16_t center = 160;
    const font *initial_font = fonts_get_font(font_sz);
    uint16_t current_line_spacing = line_spacing + initial_font->h;
    vga_index current_text_color = text_color;
    vga_index current_shadow_color = shadow_color;
    int bytes_used;
    int count = 0;

    while(start < len) {
        while(buf[start] == '{') {
            if(strncmp(buf + start, "{CENTER OFF}", 12) == 0) {
                start += 12;
                current_horizontal_align = TEXT_ALIGN_LEFT;
            } else if(strncmp(buf + start, "{CENTER ON}", 11) == 0) {
                start += 11;
                current_horizontal_align = TEXT_ALIGN_CENTER;
            } else if(strncmp(buf + start, "{SIZE 8}", 8) == 0) {
                start += 8;
                // swap based on initial font family
                if(font_sz == FONT_NET1 || font_sz == FONT_NET2) {
                    current_font_size = FONT_NET1;
                } else {
                    current_font_size = FONT_BIG;
                }
            } else if(strncmp(buf + start, "{SIZE 6}", 8) == 0) {
                start += 8;
                // swap based on initial font family
                if(font_sz == FONT_NET1 || font_sz == FONT_NET2) {
                    current_font_size = FONT_NET2;
                } else {
                    current_font_size = FONT_SMALL;
                }
            } else if(strncmp(buf + start, "{SHADOWS ON}", 12) == 0) {
                start += 12;
                current_shadow = GLYPH_SHADOW_RIGHT | GLYPH_SHADOW_BOTTOM;
            } else if(strncmp(buf + start, "{SHADOWS OFF}", 13) == 0) {
                start += 13;
                current_shadow = GLYPH_SHADOW_NONE;
            } else if(strncmp(buf + start, "{COLOR:YELLOW}", 14) == 0) {
                start += 14;
                current_text_color = TEXT_YELLOW;
                current_shadow_color = TEXT_SHADOW_YELLOW;
            } else if(strncmp(buf + start, "{COLOR:DEFAULT}", 15) == 0) {
                start += 15;
                current_text_color = text_color;
                current_shadow_color = shadow_color;
            } else if(sscanf(buf + start, "{WIDTH %hu}%n", &current_width, &bytes_used) == 1 && bytes_used > 0) {
                current_width = max2(8, min2(current_width, 320));
                start += bytes_used;
            } else if(sscanf(buf + start, "{VMOVE %hu}%n", &current_y_off, &bytes_used) == 1 && bytes_used > 0) {
                current_y_off = min2(current_y_off, 200);
                start += bytes_used;
            } else if(sscanf(buf + start, "{CENTER %hu}%n", &center, &bytes_used) == 1 && bytes_used > 0) {
                start += bytes_used;
                // TODO we need to handle this properly, it will likely update the x offset
            } else if(sscanf(buf + start, "{COLOR %hhu}%n", &current_text_color, &bytes_used) == 1 && bytes_used > 0) {
                start += bytes_used;
            } else if(sscanf(buf + start, "{SPACING %hu}%n", &current_line_spacing, &bytes_used) == 1 &&
                      bytes_used > 0) {
                start += bytes_used;
            } else if(sscanf(buf + start, "{SPACINGG %hu}%n", &current_line_spacing, &bytes_used) == 1 &&
                      bytes_used > 0) {
                // handle known typo in page 2 of help menu. We assume the original parser is just looking at the string
                // prefixes
                start += bytes_used;
            } else {
                char *end = strchr(buf + start, '}');
                if(end) {
                    char tmp[20];
                    memcpy(tmp, buf + start, sizeof(tmp));
                    tmp[19] = 0;
                    // log_warn("unhandled markup detected %s", tmp);
                    start += end + 1 - (buf + start);
                } else {
                    // log_warn("unterminated markup detected");
                    return;
                }
            }
        }

        text *t = vector_append_ptr(&td->text_objects);
        defaults(t);
        text_layout_create(&t->layout);
        t->font = current_font_size;
        const font *font = fonts_get_font(t->font);
        t->w = current_width;
        t->h = current_height;
        t->x_off = current_x_off;
        t->y_off = current_y_off;
        t->text_color = current_text_color;
        t->shadow_color = current_shadow_color;
        t->vertical_align = current_vertical_align;
        t->horizontal_align = current_horizontal_align;
        t->margin = margin;
        // t->direction = direction;
        t->line_spacing = max2(0, current_line_spacing - font->h);
        t->letter_spacing = letter_spacing;
        t->shadow = current_shadow;
        t->glyph_margin = glyph_margin;
        // t->max_lines = max_lines;

        char *endptr = strchr(buf + start, '{');
        if(endptr) {
            size_t end = endptr - buf;
            str_from_slice(&t->buf, buf0, start, end);
            start = end;
        } else {
            if(start == len) {
                // empty trailing string
                text_layout_free(&t->layout);
                vector_delete_at(&td->text_objects, count);
                continue;
            }
            str_from_slice(&t->buf, buf0, start, len);
            start = len;
        }

        size_t line_len = str_size(&t->buf);
        bool found = false;

        for(size_t i = 0; i < line_len; i++) {
            if(!isspace(str_at(&t->buf, i))) {
                found = true;
                break;
            }
        }

        if(!found) {
            str_rstrip(&t->buf);
        }

        if(str_size(&t->buf) == 0 || (count == 0 && !found)) {
            text_layout_free(&t->layout);
            vector_delete_at(&td->text_objects, count);
            continue;
        }

        t->margin.bottom = t->line_spacing;

        if(count) {
            // not the first one, so drop the top margin
            t->margin.top = 0;
        } else if(start == len) {
            // last one, so restore the bottom margin
            t->margin.bottom = margin.bottom;
        }

        // TODO be careful with margins! the first element should get the top margin, the last one should get the bottom
        // one and the right margin should only be provided if the line is likely to wrap?
        text_layout_compute(&t->layout, &t->buf, font, t->w, t->h, t->vertical_align, t->horizontal_align, t->margin,
                            t->direction, t->line_spacing, t->letter_spacing, 255);
        t->cache_flags &= ~INVALIDATE_LAYOUT;
        count++;
    }
}

uint16_t text_document_get_text_count(text_document *d) {
    return vector_size(&d->text_objects);
}

text *text_document_get_text(text_document *d, uint16_t index) {
    return vector_get(&d->text_objects, index);
}

void text_document_draw(text_document *d, uint16_t offset_x, uint16_t offset_y) {
    iterator it;
    text *item;
    vector_iter_begin(&d->text_objects, &it);
    foreach(it, item) {
        text_draw(item, offset_x, offset_y);
        // for now, assume every text object ends with a newline
        offset_y += item->layout.h;
    }
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
