#ifndef VGA_STATE_H
#define VGA_STATE_H

#include "video/damage_tracker.h"
#include "video/vga_palette.h"
#include "video/vga_remap.h"
#include <stdbool.h>

typedef void (*vga_palette_transform)(damage_tracker *damage, vga_palette *palette, void *userdata);

void vga_state_init(void);
void vga_state_close(void);
void vga_state_render(void);

void vga_state_mark_palette_flushed(void);
void vga_state_mark_remaps_flushed(void);

bool vga_state_is_palette_dirty(vga_palette **palette, vga_index *dirty_range_start, vga_index *dirty_range_end);
bool vga_state_is_remap_dirty(vga_remap_tables **remaps);

/**
 * Copies current base palette to stash.
 */
void vga_state_push_palette(void);

/**
 * Recover base palette from stash. Replaces current base palette.
 */
void vga_state_pop_palette(void);

void vga_state_mul_base_palette(vga_index start, vga_index end, float multiplier);
void vga_state_set_remaps_from(const vga_remap_tables *src);
void vga_state_set_base_palette_from(const vga_palette *src);
void vga_state_set_base_palette_from_range(const vga_palette *src, vga_index dst_start, vga_index src_start,
                                           vga_index count);
void vga_state_set_base_palette_index(vga_index index, const vga_color *color);
void vga_state_set_base_palette_range(vga_index start, vga_index count, vga_color *src_colors);
void vga_state_copy_base_palette_range(vga_index dst, vga_index src, vga_index count);

void vga_state_use_palette_transform(vga_palette_transform transform_callback, void *userdata);

#endif // VGA_STATE_H
