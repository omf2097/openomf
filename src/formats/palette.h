/**
 * @file palette.h
 * @brief Palette handling.
 * @details Functions and structs for reading, writing and modifying OMF:2097 palette data.
 * @copyright MIT License
 * @date 2013-2026
 * @author Andrew Thompson
 * @author Tuomas Virtanen
 */

#ifndef PALETTE_H
#define PALETTE_H

#include "formats/internal/memreader.h"
#include "formats/internal/memwriter.h"
#include "formats/internal/reader.h"
#include "formats/internal/writer.h"
#include "video/vga_palette.h"
#include "video/vga_remap.h"
#include <stdint.h>

/** @brief Resolves an RGB color to palette index
 *
 * Attempts to resolve an RGB color to a palette index. The requested RGB color
 * must exist on the palette; there is no lookup tolerance at all. If requested
 * color is not found, index 0 is returned.
 *
 * @param r Red color index (0 - 0xFF)
 * @param g Green color index (0 - 0xFF)
 * @param b Blue color index (0 - 0xFF)
 * @param pal Palette data struct pointer
 * @return Resolved color index
 */
unsigned char palette_resolve_color(uint8_t r, uint8_t g, uint8_t b, const vga_palette *pal);

/** @brief Resolves an RGB color to the closest palette index within a range
 *
 * Resolves an RGB color to a palette index, even if it is not a perfect match.
 * Only the given range is searched (inclusive).
 *
 * @param pal Palette data struct pointer
 * @param start First index of the search range
 * @param end Last index of the search range
 * @param r Red component of the reference color (0 - 0xFF)
 * @param g Green component of the reference color (0 - 0xFF)
 * @param b Blue component of the reference color (0 - 0xFF)
 * @return Index of the closest color in the range
 */
vga_index palette_resolve_closest(const vga_palette *pal, vga_index start, vga_index end, uint8_t r, uint8_t g,
                                  uint8_t b);

/** @brief Exports palette to GIMP palette file.
 *
 * Exports a palette to GIMP palette format (GPL).
 *
 * @retval SD_FILE_OPEN_ERROR File could not be opened for writing.
 * @retval SD_SUCCESS Success.
 *
 * @param pal Palette to export.
 * @param filename Name of the file to export to.
 */
int palette_to_gimp_palette(const vga_palette *pal, const path *filename);

/** @brief Imports palette from GIMP palette file.
 *
 * Imports a palette from GIMP palette format (GPL). The palette struct
 * must be initialized with palette_create before this.
 *
 * @retval SD_FILE_OPEN_ERROR File could not be opened for reading.
 * @retval SD_FILE_INVALID_TYPE File type was wrong.
 * @retval SD_SUCCESS Success.
 *
 * @param pal Palette to import to.
 * @param filename Name of the file to import from.
 */
int palette_from_gimp_palette(vga_palette *pal, const path *filename);

/** @brief Set the default menu colors on the base palette. */
void palette_set_menu_colors(void);

/** @brief Animate the pulsing menu color on the base palette.
 *
 * @param tick Current tick, used to pick the pulse phase.
 */
void palette_pulse_menu_colors(int tick);

/** @brief Load a range of palette colors from an open memory reader.
 *
 * @retval SD_SUCCESS Success.
 *
 * @param reader Open memory reader to read from.
 * @param pal Palette to load into.
 * @param index_start First color index to load.
 * @param index_count Number of colors to load.
 */
int palette_mload_range(memreader *reader, vga_palette *pal, int index_start, int index_count);

/** @brief Load a range of palette colors from an open reader.
 *
 * @retval SD_SUCCESS Success.
 *
 * @param reader Open reader to read from.
 * @param pal Palette to load into.
 * @param index_start First color index to load.
 * @param index_count Number of colors to load.
 */
int palette_load_range(sd_reader *reader, vga_palette *pal, int index_start, int index_count);

/** @brief Load a full 256-color palette from an open reader.
 *
 * @retval SD_SUCCESS Success.
 *
 * @param reader Open reader to read from.
 * @param pal Palette to load into.
 */
int palette_load(sd_reader *reader, vga_palette *pal);

/** @brief Load palette remap tables from an open reader.
 *
 * @retval SD_SUCCESS Success.
 *
 * @param reader Open reader to read from.
 * @param remaps Remap tables to load into.
 */
int palette_remaps_load(sd_reader *reader, vga_remap_tables *remaps);

/** @brief Save a range of palette colors to an open memory writer.
 *
 * @param writer Open memory writer to write to.
 * @param pal Palette to save from.
 * @param index_start First color index to save.
 * @param index_count Number of colors to save.
 */
void palette_msave_range(memwriter *writer, const vga_palette *pal, int index_start, int index_count);

/** @brief Save a range of palette colors to an open writer.
 *
 * @param writer Open writer to write to.
 * @param pal Palette to save from.
 * @param index_start First color index to save.
 * @param index_count Number of colors to save.
 */
void palette_save_range(sd_writer *writer, const vga_palette *pal, int index_start, int index_count);

/** @brief Save a full 256-color palette to an open writer.
 *
 * @param writer Open writer to write to.
 * @param pal Palette to save from.
 */
void palette_save(sd_writer *writer, const vga_palette *pal);

/** @brief Save palette remap tables to an open writer.
 *
 * @param writer Open writer to write to.
 * @param remaps Remap tables to save from.
 */
void palette_remaps_save(sd_writer *writer, const vga_remap_tables *remaps);

/** @brief Load a player's HAR colors from a palette into the base palette.
 *
 * @param src Source palette.
 * @param player Player index.
 */
void palette_load_player_colors(const vga_palette *src, int player);

/** @brief Copy a 16-color alternate-palette block into a player color slot.
 *
 * @param dst Destination palette.
 * @param player Player index.
 * @param src_color Source color block index.
 * @param dst_color Destination color block index.
 */
void palette_load_altpal_player_color(vga_palette *dst, int player, int src_color, int dst_color);

/** @brief Set a player color block on the base palette.
 *
 * @param player Player index.
 * @param src_color Source color block index.
 * @param dst_color Destination color block index.
 */
void palette_set_player_color(int player, int src_color, int dst_color);

/** @brief Expand a player's color block across the palette.
 *
 * @param pal Palette to modify.
 */
void palette_set_player_expanded_color(vga_palette *pal);

/** @brief Copy a range of colors from one palette to another.
 *
 * @param dst Destination palette.
 * @param src Source palette.
 * @param index_start First color index to copy.
 * @param index_count Number of colors to copy.
 */
void palette_copy(vga_palette *dst, const vga_palette *src, int index_start, int index_count);

#endif // PALETTE_H
