/**
 * @file pcx.h
 * @brief PCX image file handling.
 * @details Functions and structs for reading PCX images and PCX-based font sheets.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef PCX_H
#define PCX_H

#include "formats/vga_image.h"
#include <stdint.h>

/** @brief PCX image file */
typedef struct {
    uint8_t manufacturer;
    uint8_t version;
    uint8_t encoding;
    uint8_t bits_per_plane;

    uint16_t window_x_min;
    uint16_t window_y_min;
    uint16_t window_x_max;
    uint16_t window_y_max;

    uint16_t horz_dpi;
    uint16_t vert_dpi;

    uint8_t header_palette[48];
    uint8_t reserved;
    uint8_t color_planes;

    uint16_t bytes_per_plane_line;
    uint16_t palette_info;

    uint16_t hor_scr_size;
    uint16_t ver_scr_size;

    // After the headers here, there is 54 bytes of padding.

    sd_vga_image image;
    vga_palette palette;
} pcx_file;

/** @brief PCX font glyph location */
typedef struct {
    uint16_t x;
    uint8_t y;
    uint8_t width;
} pcx_font_glyph;

/** @brief PCX font sheet */
typedef struct {
    pcx_file pcx;
    uint8_t glyph_height;
    uint8_t glyph_count;
    pcx_font_glyph glyphs[256];
} pcx_font;

/** @brief Load a PCX image file
 *
 * @retval SD_FILE_INVALID_TYPE File is not a valid PCX.
 * @retval SD_FILE_READ_ERROR File could not be read.
 * @retval SD_SUCCESS Success.
 *
 * @param pcx PCX struct to fill.
 * @param filename Name of the PCX file to load from.
 */
int pcx_load(pcx_file *pcx, const path *filename);

/** @brief Load a PCX font sheet
 *
 * @retval SD_FILE_INVALID_TYPE File is not a valid PCX.
 * @retval SD_FORMAT_NOT_SUPPORTED Image is not a valid font sheet.
 * @retval SD_SUCCESS Success.
 *
 * @param font PCX font struct to fill.
 * @param filename Name of the PCX file to load from.
 */
int pcx_load_font(pcx_font *font, const path *filename);

/** @brief Decode a font sheet glyph to a VGA image
 *
 * @retval SD_INVALID_INPUT Glyph index is out of range.
 * @retval SD_SUCCESS Success.
 *
 * @param font PCX font struct pointer.
 * @param o Destination VGA image. Must be pre-allocated.
 * @param ch Glyph index to decode.
 * @param palette_offset Offset added to each palette index.
 */
int pcx_font_decode(const pcx_font *font, sd_vga_image *o, uint8_t ch, int8_t palette_offset);

/** @brief Free a PCX image struct.
 *
 * @param pcx PCX struct to free.
 */
void pcx_free(pcx_file *pcx);

/** @brief Free a PCX font struct.
 *
 * @param font PCX font struct to free.
 */
void pcx_font_free(pcx_font *font);

#endif // PCX_H
