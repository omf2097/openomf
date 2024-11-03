/*! \file
 * \brief Font file handling.
 * \details Functions and structs for reading, writing and modifying OMF:2097 font (GRAPHCHR.DAT, CHRSMAL.DAT) files.
 * \copyright MIT license.
 * \date 2013-2014
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */

#ifndef SD_FONTS_H
#define SD_FONTS_H

#include "formats/rgba_image.h"
#include "vga_image.h"
#include <stdint.h>

/*! \brief Font character
 *
 * A single font character. Simply contains the data. Representation depends on font size.
 */
typedef struct {
    char data[8]; ///< Font data
} sd_char;

/*! \brief Font container
 *
 * Contains all characters for the font. This is pretty much just a glorified list,
 * with a fixed size of 224 character entries.
 */
typedef struct {
    unsigned int h;     ///< Font height in pixels
    sd_char chars[224]; ///< Font characters
} sd_font;

/*! \brief Initialize font structure
 *
 * Initializes the font structure with empty values.
 *
 * \retval SD_INVALID_INPUT Font struct pointer was NULL
 * \retval SD_SUCCESS Success.
 *
 * \param font Allocated font struct pointer.
 */
int sd_font_create(sd_font *font);

/*! \brief Free font structure
 *
 * Frees up all memory reserved by the font structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * \param font font struct to modify.
 */
void sd_font_free(sd_font *font);

/*! \brief Load a font file
 *
 * Loads the given fontfile to memory. The structure must be initialized with sd_font_create()
 * before using this function. Loading to a previously loaded or filled sd_font structure
 * will result in old data and pointers getting lost. This is very likely to cause a memory leak.
 *
 * \retval SD_FILE_OPEN_ERROR File could not be opened.
 * \retval SD_SUCCESS Success.
 *
 * \param font Font struct pointer.
 * \param filename Name of the fontfile to load from.
 * \param font_h Font height. For CHRSMAL.DAT this is 6, for GRAPHCHR.DAT this is 8.
 */
int sd_font_load(sd_font *font, const char *filename, unsigned int font_h);

/*! \brief Save fontfile
 *
 * Saves the given fontfile from memory to a file on disk. The structure must be at
 * least initialized by using sd_font_create() before running this.
 *
 * \retval SD_FILE_OPEN_ERROR File could not be opened for writing.
 * \retval SD_SUCCESS Success.
 *
 * \param font Font struct pointer.
 * \param filename Name of the fontfile to save into.
 */
int sd_font_save(const sd_font *font, const char *filename);

/*! \brief Decodes a character to an image
 *
 * Decodes a character to an RGBA8888 Image. Opacity will automatically be
 * set to 100%. Note! Image surface MUST be allocated and right size before using this!
 *
 * \retval SD_INVALID_INPUT Font or image struct was NULL, or character index was >= 224.
 * \retval SD_SUCCESS Success.
 *
 * \param font Font struct pointer.
 * \param surface Image surface to save to. Must be pre-allocated!
 * \param ch Character to load. Must be 0 <= ch <= 224.
 * \param color Color palette index (0 - 0xFF)
 */
int sd_font_decode(const sd_font *font, sd_vga_image *surface, uint8_t ch, uint8_t color);

/**
 * Same as sd_font_decode, but decodes to an RGBA surface.
 */
int sd_font_decode_rgb(const sd_font *font, sd_rgba_image *o, uint8_t ch, uint8_t r, uint8_t g, uint8_t b);

#endif // SD_FONTS_H
