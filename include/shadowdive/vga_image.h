/*! \file 
 * \brief Functions for dealing with paletted image data.
 * \license MIT
 */ 

#ifndef _SD_VGA_IMAGE_H
#define _SD_VGA_IMAGE_H

#include "shadowdive/rgba_image.h"
#include "shadowdive/palette.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned int w; ///< Pixel width
    unsigned int h; ///< Pixel height
    unsigned int len; ///< Byte length
    char *data; ///< Palette representation of image data
    char *stencil; ///< holds 0 or 1 indicating whether a pixel is present
} sd_vga_image;

/*! \brief Initialize VGA image structure
 *
 * Initializes the VGA image structure with empty values.
 *
 * \retval SD_INVALID_INPUT VGA image struct pointer was NULL
 * \retval SD_SUCCESS Success.
 *
 * \param img Allocated VGA image struct pointer.
 */
int sd_vga_image_create(sd_vga_image *img, unsigned int w, unsigned int h);

/*! \brief Copy VGA image structure
 *
 * Copies the contents of an VGA image structure. _ALL_ internals will be copied.
 * The copied structure must be freed using sd_vga_image_free().
 *
 * Destination buffer does not need to be cleared. Source buffer must be a valid
 * VGA image structure, or problems are likely to appear.
 *
 * \retval SD_OUT_OF_MEMORY Memory ran out. Destination struct should now be considered invalid and freed.
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

/*! \brief Regenerates the stencil from a color index
 * 
 * This function regenerates the invisibility mask for the VGA image.
 * A necative value for stencil_index will lead to a completely opaque background,
 * while a value of 0-255 will create a stencil from colors of this index.
 *
 * \retval SD_INVALID_INPUT Bad index value or img was NULL
 * \retval SD_SUCCESS All good.
 *
 * \param img VGA image struct to modify.
 * \param stencil_index Color key to use for invisibility
 */
int sd_vga_image_stencil_index(sd_vga_image *img, int stencil_index);

/*! \brief Encode RGBA data to VGA data
 *
 * Encodes RGBA image data to VGA image data. Color values will be matched to exact values in
 * the palette. If no matching value is found for the pixel, the pixel color will be black.
 *
 * Note! The output VGA image will be created here. If the image had been
 * already created by using sd_vga_image_create() previously, there may
 * potentially be a memory leak, since the old image internals will not be freed.
 *
 * \retval SD_OUT_OF_MEMORY Memory ran out. Any output should be considered invalid and freed.
 * \retval SD_INVALID_INPUT Dst, src or palette was NULL.
 * \retval SD_SUCCESS Success. 
 *
 * \param dst Destination VGA image pointer.
 * \param src Source RGBA image pointer
 * \param pal Palette that should be used for the conversion
 * \param remapping Palette remapping table that should be used. -1 for none.
 */
int sd_vga_image_encode(
    sd_vga_image *dst,
    const sd_rgba_image *src,
    const sd_palette *pal,
    int remapping);

/*! \brief Decode VGA data to RGBA format
 *
 * Decodes the VGA image to RGBA image format.
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
 * \param src Source VGA image pointer
 * \param pal Palette that should be used for the conversion
 * \param remapping Palette remapping table that should be used. -1 for none.
 */
int sd_vga_image_decode(
    sd_rgba_image *dst,
    const sd_vga_image *src,
    const sd_palette *pal,
    int remapping);

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
 * \retval SD_INVALID_INPUT Image or filename was NULL
 * \retval SD_FILE_OPEN_ERROR File could not be opened for writing.
 * \retval SD_FORMAT_NOT_SUPPORTED File format (PNG) is not supported.
 * \retval SD_SUCCESS Success.
 *
 * \param img Source image pointer
 * \param pal Palette for the image
 * \param filename Destination filename
 */
int sd_vga_image_to_png(const sd_vga_image *img, const sd_palette *pal, const char *filename);

#ifdef __cplusplus
}
#endif

#endif // _SD_VGA_IMAGE_H
