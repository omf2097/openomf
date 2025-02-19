#include "game/gui/text/text_layout.h"
#include "resources/fonts.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/miscmath.h"
#include "video/surface.h"

text_layout *text_layout_create(uint16_t w, uint16_t h) {
    text_layout *layout = omf_calloc(1, sizeof(text_layout));
    layout->w = w;
    layout->h = h;
    vector_create(&layout->items, sizeof(text_layout_item));
    return layout;
}

void text_layout_free(text_layout **layout) {
    vector_free(&(*layout)->items);
    omf_free(*layout);
}

/**
 * Jumps over a line of text, and finds the starting index of the next line. If text could not be fit into
 * the given space, start_index is returned.
 */
size_t find_next_line_end(const str *buf, const font *font, text_direction direction, size_t start_index,
                          uint16_t max_width) {
    assert(buf != NULL);
    assert(font != NULL);

    // Grab a pointer to the source string -- this is just faster to process.
    const char *ptr = str_c(buf);
    size_t len = str_size(buf);
    if(len == 0) {
        // Special case -- nothing to do here.
        return 0;
    }

    size_t cut_off = start_index;
    bool found_cut_off = false;
    uint16_t pos = 0;
    for(size_t i = start_index; i < len; i++) {
        fprintf(stderr, "Char %lld: ", i);
        if(ptr[i] == '\n') {
            fprintf(stderr, "linebreak found at %lld\n", i);
            return i + 1;
        }

        // Special characters handled, try to get printable surface.
        const surface *s = font_get_surface(font, ptr[i]);
        if(s == NULL) {
            // Character has no surface, just skip it.
            fprintf(stderr, "No surface!\n");
            continue;
        }

        uint16_t step = (direction == TEXT_HORIZONTAL) ? s->w : s->h;
        fprintf(stderr, "sur = (%d, %d), pos %d + %d / %d ...", s->w, s->h, pos, step, max_width);

        if(pos + step > max_width) {
            // If there is no more room in row direction, stop here.
            fprintf(stderr, "Out of room!\n");
            if(found_cut_off) {
                return cut_off;
            } else {
                return i;
            }
        }
        pos += step;

        if(ptr[i] == ' ' || ptr[i] == '-') {
            fprintf(stderr, " (Cut-off found at %lld) ...", i);
            cut_off = i + 1;
            found_cut_off = true;
        }

        fprintf(stderr, "Next!\n");
    }
    return len;
}

text_layout_error text_layout_compute(text_layout *layout, const str *buf, const font *font,
                                      text_vertical_align vertical_align, text_horizontal_align horizontal_align,
                                      text_padding padding, text_direction direction, uint8_t max_lines) {
    assert(buf != NULL);
    assert(layout->w > padding.left + padding.right);
    assert(layout->h > padding.top + padding.bottom);

    // Clear any lingering data, in case this layout is reused.
    vector_clear(&layout->items);

    uint16_t w = layout->w - padding.left - padding.right;
    uint16_t h = layout->h - padding.top - padding.bottom;
    uint16_t max_row = (direction == TEXT_HORIZONTAL) ? w : h;
    // uint16_t max_col = (direction == TEXT_HORIZONTAL) ? h : w;

    size_t start = 0;
    size_t len = str_size(buf);
    size_t line = 0;
    while(start < len) {
        size_t end = find_next_line_end(buf, font, direction, start, max_row);
        if(end == start) {
            // We couldn't fit in any text, stop here.
            return LAYOUT_NO_HORIZONTAL_SPACE;
        }
        fprintf(stderr, "line %lld: %lld -> %lld\n", line, start, end);
        start = end;
        line++;
    }

    return LAYOUT_NO_ERROR;
}
