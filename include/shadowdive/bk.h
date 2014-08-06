/*! \file 
 * \brief Contains functions for handling .BK (arena/background) files.
 * \license MIT
 */ 

#ifndef _SD_BK_H
#define _SD_BK_H

#include <stdint.h>
#include "shadowdive/palette.h"
#include "shadowdive/bkanim.h"
#include "shadowdive/vga_image.h"

#ifdef __cplusplus 
extern "C" {
#endif

#define MAX_BK_ANIMS 50
#define MAX_BK_PALETTES 8

typedef struct {
    uint32_t file_id;
    uint8_t unknown_a;
    uint8_t palette_count;

    sd_bk_anim *anims[MAX_BK_ANIMS];
    sd_vga_image *background;
    sd_palette *palettes[MAX_BK_PALETTES];

    char soundtable[30];
} sd_bk_file;

/*! \brief Initialize BK container
 *
 * Initializes the BK container with empty values.
 *
 * Return values:
 * - SD_INVALID_INPUT If bk struct pointer was NULL
 * - SD_SUCCESS on success.
 *
 * \param bk Allocated BK struct pointer.
 * \return SD_SUCCESS or errorcode.
 */
int sd_bk_create(sd_bk_file *bk);

/*! \brief Copy BK structure
 *
 * Copies the contents of an BK file structure. _ALL_ internals will be copied.
 * The copied structure must be freed using sd_bk_free().
 *
 * Destination buffer does not need to be cleared. Source buffer must be a valid
 * BK file structure, of problems are likely to appear.
 *
 * Return values:
 * - SD_OUT_OF_MEMORY If memory ran out. The destination struct should be considered invalid and freed.
 * - SD_INVALID_INPUT If either of the input pointers was NULL.
 * - SD_SUCCESS on success.
 *
 * \param dst Destination BK struct pointer.
 * \param src Source BK struct pointer.
 * \return SD_SUCCESS or errorcode.
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
 * Return values:
 * - SD_OUT_OF_MEMORY If memory ran out. Background was not set.
 * - SD_INVALID_INPUT If bk struct was NULL.
 * - SD_SUCCESS on success.
 * 
 * \param bk BK struct pointer.
 * \param img VGA image data struct.
 * \return SD_SUCCESS or errorcode.
 */
int sd_bk_set_background(sd_bk_file *bk, const sd_vga_image *img);

/*! \brief Get background image
 *
 * Returns a pointer to the background image data.
 *
 * Return values:
 * - NULL if background does not exist
 * - A pointer to sd_vga_image if background exists.
 * 
 * \param bk BK struct pointer.
 * \return A pointer to sd_vga_image or NULL.
 */
sd_vga_image* sd_bk_get_background(const sd_bk_file *bk);

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
 * Return values:
 * - SD_OUT_OF_MEMORY If memory ran out. This struct should now be considered invalid and freed.
 * - SD_INVALID_INPUT If index value was invalid or bk struct was NULL.
 * - SD_SUCCESS on success.
 *
 * \param bk BK struct pointer.
 * \param index Animation index. Must be 0 <= index <= 49
 * \param anim Animation pointer. NULL here means that index will be cleared.
 * \return SD_SUCCESS or errorcode.
 */
int sd_bk_set_anim(sd_bk_file *bk, int index, const sd_bk_anim *anim);

/*! \brief Get BK animation
 *
 * Returns a pointer to a bk animation data at given index. Index must be between
 * 0 and 49 (inclusive); any other value will return NULL.
 * 
 * Return values:
 * - NULL if there is no BK animation at index, or index is otherwise invalid.
 * - A pointer to sd_bk_anim on success.
 *
 * \param bk BK struct pointer.
 * \param index Animation index. Must be 0 <= index <= 49
 * \return Sd_bk_anim pointer if success, or NULL if error or index has no data.
 */
sd_bk_anim* sd_bk_get_anim(const sd_bk_file *bk, int index);

/*! \brief Set palette
 *
 * Sets a palette to index in BK file structure. You can only replace already set
 * palettes by using this function, so index must be between 0 and palette_count.
 * For adding new palettes, please see sd_bk_push_palette().
 *
 * Palette data will be copied. Make sure to free your local copy yourself.
 * Old data in index will be freed automatically.
 *
 * Return values:
 * - SD_OUT_OF_MEMORY If memory ran out. Palette data was not copied.
 * - SD_INVALID_INPUT If index was invalid or some input pointer was NULL.
 * - SD_SUCCESS on success.
 *
 * \param bk BK struct pointer.
 * \param index Palette index.
 * \param palette A Valid sd_palette object pointer.
 * \return SD_SUCCESS or errorcode.
 */
int sd_bk_set_palette(sd_bk_file *bk, int index, const sd_palette *palette);

/*! \brief Push palette
 *
 * Pushes a palette to the end of the palette list. If list is full,
 * error value SD_INVALID_INPUT will be returned.
 *
 * Palette data will be copied. Make sure to free your local copy yourself.
 *
 * Return values:
 * - SD_OUT_OF_MEMORY If memory ran out. Palette data was not copied.
 * - SD_INVALID_INPUT If palette list is already full.
 * - SD_SUCCESS on success.
 *
 * \param bk BK struct pointer.
 * \param palette A Valid sd_palette object pointer.
 * \return SD_SUCCESS or errorcode.
 */
int sd_bk_push_palette(sd_bk_file *bk, const sd_palette *palette);

/*! \brief Pop palette
 *
 * Pops a palette from the end of the palette list. If palette list is already empty,
 * error will be returned and nothing will happen.
 *
 * Popped palette data will be freed automatically.
 *
 * Return values:
 * - SD_INVALID_INPUT If there was nothing to pop.
 * - SD_SUCCESS on success.
 *
 * \param bk BK struct pointer.
 * \return SD_SUCCESS or errorcode.
 */
int sd_bk_pop_palette(sd_bk_file *bk);

/*! \brief Get palette
 *
 * Returns a pointer to a palette at index from BK file structure. Index must contain
 * a valid palette. If it doesn't, NULL will be returned. Only values between 0 and 7 (inclusive)
 * are valid palette indices.
 *
 * Return values:
 * - NULL if index is wrong or no palette exists at index.
 * - A pointer to the sd_palette structure on success.
 *
 * \param bk BK struct pointer.
 * \param index Palette index. Must be 0 <= index <= 7
 * \return Palette pointer or NULL.
 */
sd_palette* sd_bk_get_palette(const sd_bk_file *bk, int index);

/*! \brief Load .BK file
 * 
 * Loads the given BK file to memory. The structure must be initialized with sd_bk_create() 
 * before using this function. Loading to a previously loaded or filled sd_bk_file structure 
 * will result in old data and pointers getting lost. This is very likely to cause a memory leak.
 *
 * Return values:
 * - SD_FILE_OPEN_ERROR If file could not be opened.
 * - SD_FILE_PARSE_ERROR If file does not contain valid data or has syntax problems.
 * - SD_OUT_OF_MEMORY If memory ran out. This struct should now be considered invalid and freed.
 * - SD_SUCCESS on success.
 *
 * \param bk BK struct pointer.
 * \param filename Name of the BK file.
 * \return SD_SUCCESS or error code.
 */
int sd_bk_load(sd_bk_file *bk, const char *filename);

/*! \brief Save .BK file
 * 
 * Saves the given BK file from memory to a file on disk. The structure must be at
 * least initialized by using sd_bk_create() before running this.
 * 
 * Return values:
 * - SD_FILE_OPEN_ERROR If file could not be opened.
 * - SD_SUCCESS on success.
 * 
 * \param bk BK struct pointer. 
 * \param filename Name of the BK file to save into.
 * \return SD_SUCCESS or errorcode.
 */
int sd_bk_save(const sd_bk_file *bk, const char* filename);

/*! \brief Free BK container
 * Frees up the bk struct memory.
 * \param bk BK struct pointer.
 */
void sd_bk_free(sd_bk_file *bk);

#ifdef __cplusplus
}
#endif

#endif // _SD_BK_H
