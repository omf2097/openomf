/*! \file 
 * \brief Functions for dealing with RGBA image data.
 * \license MIT
 */ 

#ifndef _SD_RGBA_IMAGE_H
#define _SD_RGBA_IMAGE_H

#ifdef __cplusplus 
extern "C" {
#endif

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
