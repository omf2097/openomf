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
 * Jumps over a line of text, and finds the starting index of the next line. If no text could not be fit into
 * the given space, start_index is returned.
 */
size_t find_next_line_end(const str *buf, const font *font, text_row_direction direction, size_t start_index,
                          uint8_t letter_spacing, uint16_t max_width) {
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
        // Linebreak encountered, we are done here.
        if(ptr[i] == '\n') {
            return i + 1;
        }

        // Check if this is a potential cut-off point. Cut-off is used if we run out of space
        // before we reach an actual linebreak.
        if(ptr[i] == ' ' || ptr[i] == '-') {
            cut_off = i + 1;
            found_cut_off = true;
        }

        // Special characters handled, try to get printable surface.
        const surface *s = font_get_surface(font, ptr[i]);
        if(s == NULL) {
            // Character has no surface, just skip it since we can't do anything with it.
            assert(false && "Encountered unprintable character!");
            continue;
        }

        pos += letter_spacing + (direction == TEXT_ROW_HORIZONTAL ? s->w : s->h);
        if(pos > max_width) {
            // No more room on the row! If we found a cut-off point, we use it (space, line).
            // Otherwise try to print what we can and bail. This may break words outside word boundaries,
            // but this is a best-effort case.
            if(found_cut_off) {
                return cut_off;
            } else {
                return i;
            }
        }
    }
    return len;
}

typedef struct row_size {
    uint16_t w;
    uint16_t h;
} row_size;

row_size find_row_metrics(const str *buf, const font *font, text_row_direction direction, size_t start, size_t end) {
    row_size ret = {0, 0};
    const char *ptr = str_c(buf);
    for(size_t i = start; i < end; i++) {
        const surface *s = font_get_surface(font, ptr[i]);
        if(s == NULL) {
            assert(false && "Encountered unprintable character!");
            continue;
        }
        ret.h = max(ret.h, (direction == TEXT_ROW_HORIZONTAL) ? s->h : s->w);
        ret.w += (direction == TEXT_ROW_HORIZONTAL) ? s->w : s->h;
    }
    return ret;
}

typedef struct text_row {
    size_t start_index; // First letter of this row
    size_t end_index;   // First letter of next row
    row_size size;
} text_row;

uint16_t find_rows(vector *rows, const str *buf, const font *font, text_row_direction direction, uint8_t letter_spacing,
                   uint8_t line_spacing, uint16_t max_width, uint16_t max_height) {
    size_t start = 0, len = str_size(buf);
    size_t line = 0;
    size_t row_heights = 0, total_height = 0;
    row_size row_size;

    while(start < len) {
        size_t next_start = find_next_line_end(buf, font, direction, start, letter_spacing, max_width);
        if(next_start == start) {
            // If we run out of horizontal space, stop here.
            assert(false && "Ran out of horizontal space when flowing text!");
            goto exit;
        }

        // Check if the last character is something we should trim off.
        size_t end = next_start;
        char last_char = str_at(buf, next_start - 1);
        if(last_char == '\n' || last_char == ' ') {
            end--;
        }

        row_size = find_row_metrics(buf, font, direction, start, end);
        total_height = row_heights + row_size.h + line * line_spacing;
        if(total_height > max_height) {
            // If we run out of vertical space, stop here.
            assert(false && "Ran out of vertical space when flowing text!");
            goto exit;
        }

        text_row *row = vector_append_ptr(rows);
        row->start_index = start;
        row->end_index = end;
        row->size = row_size;

        // Step to the next row. End position is always the start position of the next line.
        row_heights += row_size.h;
        start = next_start;
        line++;
    }

exit:
    return row_heights + line * line_spacing;
}

uint16_t valign_offset(text_vertical_align align, uint16_t bbox_h, uint16_t block_h) {
    switch(align) {
        case ALIGN_TEXT_TOP:
            return 0;
        case ALIGN_TEXT_MIDDLE:
            return (bbox_h - block_h) / 2;
        case ALIGN_TEXT_BOTTOM:
            return bbox_h - block_h;
    }
    assert(false && "Unknown text_vertical_align");
    return 0; // Should never come here.
}

uint16_t halign_offset(text_horizontal_align align, uint16_t bbox_w, uint16_t block_w) {
    switch(align) {
        case ALIGN_TEXT_LEFT:
            return 0;
        case ALIGN_TEXT_CENTER:
            return (bbox_w - block_w) / 2;
        case ALIGN_TEXT_RIGHT:
            return bbox_w - block_w;
    }
    assert(false && "Unknown text_horizontal_align");
    return 0; // Should never come here.
}

void text_layout_compute(text_layout *layout, const str *buf, const font *font, text_vertical_align vertical_align,
                         text_horizontal_align horizontal_align, text_margin margin, text_row_direction direction,
                         uint8_t line_spacing, uint8_t letter_spacing, uint8_t max_lines) {
    assert(buf != NULL);
    assert(layout->w > margin.left + margin.right);
    assert(layout->h > margin.top + margin.bottom);
    const char *src = str_c(buf);

    // Find the actual drawable area of the bounding box
    uint16_t w = layout->w - margin.left - margin.right;
    uint16_t h = layout->h - margin.top - margin.bottom;
    uint16_t max_width = (direction == TEXT_ROW_HORIZONTAL) ? w : h;
    uint16_t max_height = (direction == TEXT_ROW_HORIZONTAL) ? h : w;

    // Figure out how many rows we render, and what their sizes are.
    vector rows;
    vector_create(&rows, sizeof(text_row));
    uint16_t block_h = find_rows(&rows, buf, font, direction, letter_spacing, line_spacing, max_width, max_height);

    // Clear any lingering data now, as we are ready to write!
    vector_clear(&layout->items);

    // Walk through the generated rows, so that we can find row alignments and positions of the individual letters.
    iterator it;
    text_row *row = NULL;
    vector_iter_begin(&rows, &it);
    uint16_t y = valign_offset(vertical_align, max_height, block_h);
    foreach(it, row) {
        uint16_t x = halign_offset(horizontal_align, max_width, row->size.w);
        for(size_t i = row->start_index; i < row->end_index; i++) {
            const surface *glyph = font_get_surface(font, src[i]);
            if(glyph != NULL) {
                text_layout_item *item = vector_append_ptr(&layout->items);
                item->glyph = glyph;
                item->x = x;
                item->y = y;
                x += item->glyph->w + letter_spacing;
            }
        }
        y += row->size.h + line_spacing;
    }

    // This is the temporary row buffer, it's no longer needed.
    vector_free(&rows);
}
