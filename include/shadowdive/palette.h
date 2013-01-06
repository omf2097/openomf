#ifndef _SD_PALETTE_H
#define _SD_PALETTE_H

typedef struct sd_reader_t sd_reader;
typedef struct sd_writer_t sd_writer;

typedef struct sd_palette_t {
    char data[256][3];
    char remaps[19][256];
} sd_palette;

void sd_palette_to_rgb(sd_palette *palette);

void sd_palette_to_gimp_palette(char *filename, sd_palette *palette);

int sd_palette_load(sd_reader *reader, sd_palette *palette);
void sd_palette_save(sd_writer *writer, sd_palette *palette);

#endif // _SD_PALETTE_H
