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

// This is hardcoded to file format. Do not change.
#define MAX_BK_ANIMS 50

// 8 palettes ought to be enough for everybody
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
 * Initializes the BK container with empty values. When saved, this will create an empty BK file.
 * \param af Allocated BK struct pointer.
 * \return SD_SUCCESS or errorcode.
 */
int sd_bk_create(sd_bk_file *bk);

/*! \brief Copy BK structure
 * Copies a valid BK structure. Note: Source structure must be valid.
 * \param dst Destination BK struct pointer.
 * \param src Source BK struct pointer.
 * \return SD_SUCCESS or errorcode.
 */
int sd_bk_copy(sd_bk_file *dst, const sd_bk_file *src);

/*! \brief Set background image
 * Sets the background image of the BK file.
 * \param bk BK struct pointer.
 * \param img VGA image data struct.
 * \return SD_SUCCESS or errorcode.
 */
int sd_bk_set_background(sd_bk_file *bk, const sd_vga_image *img);

/*! \brief Get background image
 * Gets the background image of the BK file.
 * \param bk BK struct pointer.
 * \return A pointer to sd_vga_image* on success, or NULL if no image is set.
 */
sd_vga_image* sd_bk_get_background(sd_bk_file *bk);

/*! \brief Set bk animation
 * Sets a BK animation in BK file structure. 
 * \param bk BK struct pointer.
 * \param index Animation index. Must be 0 <= index <= 49
 * \param anim Animation pointer.
 * \return SD_SUCCESS or errorcode.
 */
int sd_bk_set_anim(sd_bk_file *bk, int index, const sd_bk_anim *anim);

/*! \brief Get bk animation
 * Gets a BK animation from BK file structure. 
 * \param bk BK struct pointer.
 * \param index Animation index. Must be 0 <= index <= 49
 * \return Sd_bk_anim pointer if success, or NULL if error or index has no data.
 */
sd_bk_anim* sd_bk_get_anim(sd_bk_file *bk, int index);

/*! \brief Set palette
 * Sets a palette to index in BK file structure.
 * \param bk BK struct pointer.
 * \param index Palette index.
 * \param palette A Valid sd_palette object pointer.
 */
void sd_bk_set_palette(sd_bk_file *bk, int index, const sd_palette *palette);

/*! \brief Push palette
 * Pushes a palette to the end of the palette list
 * \param bk BK struct pointer.
 * \param palette A Valid sd_palette object pointer.
 */
int sd_bk_push_palette(sd_bk_file *bk, const sd_palette *palette);

/*! \brief Pop palette
 * Pops a palette from the end of the palette list
 * \param bk BK struct pointer.
 */
int sd_bk_pop_palette(sd_bk_file *bk);

/*! \brief Get palette
 * Gets a palette from BK file structure. 
 * \param bk BK struct pointer.
 * \param index Palette index. Must be 0 <= index <= 63
 * \return Palette pointer if success, or NULL if error or index has no data.
 */
sd_palette* sd_bk_get_palette(sd_bk_file *bk, int index);

/*! \brief Load .BK file
 * Loads the given BK file to memory
 * \param bk BK struct pointer.
 * \param filename Name of the BK file.
 * \return SD_SUCCESS on success, or SD_FILE_OPEN_ERROR or SD_FILE_PARSE_ERROR on failure.
 */
int sd_bk_load(sd_bk_file *bk, const char *filename);

/*! \brief Save .BK file
 * Saves the given BK file from memory to a file on disk.
 * \param bk BK struct pointer. 
 * \param filename Name of the BK file to save into.
 * \return SD_SUCCESS on success, or SD_FILE_OPEN_ERROR on failure.
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
