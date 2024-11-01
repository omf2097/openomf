/*! \file
 * \brief Scene file handling.
 * \details Functions and structs for reading, writing and modifying OMF:2097 scene (BK) files.
 * \copyright MIT license.
 * \date 2013-2014
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */

#ifndef SD_BK_H
#define SD_BK_H

#include "formats/bkanim.h"
#include "formats/palette.h"
#include "formats/vga_image.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_BK_ANIMS 50   ///< Amount of animations in the BK file. This is fixed!
#define MAX_BK_PALETTES 8 ///< Maximum amount of palettes allowed in BK file.

/*! \brief BK file information
 *
 * Contains information about an OMF:2097 scene. Eg. arenas, menus, intro, etc.
 */
typedef struct {
    uint32_t file_id;      ///< File ID
    uint8_t unknown_a;     ///< Unknown value
    uint8_t palette_count; ///< Number of palettes in the BK file

    sd_bk_anim *anims[MAX_BK_ANIMS];           ///< All animations contained by the BK file
    sd_vga_image *background;                  ///< Background image. If NULL, a black background will be used.
    vga_palette *palettes[MAX_BK_PALETTES];    ///< All palettes in the BK file.
    vga_remap_tables *remaps[MAX_BK_PALETTES]; ///< Remappings for the palettes

    char soundtable[30]; ///< All sounds used by the animations in this BK file.
} sd_bk_file;

/*! \brief Initialize BK file structure
 *
 * Initializes the BK file structure with empty values.
 *
 * \retval SD_INVALID_INPUT BK struct pointer was NULL
 * \retval SD_SUCCESS Success.
 *
 * \param bk Allocated BK struct pointer.
 */
int sd_bk_create(sd_bk_file *bk);

/*! \brief Copy BK structure
 *
 * Copies the contents of an BK file structure. _ALL_ internals will be copied.
 * The copied structure must be freed using sd_bk_free().
 *
 * Destination buffer does not need to be cleared. Source buffer must be a valid
 * BK file structure, or problems are likely to appear.
 *
 * \retval SD_INVALID_INPUT Either of the input pointers was NULL.
 * \retval SD_SUCCESS Success.
 *
 * \param dst Destination BK struct pointer.
 * \param src Source BK struct pointer.
 */
int sd_bk_copy(sd_bk_file *dst, const sd_bk_file *src);

/*! \brief Set background image
 *
 * Sets the background image of the BK file. If this is not used to a
 * new file, a black background will be used.
 *
 * Image data will be copied. Make sure to free your local copy yourself.
 *
 * A NULL value for image data means that background will be removed.
 *
 * \retval SD_INVALID_INPUT BK struct was NULL.
 * \retval SD_SUCCESS Success.
 *
 * \param bk BK struct pointer.
 * \param img VGA image data struct.
 */
int sd_bk_set_background(sd_bk_file *bk, const sd_vga_image *img);

/*! \brief Get background image
 *
 * Returns a pointer to the background image data.
 *
 * \retval NULL Background does not exist
 * \retval sd_vga_image* Background image.
 *
 * \param bk BK struct pointer.
 */
sd_vga_image *sd_bk_get_background(const sd_bk_file *bk);

/*! \brief Set bk animation
 *
 * Sets a BK animation in BK file structure. Index must be between 0 and 49 (inclusive);
 * All other index values will result in error.
 *
 * BK animation data will be copied. Make sure to free your local copy yourself.
 * Old data at index will be freed automatically.
 *
 * Animation input value of NULL will mean that data at index will be freed!
 *
 * \retval SD_INVALID_INPUT index value was invalid or bk struct was NULL.
 * \retval SD_SUCCESS Success.
 *
 * \param bk BK struct pointer.
 * \param index Animation index. Must be 0 <= index <= 49
 * \param anim Animation pointer. NULL here means that index will be cleared.
 */
int sd_bk_set_anim(sd_bk_file *bk, int index, const sd_bk_anim *anim);

