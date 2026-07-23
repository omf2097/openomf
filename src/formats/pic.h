/**
 * @file pic.h
 * @brief Player image file handling.
 * @details Functions and structs for reading, writing and modifying OMF:2097 player image (PIC) files.
 * @copyright MIT License
 * @date 2013-2026
 * @author Andrew Thompson
 * @author Tuomas Virtanen
 */

#ifndef SD_PIC_H
#define SD_PIC_H

#include "formats/sprite.h"
#include "utils/vector.h"

#define MAX_PIC_PHOTOS 256 ///< Maximum amount of PIC photos that will fit the array

/** @brief PIC pilot portrait
 *
 * Contains a pilot face portrait. There is still some unknown data here.
 */
typedef struct {
    int is_player;     ///< Is a player ? 1 = yes, 0 = no.
    int sex;           ///< Sex of person in photo. 1 = Female, 0 = Male.
    vga_palette pal;   ///< Image palette.
    sd_sprite *sprite; ///< Photo sprite
    uint8_t has_photo; ///< Nonzero if photo sprite data follows.
} sd_pic_photo;

/** @brief PIC pilot portrait list
 *
 * Contains a list of pilot face portraits.
 */
typedef struct {
    vector photos; ///< Photo list, holds sd_pic_photo elements
} sd_pic_file;

/** @brief Initialize PIC file structure
 *
 * Initializes the PIC file structure with empty values.
 *
 * @param pic Allocated PIC struct pointer.
 */
void sd_pic_create(sd_pic_file *pic);

/** @brief Free PIC file structure
 *
 * Frees up all memory reserved by the PIC structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * @param pic PIC file struct pointer.
 */
void sd_pic_free(sd_pic_file *pic);

/** @brief Load a PIC file
 *
 * Loads the given PIC file to memory. The structure must be initialized with sd_pic_create()
 * before using this function. Loading to a previously loaded or filled sd_pic_file structure
 * will result in old data and pointers getting lost. This is very likely to cause a memory leak.
 *
 * @retval SD_FILE_OPEN_ERROR File could not be opened.
 * @retval SD_SUCCESS Success.
 *
 * @param pic PIC file struct pointer.
 * @param filename Name of the PIC file to load from.
 */
int sd_pic_load(sd_pic_file *pic, const path *filename);

/** @brief Save PIC file
 *
 * Saves the given PIC file from memory to a file on disk. The structure must be at
 * least initialized by using sd_pic_create() before running this.
 *
 * @retval SD_FILE_OPEN_ERROR File could not be opened for writing.
 * @retval SD_SUCCESS Success.
 *
 * @param pic PIC file struct pointer.
 * @param filename Name of the PIC file to save into.
 */
int sd_pic_save(const sd_pic_file *pic, const path *filename);

/** @brief Returns a PIC image entry.
 *
 * Returns a pointer to a PIC image entry.
 *
 * The structure memory will be owned by the library; do not attempt to
 * free it.
 *
 * @retval NULL Entry id is out of range or the image does not exist.
 * @retval sd_pic_photo* Photo image struct pointer on success.
 *
 * @param pic PIC file struct pointer.
 * @param entry_id Photo picture number to get.
 */
const sd_pic_photo *sd_pic_get(const sd_pic_file *pic, int entry_id);

#endif // SD_PIC_H
