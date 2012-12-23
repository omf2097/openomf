#ifndef _BK_H
#define _BK_H

#include <stdint.h>

typedef struct bk_file_t {
    uint32_t file_id;
    uint8_t unknown_a;
    uint16_t img_w;
    uint16_t img_h;
} bk_file;

/**
  * Load .BK file. Returns 0 on error.
  */
bk_file* bk_load(const char *filename);

/**
  * Saves BK file structure. Returns 1 on success, 0 on error.
  */
int bk_save(const char* filename, bk_file *data);

/**
  * Destroys bk_file structure from memory.
  */
void bk_destroy(bk_file *data);

#endif // _BK_H
