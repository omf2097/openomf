/*! \file
 * \brief Palette handling.
 * \details Functions and structs for reading, writing and modifying OMF:2097 palette data.
 * \copyright MIT license.
 * \date 2013-2014
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */

#ifndef _SD_PALETTE_H
#define _SD_PALETTE_H

#include <stdint.h>
#ifdef SD_USE_INTERNAL
    #include "shadowdive/internal/memreader.h"
    #include "shadowdive/internal/reader.h"
    #include "shadowdive/internal/memwriter.h"
    #include "shadowdive/internal/writer.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Palette struct
 *
 * Contains the OMF:2097 palettes in converted RGB888 format (from RGB666).
 * Conversion is done automatically on palette load and save.
 */
typedef struct {
    unsigned char data[256][3];   ///< Palette data (256 indices, 3 channels/index)
    unsigned char remaps[19][256]; ///< Palette remapping tables.
} sd_palette;

/*! \brief Initialize palette data structure
 *
 * Initializes the palette data structure with empty palette and remapping data.
 *
 * \retval SD_INVALID_INPUT Palette struct pointer was NULL
 * \retval SD_SUCCESS Success.
 *
 * \param pal Allocated palette data struct pointer.
 */
int sd_palette_create(sd_palette *pal);

/*! \brief Free REC file structure
 *
 * Frees up all memory reserved by the palette data structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * \param pal Palette data struct pointer.
 */
void sd_palette_free(sd_palette *pal);

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
unsigned char sd_palette_resolve_color(uint8_t r, uint8_t g, uint8_t b, const sd_palette *pal);

/*! \brief Exports palette to GIMP palette file.
 *
 * Exports an sd_palette to GIMP palette format (GPL). Palette remappings are NOT
 * exported, only the 256 color palette.
 *
 * \retval SD_INVALID_INPUT Palette or filename ptr was NULL.
 * \retval SD_FILE_OPEN_ERROR File could not be opened for writing.
 * \retval SD_SUCCESS Success.
 *
 * \param palette Palette to export.
 * \param filename Name of the file to export to.
 */
int sd_palette_to_gimp_palette(const sd_palette *palette, const char *filename);

/*! \brief Imports palette from GIMP palette file.
 *
 * Imports an sd_palette from GIMP palette format (GPL). The sd_palette struct
 * must be initialized with sd_palette_create before this.
 *
 * \retval SD_INVALID_INPUT Palette or filename ptr was NULL.
 * \retval SD_FILE_OPEN_ERROR File could not be opened for reading.
 * \retval SD_FILE_INVALID_TYPE File type was wrong.
 * \retval SD_SUCCESS Success.
 *
 * \param palette Palette to import to.
 * \param filename Name of the file to import from.
 */
int sd_palette_from_gimp_palette(sd_palette *palette, const char *filename);

#ifdef SD_USE_INTERNAL
int sd_palette_mload_range(sd_mreader *reader, sd_palette *palette, int index_start, int index_count);
int sd_palette_load_range(sd_reader *reader, sd_palette *palette, int index_start, int index_count);
int sd_palette_load(sd_reader *reader, sd_palette *palette);
void sd_palette_msave_range(sd_mwriter *writer, const sd_palette *palette, int index_start, int index_count);
void sd_palette_save_range(sd_writer *writer, const sd_palette *palette, int index_start, int index_count);
void sd_palette_save(sd_writer *writer, const sd_palette *palette);
#endif

#ifdef __cplusplus
}
#endif

#endif // _SD_PALETTE_H
