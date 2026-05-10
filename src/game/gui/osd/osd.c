#include "game/gui/osd/osd.h"

#include "game/gui/text/text.h"
#include "utils/list.h"
#include "video/video.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#define OSD_TTL 700
#define OSD_FADE_TICKS 50

#define OSD_HORIZONTAL_MARGIN 4
#define OSD_BOTTOM_MARGIN 4
#define OSD_BLOCK_MARGIN 2
#define OSD_TOP_LIMIT (NATIVE_H / 2)

#define OSD_BUF_MAX 256

#define OSD_DEFAULT_COLOR 0xFD
#define OSD_DEFAULT_SHADOW 0xC0

typedef struct osd_block {
    text *data;
    int created_at;
} osd_block;

typedef struct osd {
    list blocks;
    int tick;
    vga_index default_text_color;
    vga_index default_text_shadow_color;
} osd;

static osd osd_state;

static void osd_block_free_cb(void *data) {
    osd_block *block = data;
    if(block->data) {
        text_free(&block->data);
    }
}

bool osd_init(void) {
    list_create_cb(&osd_state.blocks, osd_block_free_cb);
    osd_state.tick = 0;
    osd_state.default_text_color = OSD_DEFAULT_COLOR;
    osd_state.default_text_shadow_color = OSD_DEFAULT_SHADOW;
    return true;
}

void osd_close(void) {
    list_free(&osd_state.blocks);
}

void osd_clear(void) {
    list_clear(&osd_state.blocks);
}

void osd_set_default_color(const vga_index color) {
    osd_state.default_text_color = color;
}

void osd_set_default_shadow_color(const vga_index shadow_color) {
    osd_state.default_text_shadow_color = shadow_color;
}

static text *create_text_from(const char *msg, const vga_index color, const vga_index shadow_color) {
    text *t = text_create_from_c(msg);
    text_set_font(t, FONT_SMALL);
    text_set_color(t, color);
    text_set_shadow_color(t, shadow_color);
    text_set_shadow_style(t, GLYPH_SHADOW_BOTTOM | GLYPH_SHADOW_RIGHT);
    text_set_horizontal_align(t, TEXT_ALIGN_LEFT);
    text_set_vertical_align(t, TEXT_ALIGN_TOP);
    text_set_word_wrap(t, true);
    text_set_bounding_box(t, NATIVE_W - 2 * OSD_HORIZONTAL_MARGIN, TEXT_BBOX_MAX);
    text_generate_layout(t);
    return t;
}

static void push_block(const char *msg, const vga_index color, const vga_index shadow_color) {
    assert(msg != NULL);
    osd_block entry;
    entry.data = create_text_from(msg, color, shadow_color);
    entry.created_at = osd_state.tick;
    list_append(&osd_state.blocks, &entry, sizeof(osd_block));
}

void osd_vprint_color(const vga_index color, const vga_index shadow_color, const char *fmt, va_list ap) {
    char buf[OSD_BUF_MAX];
    vsnprintf(buf, OSD_BUF_MAX, fmt, ap);
    push_block(buf, color, shadow_color);
}

void osd_vprint(const char *fmt, va_list ap) {
    osd_vprint_color(osd_state.default_text_color, osd_state.default_text_shadow_color, fmt, ap);
}

void osd_print(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    osd_vprint_color(osd_state.default_text_color, osd_state.default_text_shadow_color, fmt, ap);
    va_end(ap);
}

void osd_print_color(const vga_index color, const vga_index shadow_color, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    osd_vprint_color(color, shadow_color, fmt, ap);
    va_end(ap);
}

void osd_tick(void) {
    osd_state.tick++;
    iterator it;
    list_iter_begin(&osd_state.blocks, &it);
    osd_block *block;
    foreach(it, block) {
        if(osd_state.tick - block->created_at > OSD_TTL) {
            list_delete(&osd_state.blocks, &it);
        }
    }
}

void osd_render(void) {
    iterator it;
    list_iter_end(&osd_state.blocks, &it);
    osd_block *block;
    int pos_y = NATIVE_H - OSD_BOTTOM_MARGIN;
    while((block = iter_prev(&it)) != NULL) {
        pos_y -= text_get_layout_height(block->data);
        if(pos_y < OSD_TOP_LIMIT) {
            break;
        }

        const int age = osd_state.tick - block->created_at;
        const int fade_start = OSD_TTL - OSD_FADE_TICKS;
        const uint8_t opacity = (age > fade_start) ? (255 - (age - fade_start) * 255 / OSD_FADE_TICKS) : 255;

        text_draw_opacity(block->data, OSD_HORIZONTAL_MARGIN, pos_y, opacity);
        pos_y -= OSD_BLOCK_MARGIN;
    }
}
