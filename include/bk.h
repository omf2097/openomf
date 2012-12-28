#ifndef _BK_H
#define _BK_H

#include <stdint.h>
#include "palette.h"
#include "vga_image.h"
#include "animation.h"

typedef struct bk_file_t {
    uint32_t file_id;
    uint8_t unknown_a;
    uint16_t img_w;
    uint16_t img_h;

    sd_animation *animations[50];

    sd_vga_image *background;

    uint8_t num_palettes;
    sd_palette **palettes;

    char footer[30];
} sd_bk_file;

/**
  * Load .BK file. Returns 0 on error.
  */
sd_bk_file* sd_bk_load(const char *filename);

/**
  * Saves BK file structure. Returns 1 on success, 0 on error.
  */
int sd_bk_save(const char* filename, sd_bk_file *bk);

/**
  * Deletes bk_file structure from memory.
  */
void sd_bk_delete(sd_bk_file *bk);

#endif // _BK_H
