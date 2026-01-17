/**
 * @file vga_state.h
 * @brief VGA palette state management
 */

#ifndef VGA_STATE_H
#define VGA_STATE_H

#include "utils/path.h"
#include "video/damage_tracker.h"
#include "video/vga_palette.h"
#include "video/vga_remap.h"
#include <stdbool.h>

/**
 * @brief Callback type for palette transformation
 * @param damage Damage tracker to mark modified palette ranges
 * @param palette Palette to transform
 * @param userdata User-provided context data
 */
typedef void (*vga_palette_transform)(damage_tracker *damage, vga_palette *palette, void *userdata);

/**
 * @brief Initialize the VGA state subsystem
 */
void vga_state_init(void);

/**
 * @brief Close the VGA state subsystem and reset state
 */
void vga_state_close(void);

/**
 * @brief Render the current palette state
 *
 * Applies base palette and any registered transformers to produce
 * the final current palette.
 */
void vga_state_render(void);

/**
 * @brief Mark the palette as flushed to the renderer
 */
void vga_state_mark_palette_flushed(void);

/**
 * @brief Mark the remap tables as flushed to the renderer
 */
void vga_state_mark_remaps_flushed(void);

/**
 * @brief Mark all palette and remap data as dirty
 */
void vga_state_mark_dirty(void);

/**
 * @brief Check if the palette has been modified
 * @param palette Output for pointer to current palette
 * @param dirty_range_first Output for first dirty index (can be NULL)
 * @param dirty_range_last Output for last dirty index (can be NULL)
 * @return true if palette is dirty, false otherwise
 */
bool vga_state_is_palette_dirty(vga_palette **palette, vga_index *dirty_range_first, vga_index *dirty_range_last);

/**
 * @brief Check if the remap tables have been modified
 * @param remaps Output for pointer to remap tables
 * @return true if remaps are dirty, false otherwise
 */
bool vga_state_is_remap_dirty(vga_remap_tables **remaps);

/**
 * @brief Push the current base palette onto the stash
 */
void vga_state_push_palette(void);

/**
 * @brief Pop the stashed palette back to base palette
 */
void vga_state_pop_palette(void);

/**
 * @brief Multiply palette colors by a factor
 * @param start Starting palette index
 * @param end Ending palette index (exclusive)
 * @param multiplier Factor to multiply by (0.0 to 1.0)
 */
void vga_state_mul_base_palette(vga_index start, vga_index end, float multiplier);

/**
 * @brief Set remap tables from source
 * @param src Source remap tables to copy from
 */
void vga_state_set_remaps_from(const vga_remap_tables *src);

/**
 * @brief Set the entire base palette from source
 * @param src Source palette to copy from
 */
void vga_state_set_base_palette_from(const vga_palette *src);

/**
 * @brief Copy a range of colors from source palette to base palette
 * @param src Source palette
 * @param dst_start Destination start index in base palette
 * @param src_start Source start index in source palette
 * @param count Number of colors to copy
 */
void vga_state_set_base_palette_from_range(const vga_palette *src, vga_index dst_start, vga_index src_start,
                                           vga_index count);

/**
 * @brief Set a single color in the base palette
 * @param index Palette index to set
 * @param color Color value to set
 */
void vga_state_set_base_palette_index(vga_index index, const vga_color *color);

/**
 * @brief Set a range of colors in the base palette
 * @param start Starting palette index
 * @param count Number of colors to set
 * @param src_colors Array of colors to copy
 */
void vga_state_set_base_palette_range(vga_index start, vga_index count, vga_color *src_colors);

/**
 * @brief Copy a range of colors within the base palette
 * @param dst Destination start index
 * @param src Source start index
 * @param count Number of colors to copy
 */
void vga_state_copy_base_palette_range(vga_index dst, vga_index src, vga_index count);

/**
 * @brief Register a palette transform callback for the current frame
 * @param transform_callback Transform function to apply
 * @param userdata User data to pass to the callback
 */
void vga_state_enable_palette_transform(vga_palette_transform transform_callback, void *userdata);

/**
 * @brief Write a debug screenshot of the current palette
 * @param filename Output file path
 */
void vga_state_debug_screenshot(const path *filename);

#endif // VGA_STATE_H
