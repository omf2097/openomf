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
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Palette struct
 *
 * Contains the OMF:2097 palettes in converted RGB888 format (from RGB666).
 * Conversion is done automatically on palette load and save.
 */
typedef struct {
    unsigned char data[256][3];    ///< Palette data (256 indices, 3 channels/index)
    unsigned char remaps[19][256]; ///< Palette remapping tables.
} palette;

/*! \brief Initialize palette data structure
 *
 * Initializes the palette data structure with empty palette and remapping data.
 *
 * \retval SD_INVALID_INPUT Palette struct pointer was NULL
 * \retval SD_SUCCESS Success.
 *
 * \param pal Allocated palette data struct pointer.
 */
int palette_create(palette *pal);

/*! \brief Free REC file structure
 *
 * Frees up all memory reserved by the palette data structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * \param pal Palette data struct pointer.
 */
void palette_free(palette *pal);

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
unsigned char palette_resolve_color(uint8_t r, uint8_t g, uint8_t b, const palette *pal);

/*! \brief Exports palette to GIMP palette file.
 *
 * Exports a palette to GIMP palette format (GPL). Palette remappings are NOT
 * exported, only the 256 color palette.
 *
 * \retval SD_INVALID_INPUT Palette or filename ptr was NULL.
 * \retval SD_FILE_OPEN_ERROR File could not be opened for writing.
 * \retval SD_SUCCESS Success.
 *
 * \param pal Palette to export.
 * \param filename Name of the file to export to.
 */
int palette_to_gimp_palette(const palette *pal, const char *filename);

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
int palette_from_gimp_palette(palette *pal, const char *filename);

int palette_mload_range(memreader *reader, palette *pal, int index_start, int index_count);
int palette_load_range(sd_reader *reader, palette *pal, int index_start, int index_count);
int palette_load(sd_reader *reader, palette *pal);
void palette_msave_range(memwriter *writer, const palette *pal, int index_start, int index_count);
void palette_save_range(sd_writer *writer, const palette *pal, int index_start, int index_count);
void palette_save(sd_writer *writer, const palette *pal);
void palette_set_player_color(palette *pal, int player, int sourcecolor, int destcolor);
palette *palette_copy(palette *src);

#ifdef __cplusplus
}
#endif

#endif // PALETTE_H