/*! \brief Get BK animation
 *
 * Returns a pointer to a bk animation data at given index. Index must be between
 * 0 and 49 (inclusive); any other value will return NULL.
 *
 * \retval NULL No BK animation at index, or index is otherwise invalid.
 * \retval sd_bk_anim* Success.
 *
 * \param bk BK struct pointer.
 * \param index Animation index. Must be 0 <= index <= 49
 */
sd_bk_anim *sd_bk_get_anim(const sd_bk_file *bk, int index);

/*! \brief Set palette
 *
 * Sets a palette to index in BK file structure. You can only replace already set
 * palettes by using this function, so index must be between 0 and palette_count.
 * For adding new palettes, please see sd_bk_push_palette().
 *
 * Palette data will be copied. Make sure to free your local copy yourself.
 * Old data in index will be freed automatically.
 *
 * \retval SD_INVALID_INPUT Index was invalid or some input pointer was NULL.
 * \retval SD_SUCCESS Success.
 *
 * \param bk BK struct pointer.
 * \param index Palette index.
 * \param pal A Valid palette object pointer.
 */
int sd_bk_set_palette(sd_bk_file *bk, int index, const vga_palette *pal);

/*! \brief Push palette
 *
 * Pushes a palette to the end of the palette list. If list is full,
 * error value SD_INVALID_INPUT will be returned.
 *
 * Palette data will be copied. Make sure to free your local copy yourself.
 *
 * \retval SD_INVALID_INPUT Palette list is already full.
 * \retval SD_SUCCESS Success.
 *
 * \param bk BK struct pointer.
 * \param pal A Valid palette object pointer.
 */
int sd_bk_push_palette(sd_bk_file *bk, const vga_palette *pal);

/*! \brief Pop palette
 *
 * Pops a palette from the end of the palette list. If palette list is already empty,
 * error will be returned and nothing will happen.
 *
 * Popped palette data will be freed automatically.
 *
 * \retval SD_INVALID_INPUT There was nothing to pop.
 * \retval SD_SUCCESS Success.
 *
 * \param bk BK struct pointer.
 */
int sd_bk_pop_palette(sd_bk_file *bk);

/*! \brief Get palette
 *
 * Returns a pointer to a palette at index from BK file structure. Index must contain
 * a valid palette. If it doesn't, NULL will be returned. Only values between 0 and 7 (inclusive)
 * are valid palette indices.
 *
 * \retval NULL Index is wrong or no palette exists at index.
 * \retval palette* Success.
 *
 * \param bk BK struct pointer.
 * \param index Palette index. Must be 0 <= index <= 7
 */
vga_palette *sd_bk_get_palette(const sd_bk_file *bk, int index);

/*! \brief Load .BK file
 *
 * Loads the given BK file to memory. The structure must be initialized with sd_bk_create()
 * before using this function. Loading to a previously loaded or filled sd_bk_file structure
 * will result in old data and pointers getting lost. This is very likely to cause a memory leak.
 *
 * \retval SD_FILE_OPEN_ERROR File could not be opened.
 * \retval SD_FILE_PARSE_ERROR File does not contain valid data or has syntax problems.
 * \retval SD_SUCCESS Success.
 *
 * \param bk BK struct pointer.
 * \param filename Name of the BK file to load from.
 */
int sd_bk_load(sd_bk_file *bk, const char *filename);
int sd_bk_load_from_pcx(sd_bk_file *bk, const char *filename);

/*! \brief Save .BK file
 *
 * Saves the given BK file from memory to a file on disk. The structure must be at
 * least initialized by using sd_bk_create() before running this.
 *
 * \retval SD_FILE_OPEN_ERROR File could not be opened for writing.
 * \retval SD_SUCCESS Success.
 *
 * \param bk BK struct pointer.
 * \param filename Name of the BK file to save into.
 */
int sd_bk_save(const sd_bk_file *bk, const char *filename);

/*! \brief Free BK file structure
 *
 * Frees up all memory reserved by the BK structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * \param bk BK file struct pointer.
 */
void sd_bk_free(sd_bk_file *bk);

#ifdef __cplusplus
}
#endif

#endif // SD_BK_H
