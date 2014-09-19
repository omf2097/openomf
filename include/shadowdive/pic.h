/*! \file
 * \brief Player image file handling.
 * \details Functions and structs for reading, writing and modifying OMF:2097 player image (PIC) files.
 * \copyright MIT license.
 * \date 2013-2014
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */

#ifndef _SD_PIC_H
#define _SD_PIC_H

#include "shadowdive/sprite.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PIC_PHOTOS 256 ///< Maximum amount of PIC photos that will fit the array

/*! \brief PIC pilot portrait
 *
 * Contains a pilot face portrait. There is still some unknown data here.
 * \todo find out what the unk_flag field is.
 */
typedef struct {
    int is_player;     ///< Is a player ? 1 = yes, 0 = no.
    int sex;           ///< Sex of person in photo. 1 = Female, 0 = Male.
    sd_palette pal;    ///< Image palette.
    sd_sprite *sprite; ///< Photo sprite
    uint8_t unk_flag;  ///< Unknown flag.
} sd_pic_photo;

/*! \brief PIC pilot portrait list
 *
 * Contains a list of pilot face portraits.
 */
typedef struct {
    int photo_count;   ///< Photo count
    sd_pic_photo *photos[MAX_PIC_PHOTOS]; ///< Photo array
} sd_pic_file;

/*! \brief Initialize PIC file structure
 *
 * Initializes the PIC file structure with empty values.
 *
 * \retval SD_INVALID_INPUT PIC struct pointer was NULL
 * \retval SD_SUCCESS Success.
 *
 * \param pic Allocated PIC struct pointer.
 */
int sd_pic_create(sd_pic_file *pic);

/*! \brief Free PIC file structure
 *
 * Frees up all memory reserved by the PIC structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * \param pic PIC file struct pointer.
 */
void sd_pic_free(sd_pic_file *pic);

/*! \brief Load a PIC file
 *
 * Loads the given PIC file to memory. The structure must be initialized with sd_pic_create()
 * before using this function. Loading to a previously loaded or filled sd_pic_file structure
 * will result in old data and pointers getting lost. This is very likely to cause a memory leak.
 *
 * \retval SD_FILE_OPEN_ERROR File could not be opened.
 * \retval SD_OUT_OF_MEMORY Memory ran out. This struct should now be considered invalid and freed.
 * \retval SD_SUCCESS Success.
 *
 * \param pic PIC file struct pointer.
 * \param filename Name of the PIC file to load from.
 */
int sd_pic_load(sd_pic_file *pic, const char *filename);

/*! \brief Save PIC file
 *
 * Saves the given PIC file from memory to a file on disk. The structure must be at
 * least initialized by using sd_pic_create() before running this.
 *
 * \retval SD_FILE_OPEN_ERROR File could not be opened for writing.
 * \retval SD_SUCCESS Success.
 *
 * \param pic PIC file struct pointer.
 * \param filename Name of the PIC file to save into.
 */
int sd_pic_save(const sd_pic_file *pic, const char *filename);

/*! \brief Returns a PIC image entry.
 *
 * Returns a pointer to a PIC image entry.
 *
 * The structure memory will be owned by the library; do not attempt to
 * free it.
 *
 * \retval NULL If PIC ptr was NULL or image entry does not exist.
 * \retval sd_pic_photo* Photo image struct pointer on success.
 *
 * \param pic PIC file struct pointer.
 * \param entry_id Photo picture number to get.
 */
const sd_pic_photo* sd_pic_get(const sd_pic_file *pic, int entry_id);

#ifdef __cplusplus
}
#endif

#endif // _SD_PIC_H
