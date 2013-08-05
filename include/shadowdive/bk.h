/*! \file 
 * \brief Contains functions for handling .BK (arena/background) files.
 * \license MIT
 */ 

#ifndef _SD_BK_H
#define _SD_BK_H

#include <stdint.h>

#ifdef __cplusplus 
extern "C" {
#endif

#ifndef _SD_PALETTE_H
typedef struct sd_palette_t sd_palette;
#endif

#ifndef _SD_ANIMATION_H
typedef struct sd_bk_anim_t sd_bk_anim;
#endif

#ifndef _SD_VGA_IMAGE_H
typedef struct sd_vga_image_t sd_vga_image;
#endif

typedef struct sd_bk_file_t {
    uint32_t file_id;
    uint8_t unknown_a;

    sd_bk_anim *anims[50];

    sd_vga_image *background;

    uint8_t num_palettes;
    sd_palette **palettes;

    char soundtable[30];
} sd_bk_file;

/*! \brief Create BK container
 * Creates the BK container struct, allocated memory etc.
 * \return BK container struct pointer
 */
sd_bk_file* sd_bk_create();

/*! \brief Set background image
 * Loads Sets the background image of the BK file.
 * \param bk BK struct pointer.
 * \param img Image data struct
 * \return SD_SUCCESS on success.
 */
void sd_bk_set_background(sd_bk_file *bk, sd_vga_image *img);

/*! \brief Load .BK file
 * Loads the given BK file to memory
 * \param bk BK struct pointer.
 * \param filename Name of the BK file.
 * \return SD_SUCCESS on success.
 */
int sd_bk_load(sd_bk_file *bk, const char *filename);

/*! \brief Save .BK file
 * Saves the given BK file from memory to a file on disk.
 * \param bk BK struct pointer. 
 * \param filename Name of the BK file to save into.
 * \return SD_SUCCESS on success.
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
