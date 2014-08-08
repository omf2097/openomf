#ifndef _SD_PALETTE_H
#define _SD_PALETTE_H

#include <stdint.h>
#ifdef SD_USE_INTERNAL
    #include "shadowdive/internal/reader.h"
    #include "shadowdive/internal/writer.h"
#endif

#ifdef __cplusplus 
extern "C" {
#endif

typedef struct {
    unsigned char data[256][3];
    unsigned char remaps[19][256];
} sd_palette;

unsigned char sd_palette_resolve_color(uint8_t r, uint8_t g, uint8_t b, const sd_palette *pal);
int sd_palette_to_gimp_palette(const char *filename, const sd_palette *palette);

#ifdef SD_USE_INTERNAL
int sd_palette_load(sd_reader *reader, sd_palette *palette);
void sd_palette_save(sd_writer *writer, const sd_palette *palette);
#endif

#ifdef __cplusplus
}
#endif

#endif // _SD_PALETTE_H
