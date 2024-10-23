#include "video/vga_state.h"

#include <string.h>

static vga_state state;

void vga_state_init(void) {
    memset(&state, 0, sizeof(vga_state));
}

void vga_state_close(void) {
    vga_state_init();
}

bool vga_is_dirty_palette(vga_index *dirty_range_start, vga_index *dirty_range_end) {
    if (state.dirty_palette) {
        *dirty_range_start = state.dirty_range_start;
        *dirty_range_end = state.dirty_range_end;
        return true;
    }
    return false;
}

bool vga_is_dirty_remaps(void) {
    return state.dirty_remaps;
}

void vga_set_initial_state(const palette *src) {
    memcpy(&state.initial_palette, src->data, 256 * 3);
    memcpy(&state.remaps, src->remaps, 256 * VGA_REMAP_COUNT);
}

void vga_set_palette_index(vga_index index, vga_color color) {
    state.initial_palette.colors[index] = color;
}

void vga_set_palette_indices(vga_index start, vga_index count, vga_color *src_colors) {
    for (int i = 0; i < count; i++) {
        state.initial_palette.colors[start + i] = src_colors[i];
    }
}

void vga_apply_palette_transform(vga_palette_transform transform_callback, void *userdata) {

}
