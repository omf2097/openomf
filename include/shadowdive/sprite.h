/*! \file
 * \brief Sprite data handling.
 * \details Functions and structs for reading, writing and modifying OMF:2097 sprite data.
 * \copyright MIT license.
 * \date 2013-2014
 * \author huntercool
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */

#ifndef _SD_SPRITE_H
#define _SD_SPRITE_H

#include <stdint.h>
#include "shadowdive/vga_image.h"
#include "shadowdive/rgba_image.h"
#ifdef SD_USE_INTERNAL
    #include "shadowdive/internal/reader.h"
    #include "shadowdive/internal/writer.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Sprite image
 *
 * An encoded sprite image. The image is paletted, and encoded with
 * a type of RLE encoding. Decoding can be done to an RGBA image with
 * a palette, or directly to a VGA image. Sprite length varies depending on how many
 * "invisible" pixels it has.
 */
typedef struct {
    int16_t pos_x; ///< Position of sprite, X-axis
    int16_t pos_y; ///< Position of sprite, Y-axis
    uint8_t index; ///< Sprite index
    uint8_t missing; ///< Is sprite data missing? If this is 1, then data points to the data of another sprite.
    uint16_t width; ///< Pixel width of the sprite
    uint16_t height; ///< Pixel height of the sprite
    uint16_t len; ///< Byte length of the packed sprite data
    char *data; ///< Packed sprite data
} sd_sprite;

/*! \brief Initialize sprite structure
 *
 * Initializes the sprite structure with empty values.
 *
 * \retval SD_INVALID_INPUT Sprite struct pointer was NULL
 * \retval SD_SUCCESS Success.
 *
 * \param sprite Allocated sprite struct pointer.
 */
int sd_sprite_create(sd_sprite *sprite);

/*! \brief Copy sprite structure
 *
 * Copies the contents of an sprite structure. _ALL_ internals will be copied.
 * The copied structure must be freed using sd_sprite_free().
 *
 * Destination buffer does not need to be cleared. Source buffer must be a valid
 * sprite structure, or problems are likely to appear.
 *
 * \retval SD_OUT_OF_MEMORY Memory ran out. Destination struct should now be considered invalid and freed.
 * \retval SD_INVALID_INPUT Either input value was NULL.
 * \retval SD_SUCCESS Success.
 *
 * \param dst Destination sprite struct pointer.
 * \param src Source sprite struct pointer.
 */
int sd_sprite_copy(sd_sprite *dst, const sd_sprite *src);

/*! \brief Free sprite structure
 *
 * Frees up all memory reserved by the sprite structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * \param sprite Sprite struct to free.
 */
void sd_sprite_free(sd_sprite *sprite);

/*! \brief Encode RGBA data to sprite data
 *
 * Encodes RGBA image data to sprite image data. Color values will be matched to exact values in
 * the palette. If no matching value is found for the pixel, the pixel color will be black.
 *
 * \retval SD_OUT_OF_MEMORY Memory ran out. Any output should be considered invalid and freed.
 * \retval SD_INVALID_INPUT Dst, src or palette was NULL.
 * \retval SD_SUCCESS Success.
 *
 * \param dst Destination sprite struct pointer.
 * \param src Source RGBA image pointer
 * \param pal Palette that should be used for the conversion
 * \param remapping Palette remapping table that should be used. -1 for none.
 */
int sd_sprite_rgba_encode(
    sd_sprite *dst,
    const sd_rgba_image *src,
    const sd_palette *pal,
    int remapping);

/*! \brief Decode sprite data to RGBA format
 *
 * Decodes the sprite image to RGBA image format.
 *
 * Note! The output RGBA image will be created here. If the image had been
 * already created by using sd_rgba_image_create() previously, there may
 * potentially be a memory leak, since the old image internals will not be freed.
 *
 * \retval SD_OUT_OF_MEMORY Memory ran out. Any output should be considered invalid and freed.
 * \retval SD_INVALID_INPUT Dst, src or palette was NULL.
 * \retval SD_SUCCESS Success.
 *
 * \param dst Destination RGBA image struct pointer.
 * \param src Source Sprite image pointer
 * \param pal Palette that should be used for the conversion
 * \param remapping Palette remapping table that should be used. -1 for none.
 */
int sd_sprite_rgba_decode(
    sd_rgba_image *dst,
    const sd_sprite *src,
    const sd_palette *pal,
    int remapping);

/*! \brief Decode sprite to VGA image format.
 *
 * Decodes the sprite image to VGA image format.
 *
 * Note! The output VGA image will be created here. If the image had been
 * already created by using sd_vga_image_create() previously, there may
 * potentially be a memory leak, since the old image internals will not be freed.
 *
 * \retval SD_OUT_OF_MEMORY Memory ran out. Any output should be considered invalid and freed.
 * \retval SD_INVALID_INPUT Dst or src was NULL.
 * \retval SD_SUCCESS Success.
 *
 * \param dst Destination VGA image struct pointer.
 * \param src Source sprite image pointer
 */
int sd_sprite_vga_decode(
    sd_vga_image *dst,
    const sd_sprite *src);

/*! \brief Encode sprite from VGA image format.
 *
 * Encodes a VGA image to sprite format
 *
 * \retval SD_OUT_OF_MEMORY Memory ran out. Any output should be considered invalid and freed.
 * \retval SD_INVALID_INPUT Dst or src was NULL.
 * \retval SD_SUCCESS Success.
 *
 * \param dst Destination Sprite image struct pointer.
 * \param src Source VGA image pointer
 */
int sd_sprite_vga_encode(
    sd_sprite *dst,
    const sd_vga_image *src);

#ifdef SD_USE_INTERNAL
int sd_sprite_load(sd_reader *reader, sd_sprite *sprite);
int sd_sprite_save(sd_writer *writer, const sd_sprite *sprite);
#endif

#ifdef __cplusplus
}
#endif

#endif // _SD_SPRITE_H
