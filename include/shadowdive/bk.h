/*! \file 
 * \brief Contains functions for handling .BK (arena/background) files.
 * \license MIT
 */ 

#ifndef _SD_BK_H
#define _SD_BK_H

#ifdef __cplusplus 
extern "C" {
#endif

#include <stdint.h>

#ifndef _SD_PALETTE_H
typedef struct sd_palette_t sd_palette;
#endif

#ifndef _SD_ANIMATION_H
typedef struct sd_bk_anim_t sd_bk_anim;
#endif

#ifndef _SD_VGA_IMAGE_H
typedef struct sd_vga_image_t sd_vga_image;
#endif

// This is hardcoded to file format. Do not change.
#define MAX_BK_ANIMS 50

// This can be changed (maximum of 255)
// 64 should be enough, though
#define MAX_BK_PALETTES 64

typedef struct sd_bk_file_t {
    uint32_t file_id;
    uint8_t unknown_a;
    uint8_t num_palettes;
    sd_bk_anim *anims[MAX_BK_ANIMS];
    sd_vga_image *background;
    sd_palette *palettes[MAX_BK_PALETTES];
    char soundtable[30];
} sd_bk_file;

/*! \brief Create BK container
 * Creates the BK container struct, allocated memory etc.
 * \return BK container struct pointer
 */
sd_bk_file* sd_bk_create();

/*! \brief Set background image
 * Sets the background image of the BK file.
 * \param bk BK struct pointer.
 * \param img VGA image data struct. Note! pointer will be copied.
 */
void sd_bk_set_background(sd_bk_file *bk, sd_vga_image *img);

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
 * \param anim Animation pointer. May be NULL.
 */
void sd_bk_set_anim(sd_bk_file *bk, int index, sd_bk_anim *anim);

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
 * \param index Palette index. Must be 0 <= index <= 63
 * \param palette A Valid sd_palette object pointer. Must be reserved with malloc.
 */
void sd_bk_set_palette(sd_bk_file *bk, int index, sd_palette *palette);

/*! \brief Push palette
 * Pushes a palette to the end of the palette list
 * \param bk BK struct pointer.
 * \param palette A Valid sd_palette object pointer. Must be reserved with malloc.
 */
void sd_bk_push_palette(sd_bk_file *bk, sd_palette *palette);

/*! \brief Pop palette
 * Pops a palette from the end of the palette list
 * \param bk BK struct pointer.
 */
void sd_bk_pop_palette(sd_bk_file *bk);

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
int sd_bk_save(sd_bk_file *bk, const char* filename);

/*! \brief Free BK container
 * Frees up the bk struct memory.
 * \param bk BK struct pointer.
 */
void sd_bk_delete(sd_bk_file *bk);

#ifdef __cplusplus
}
#endif

#endif // _SD_BK_H
