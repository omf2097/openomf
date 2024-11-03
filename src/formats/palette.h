/*! \file
 * \brief Palette handling.
 * \details Functions and structs for reading, writing and modifying OMF:2097 palette data.
 * \copyright MIT license.
 * \date 2013-2014
 * \author Andrew Thompson
 * \author Tuomas Virtanen
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

/*! \brief Resolves an RGB color to palette index
 *
 * Attempts to resolve an RGB color to a palette index. The requested RGB color
 * must exist on the palette; there is no lookup tolerance at all. If requested
 * color is not found, index 0 is returned.
 *
 * \param r Red color index (0 - 0xFF)
 * \param g Green color index (0 - 0xFF)
 * \param b Blue color index (0 - 0xFF)
 * \param pal Palette data struct pointer
 * \return Resolved color index
 */
unsigned char palette_resolve_color(uint8_t r, uint8_t g, uint8_t b, const vga_palette *pal);

/*! \brief Exports palette to GIMP palette file.
 *
 * Exports a palette to GIMP palette format (GPL).
 *
 * \retval SD_INVALID_INPUT Palette or filename ptr was NULL.
 * \retval SD_FILE_OPEN_ERROR File could not be opened for writing.
 * \retval SD_SUCCESS Success.
 *
 * \param pal Palette to export.
 * \param filename Name of the file to export to.
 */
int palette_to_gimp_palette(const vga_palette *pal, const char *filename);

/*! \brief Imports palette from GIMP palette file.
 *
 * Imports a palette from GIMP palette format (GPL). The palette struct
 * must be initialized with palette_create before this.
 *
 * \retval SD_INVALID_INPUT Palette or filename ptr was NULL.
 * \retval SD_FILE_OPEN_ERROR File could not be opened for reading.
 * \retval SD_FILE_INVALID_TYPE File type was wrong.
 * \retval SD_SUCCESS Success.
 *
 * \param palette Palette to import to.
 * \param filename Name of the file to import from.
 */
int palette_from_gimp_palette(vga_palette *pal, const char *filename);

void palette_set_menu_colors(void);
void palette_pulse_menu_colors(int tick);
int palette_mload_range(memreader *reader, vga_palette *pal, int index_start, int index_count);
int palette_load_range(sd_reader *reader, vga_palette *pal, int index_start, int index_count);
int palette_load(sd_reader *reader, vga_palette *pal);
int palette_remaps_load(sd_reader *reader, vga_remap_tables *remaps);
void palette_msave_range(memwriter *writer, const vga_palette *pal, int index_start, int index_count);
void palette_save_range(sd_writer *writer, const vga_palette *pal, int index_start, int index_count);
void palette_save(sd_writer *writer, const vga_palette *pal);
void palette_remaps_save(sd_writer *writer, const vga_remap_tables *remaps);
void palette_load_player_colors(vga_palette *src, int player);
void palette_load_player_cutscene_colors(vga_palette *src);
void palette_load_altpal_player_color(vga_palette *dst, int player, int src_color, int dst_color);
void palette_set_player_color(int player, int src_color, int dst_color);
void palette_copy(vga_palette *dst, const vga_palette *src, int index_start, int index_count);

#endif // PALETTE_H
