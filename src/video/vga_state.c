#include "video/vga_state.h"

#include "utils/miscmath.h"
#include <string.h>

#define MAX_TRANSFORMER_COUNT 8

typedef struct palette_transformer {
    vga_palette_transform callback;
    void *userdata;
} palette_transformer;

typedef struct vga_state {
    vga_palette base;
    vga_palette current;
    damage_tracker dmg_base;
    damage_tracker dmg_previous;
    damage_tracker dmg_current;
    vga_remap_tables remaps;
    bool dirty_remaps;
    palette_transformer transformers[MAX_TRANSFORMER_COUNT];
    unsigned int transformer_count;
} vga_state;

static vga_state state;

void vga_state_init(void) {
    memset(&state, 0, sizeof(vga_state));
    damage_reset(&state.dmg_base);
    damage_reset(&state.dmg_previous);
    damage_reset(&state.dmg_current);
}

void vga_state_close(void) {
    vga_state_init();
}

void vga_state_render(void) {
    // We only want to render new state if something has changed. Otherwise, no-op.
    if(state.dmg_base.dirty || state.transformer_count) {
        // Copy base palette as the starting state, along with dirtiness data.
        memcpy(&state.current, &state.base, sizeof(vga_palette));
        damage_copy(&state.dmg_current, &state.dmg_base);
        damage_reset(&state.dmg_base);

        // Run transformers on top. These may modify the current palette and change dirtiness state.
        for(unsigned int i = 0; i < state.transformer_count; i++) {
            state.transformers[i].callback(&state.dmg_current, &state.current, state.transformers[i].userdata);
        }
        state.transformer_count = 0;
        damage_copy(&state.dmg_previous, &state.dmg_current);
    }
}

void vga_state_mark_palette_flushed(void) {
    damage_reset(&state.dmg_current);
    damage_reset(&state.dmg_previous);
}

void vga_state_mark_remaps_flushed(void) {
    state.dirty_remaps = false;
}

void vga_state_mul_base_palette(vga_index start, vga_index end, float multiplier) {
    vga_color *c;
    for(int i = start; i < end; i++) {
        c = &state.base.colors[i];
        c->r = multiplier * c->r;
        c->g = multiplier * c->g;
        c->b = multiplier * c->b;
    }
    damage_set_range(&state.dmg_base, start, end);
}

bool vga_state_is_palette_dirty(vga_palette **palette, vga_index *dirty_range_start, vga_index *dirty_range_end) {
    if(state.dmg_current.dirty) {
        *palette = &state.current;
        if(dirty_range_start != NULL) {
            *dirty_range_start = min2(state.dmg_current.dirty_range_start, state.dmg_previous.dirty_range_start);
        }
        if(dirty_range_end != NULL) {
            *dirty_range_end = max2(state.dmg_current.dirty_range_end, state.dmg_previous.dirty_range_end);
        }
        return true;
    }
    return false;
}

bool vga_state_is_remap_dirty(vga_remap_tables **remaps) {
    if(state.dirty_remaps) {
        *remaps = &state.remaps;
        return true;
    }
    return false;
}

void vga_state_set_remaps_from(const vga_remap_tables *src) {
    memcpy(&state.remaps, src, sizeof(vga_remap_tables));
    state.dirty_remaps = true;
}

void vga_state_copy_base_palette(vga_palette *dst) {
    memcpy(dst, &state.base, sizeof(vga_palette));
}

void vga_state_set_base_palette_from(const vga_palette *src) {
    memcpy(&state.base, src, sizeof(vga_palette));
    damage_set_range(&state.dmg_base, 0, 255);
}

void vga_state_set_base_palette_from_range(const vga_palette *src, vga_index dst_start, vga_index src_start,
                                           vga_index count) {
    memcpy(&state.base.colors[dst_start], &src->colors[src_start], count * 3);
    damage_set_range(&state.dmg_base, dst_start, dst_start + count);
}

void vga_state_set_base_palette_index(vga_index index, const vga_color *color) {
    state.base.colors[index] = *color;
    damage_set_range(&state.dmg_base, index, index);
}

void vga_state_set_base_palette_range(vga_index start, vga_index count, vga_color *src_colors) {
    assert(start + count < 256);
    memcpy(&state.base.colors[start], src_colors, count * 3);
    damage_set_range(&state.dmg_base, start, start + count);
}

void vga_state_copy_base_palette_range(vga_index dst, vga_index src, vga_index count) {
    memmove(&state.base.colors[dst], &state.base.colors[src], count * 3);
    damage_set_range(&state.dmg_base, dst, dst + count);
}

void vga_state_use_palette_transform(vga_palette_transform transform_callback, void *userdata) {
    state.transformers[state.transformer_count].callback = transform_callback;
    state.transformers[state.transformer_count].userdata = userdata;
    state.transformer_count++;
}
