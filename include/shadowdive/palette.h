#ifndef _SD_PALETTE_H
#define _SD_PALETTE_H

#include <stdint.h>

#ifdef __cplusplus 
extern "C" {
#endif

#ifndef _SD_READER_H
typedef struct sd_reader_t sd_reader;
#endif

#ifndef _SD_WRITER_H
typedef struct sd_writer_t sd_writer;
#endif

typedef struct sd_palette_t {
    unsigned char data[256][3];
    unsigned char remaps[19][256];
} sd_palette;

unsigned char sd_palette_resolve_color(uint8_t r, uint8_t g, uint8_t b, sd_palette *pal);

void sd_palette_to_gimp_palette(char *filename, sd_palette *palette);

int sd_palette_load(sd_reader *reader, sd_palette *palette);
void sd_palette_save(sd_writer *writer, sd_palette *palette);

#ifdef __cplusplus
}
#endif

#endif // _SD_PALETTE_H
