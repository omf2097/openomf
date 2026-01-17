/**
 * @file vga_state.c
 * @brief VGA palette state management implementation
 */

#include "video/vga_state.h"

#include "game/game_state.h"
#include "utils/png_writer.h"
#include <assert.h>

#define MAX_TRANSFORMER_COUNT 8 ///< Maximum number of palette transformers per frame

/**
 * @brief Palette transformer entry
 */
typedef struct palette_transformer {
    vga_palette_transform callback; ///< Transform callback function
    void *userdata;                 ///< User-provided context data
} palette_transformer;

/**
 * @brief VGA state container
 */
typedef struct vga_state {
    vga_palette pushed;                                      ///< Stashed palette for push/pop
    vga_palette base;                                        ///< Base palette before transforms
    vga_palette current;                                     ///< Current palette after transforms
    damage_tracker dmg_base;                                 ///< Damage tracker for base palette
    damage_tracker dmg_previous;                             ///< Previous frame's damage state
    damage_tracker dmg_current;                              ///< Current frame's damage state
    vga_remap_tables remaps;                                 ///< Palette remap tables
    bool dirty_remaps;                                       ///< Whether remaps need updating
    palette_transformer transformers[MAX_TRANSFORMER_COUNT]; ///< Registered transformers
    unsigned int transformer_count;                          ///< Number of active transformers
} vga_state;

static vga_state state; ///< Global VGA state instance

void vga_state_init(void) {
    memset(&state, 0, sizeof(vga_state));
    damage_reset(&state.dmg_base);
    damage_reset(&state.dmg_previous);
    damage_reset(&state.dmg_current);
}

void vga_state_close(void) {
    vga_state_init();
}

void vga_state_push_palette(void) {
    memcpy(&state.pushed, &state.base, sizeof(vga_palette));
}

void vga_state_pop_palette(void) {
    memcpy(&state.base, &state.pushed, sizeof(vga_palette));
    damage_set_all(&state.dmg_base);
}

void vga_state_render(void) {
    damage_tracker tmp;

    // We only want to render new state if something has changed. Otherwise, no-op.
    if(state.dmg_previous.dirty || state.dmg_base.dirty || state.transformer_count) {
        // Copy base palette as the starting state, along with dirtiness data.
        memcpy(&state.current, &state.base, sizeof(vga_palette));
        damage_copy(&tmp, &state.dmg_base);
        damage_reset(&state.dmg_base);

        // Run transformers on top. These may modify the current palette and change dirtiness state.
        for(unsigned int i = 0; i < state.transformer_count; i++) {
            state.transformers[i].callback(&tmp, &state.current, state.transformers[i].userdata);
        }
        damage_combine(&state.dmg_current, &tmp);
        damage_combine(&state.dmg_current, &state.dmg_previous);
        damage_copy(&state.dmg_previous, &tmp);
        state.transformer_count = 0;
    }
}

void vga_state_mark_palette_flushed(void) {
    damage_reset(&state.dmg_current);
}

void vga_state_mark_remaps_flushed(void) {
    state.dirty_remaps = false;
}

void vga_state_mark_dirty(void) {
    damage_set_all(&state.dmg_base);
    state.dirty_remaps = true;
}

void vga_state_mul_base_palette(vga_index start, vga_index end, float multiplier) {
    assert(multiplier >= 0 && multiplier <= 1.0);
    vga_color *c;
    for(int i = start; i < end; i++) {
        c = &state.base.colors[i];
        c->r = multiplier * c->r;
        c->g = multiplier * c->g;
        c->b = multiplier * c->b;
    }
    damage_add_range(&state.dmg_base, start, end);
}

bool vga_state_is_palette_dirty(vga_palette **palette, vga_index *dirty_range_first, vga_index *dirty_range_last) {
    assert(palette != NULL);
    if(state.dmg_current.dirty) {
        *palette = &state.current;
        if(dirty_range_first != NULL) {
            *dirty_range_first = state.dmg_current.dirty_range_first;
        }
        if(dirty_range_last != NULL) {
            *dirty_range_last = state.dmg_current.dirty_range_last;
        }
        return true;
    }
    return false;
}

bool vga_state_is_remap_dirty(vga_remap_tables **remaps) {
    assert(remaps != NULL);
    if(state.dirty_remaps) {
        *remaps = &state.remaps;
        return true;
    }
    return false;
}

void vga_state_set_remaps_from(const vga_remap_tables *src) {
    assert(src != NULL);
    memcpy(&state.remaps, src, sizeof(vga_remap_tables));
    state.dirty_remaps = true;
}

void vga_state_set_base_palette_from(const vga_palette *src) {
    assert(src != NULL);
    memcpy(&state.base, src, sizeof(vga_palette));
    damage_set_all(&state.dmg_base);
}

void vga_state_set_base_palette_from_range(const vga_palette *src, vga_index dst_start, vga_index src_start,
                                           vga_index count) {
    assert(src != NULL);
    assert(dst_start + count <= 256);
    assert(src_start + count <= 256);
    memcpy(&state.base.colors[dst_start], &src->colors[src_start], count * 3);
    damage_add_range(&state.dmg_base, dst_start, dst_start + count);
}

void vga_state_set_base_palette_index(vga_index index, const vga_color *color) {
    assert(color != NULL);
    state.base.colors[index] = *color;
    damage_add_range(&state.dmg_base, index, index + 1);
}

void vga_state_set_base_palette_range(vga_index start, vga_index count, vga_color *src_colors) {
    assert(start + count <= 256);
    memcpy(&state.base.colors[start], src_colors, count * 3);
    damage_add_range(&state.dmg_base, start, start + count);
}

void vga_state_copy_base_palette_range(vga_index dst, vga_index src, vga_index count) {
    assert(dst + count <= 256);
    assert(src + count <= 256);
    memmove(&state.base.colors[dst], &state.base.colors[src], count * 3);
    damage_add_range(&state.dmg_base, dst, dst + count);
}

void vga_state_enable_palette_transform(vga_palette_transform transform_callback, void *userdata) {
#ifndef NDEBUG
    for(unsigned int i = 0; i < state.transformer_count; i++) {
        if(state.transformers[i].callback == transform_callback && state.transformers[i].userdata == userdata) {
            assert(!"duplicate transform");
        }
    }
#endif

    assert(state.transformer_count < MAX_TRANSFORMER_COUNT - 1);
    state.transformers[state.transformer_count].callback = transform_callback;
    state.transformers[state.transformer_count].userdata = userdata;
    state.transformer_count++;
}

/**
 * @brief Write a debug screenshot of the current palette
 * @param filename Output file path
 */
void vga_state_debug_screenshot(const path *filename) {
    unsigned char img[256];
    for(int i = 0; i < 256; i++) {
        img[i] = i;
    }
    write_paletted_png(filename, 16, 16, &state.current, img);
}
