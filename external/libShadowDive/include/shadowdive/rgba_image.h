/*! \file
 * \brief RGBA image data handling.
 * \details Functions and structs for reading, writing and modifying RGBA image data.
 * \copyright MIT license.
 * \date 2013-2014
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */

#ifndef _SD_RGBA_IMAGE_H
#define _SD_RGBA_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief RGBA image
 *
 * A very simple RGBA8888 image. Data size is always w * h * 4 bytes.
 */
typedef struct {
    unsigned int w; ///< Image pixel width
    unsigned int h; ///< Image pixel height
    unsigned int len; ///< Image byte length
    char *data; ///< Image data
} sd_rgba_image;

/*! \brief Initialize RGBA image structure
 *
 * Initializes the RGBA image structure with empty values.
 *
 * \retval SD_INVALID_INPUT RGBA image struct pointer was NULL
 * \retval SD_SUCCESS Success.
 *
 * \param img Allocated RGBA image struct pointer.
 * \param w Image width in pixels
 * \param h Image height in pixels
 */
int sd_rgba_image_create(sd_rgba_image *img, unsigned int w, unsigned int h);

/*! \brief Copy RGBA image structure
 *
 * Copies the contents of an RGBA image structure. _ALL_ internals will be copied.
 * The copied structure must be freed using sd_rgba_image_free().
 *
 * Destination buffer does not need to be cleared. Source buffer must be a valid
 * RGBA image structure, or problems are likely to appear.
 *
 * \retval SD_OUT_OF_MEMORY Memory ran out. Destination struct should now be considered invalid and freed.
 * \retval SD_INVALID_INPUT Either input value was NULL.
 * \retval SD_SUCCESS Success.
 *
 * \param dst Destination RGBA image struct pointer.
 * \param src Source RGBA image struct pointer.
 */
int sd_rgba_image_copy(sd_rgba_image *dst, const sd_rgba_image *src);

/*! \brief Clear image with color
 *
 * Clears the RGBA image with given color.
 *
 * \retval SD_INVALID_INPUT Image pointer was NULL
 * \retval SD_SUCCESS All went as expected.
 *
 * \param img RGBA image struct to modify.
 * \param r Red channel value (0 - 0xFF)
 * \param g Green channel value (0 - 0xFF)
 * \param b Blue channel value (0 - 0xFF)
 * \param a Alpha channel value (0 - 0xFF)
 */
int sd_rgba_image_clear(sd_rgba_image *img, char r, char g, char b, char a);

/*! \brief Blit RGBA image to another image
 *
 * Does a direct image surface blit to given coordinates. No blending is done;
 * all image data is copied (including alpha channel). The whole input image
 * will be copied.
 *
 * Both images must be valid images.
 *
 * \retval SD_INVALID_INPUT Either input value was NULL.
 * \retval SD_SUCCESS Success.
 *
 * \param dst Destination RGBA image struct pointer.
 * \param src Source RGBA image struct pointer.
 * \param x Destination X coordinate
 * \param y Destination Y coordinate
 */
int sd_rgba_image_blit(sd_rgba_image *dst, const sd_rgba_image *src, int x, int y);

/*! \brief Save an RGBA image from a PNG file.
 *
 * Saves an RGBA image to a PNG file. Output PNG image format will be standard
 * RGBA format.
 *
 * \retval SD_INVALID_INPUT Image or filename was NULL
 * \retval SD_FILE_OPEN_ERROR File could not be opened for writing.
 * \retval SD_FORMAT_NOT_SUPPORTED File format (PNG) is not supported.
 * \retval SD_SUCCESS Success.
 *
 * \param img Source image pointer
 * \param filename Destination filename
 */
int sd_rgba_image_to_png(const sd_rgba_image *img, const char *filename);

/*! \brief Free RGBA image structure
 *
 * Frees up all memory reserved by the RGBA image structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * \param img RGBA image struct to modify.
 */
void sd_rgba_image_free(sd_rgba_image *img);

/*! \brief Save image to PPM file.
 *
 * Saves the RGBA image data to PPM file.
 *
 * \retval SD_FILE_OPEN_ERROR File could not be opened for writing.
 * \retval SD_SUCCESS Success.
 *
 * \param img Source image pointer
 * \param filename Destination filename
 */
int sd_rgba_image_to_ppm(const sd_rgba_image *img, const char *filename);

#ifdef __cplusplus
}
#endif

#endif // _SD_RGBA_IMAGE_H
