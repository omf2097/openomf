#ifndef _AF_H
#define _AF_H

#include <stdint.h>
#include "palette.h"
#include "vga_image.h"
#include "move.h"

typedef struct af_file_t {
    uint16_t file_id;
    uint16_t unknown_a;
    uint32_t endurance;
    uint8_t unknown_b;
    uint16_t power;
    int32_t forward_speed;
    int32_t reverse_speed;
    int32_t jump_speed;
    int32_t fall_speed;
    uint16_t unknown_c;

    sd_move *moves[70];

    char footer[30];
} sd_af_file;

/**
  * Creates AF file structure
  */
sd_af_file* sd_af_create();

/**
  * Load .AF file. Returns 0 on error.
  */
int sd_af_load(sd_af_file *af, const char *filename);

/**
  * Saves AF file structure. Returns 1 on success, 0 on error.
  */
int sd_af_save(sd_af_file *af, const char* filename);

/**
  * Deletes af_file structure from memory.
  */
void sd_af_delete(sd_af_file *af);

#endif // _AF_H
