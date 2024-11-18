/*! \file
 * \brief VGA image data handling.
 * \details Functions and structs for reading, writing and modifying OMF:2097 VGA image data.
 * \copyright MIT license.
 * \date 2013-2014
 * \author huntercool
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */

#ifndef SD_VGA_IMAGE_H
#define SD_VGA_IMAGE_H

#include "formats/palette.h"
#include "formats/rgba_image.h"

/*! \brief VGA image structure
 *
 * Contains a paletted image. The image can be exported to
 * an omf:2097 sprite by using a proper function  sd_sprite_vga_encode() and back
 * sd_sprite_vga_decode() . For RGBA conversion, a valid palette is required.
 *
 * In VGA images, the len field should always be exactly w*h bytes long.
 */
typedef struct {
    unsigned int w;   ///< Pixel width
    unsigned int h;   ///< Pixel height
    unsigned int len; ///< Byte length
    char *data;       ///< Palette representation of image data
    int transparent;
} sd_vga_image;

/*! \brief Initialize VGA image structure
 *
 * Initializes the VGA image structure with empty values. By default, all pixels are
 * set as visible (opacity 100%).
 *
 * \retval SD_INVALID_INPUT VGA image struct pointer was NULL
 * \retval SD_SUCCESS Success.
 *
 * \param img Allocated VGA image struct pointer.
 * \param w Image width in pixels
 * \param h Image height in pixels
 */
int sd_vga_image_create(sd_vga_image *img, unsigned int w, unsigned int h, int transparent);

/*! \brief Copy VGA image structure
 *
 * Copies the contents of an VGA image structure. _ALL_ internals will be copied.
 * The copied structure must be freed using sd_vga_image_free().
 *
 * Destination buffer does not need to be cleared. Source buffer must be a valid
 * VGA image structure, or problems are likely to appear.
 *
 * \retval SD_INVALID_INPUT Either input value was NULL.
 * \retval SD_SUCCESS Success.
 *
 * \param dst Destination VGA image struct pointer.
 * \param src Source VGA image struct pointer.
 */
int sd_vga_image_copy(sd_vga_image *dst, const sd_vga_image *src);

/*! \brief Free VGA image structure
 *
 * Frees up all memory reserved by the VGA image structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * \param img VGA image struct to modify.
 */
void sd_vga_image_free(sd_vga_image *img);

/*! \brief Decode VGA data to RGBA format
 *
 * Decodes the VGA image to RGBA image format.
 *
 * Note! The output RGBA image will be created here. If the image had been
 * already created by using sd_rgba_image_create() previously, there may
 * potentially be a memory leak, since the old image internals will not be freed.
 *
 * \retval SD_INVALID_INPUT Dst, src or palette was NULL.
 * \retval SD_SUCCESS Success.
 *
 * \param dst Destination RGBA image struct pointer.
 * \param src Source VGA image pointer
 * \param pal Palette that should be used for the conversion
 */
int sd_vga_image_decode(sd_rgba_image *dst, const sd_vga_image *src, const vga_palette *pal);

/*! \brief Load an indexed image from a PNG file.
 *
 * Loads an indexed (paletted) image from a PNG file. Maximum allowed image
 * size is 320x200, and the smallest allowed size is 1x1.
 *
 * Note! The output vga image will be created here. If the image had been
 * already created by using sd_vga_image_create() previously, there may
 * potentially be a memory leak, since the old image internals will not be freed.
 *
 * \retval SD_INVALID_INPUT Image or filename was NULL
 * \retval SD_FILE_INVALID_TYPE Input image was of invalid type.
 * \retval SD_FILE_OPEN_ERROR File could not be opened for reading.
 * \retval SD_FORMAT_NOT_SUPPORTED File format (PNG) is not supported.
 * \retval SD_SUCCESS Success.
 *
 * \param img Destination image pointer
 * \param filename Source filename
 */
int sd_vga_image_from_png(sd_vga_image *img, const char *filename);

/*! \brief Save an indexed image from a PNG file.
 *
 * Saves an indexed (paletted) image to a PNG file. Maximum allowed image
 * size is 320x200, and the smallest allowed size is 1x1.
 *
 * \retval SD_OUT_OF_MEMORY Memory ran out. Any output should be considered invalid and freed.
 * \retval SD_INVALID_INPUT Image or filename was NULL
 * \retval SD_FILE_OPEN_ERROR File could not be opened for writing.
 * \retval SD_FORMAT_NOT_SUPPORTED File format (PNG) is not supported.
 * \retval SD_SUCCESS Success.
 *
 * \param img Source image pointer
 * \param pal Palette for the image
 * \param filename Destination filename
 */
int sd_vga_image_to_png(const sd_vga_image *img, const vga_palette *pal, const char *filename);

#endif // SD_VGA_IMAGE_H
