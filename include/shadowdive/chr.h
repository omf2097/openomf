#ifndef _SD_CHR_H
#define _SD_CHR_H

#include "shadowdive/palette.h"
#include "shadowdive/sprite.h"
#include "shadowdive/pilot.h"

#ifdef __cplusplus 
extern "C" {
#endif

#define MAX_CHR_ENEMIES 256

typedef struct {
    char name[17];
    uint16_t wins;
    uint16_t losses;
    uint8_t rank;
    uint8_t har;
} sd_chr_enemy;

typedef struct {
    sd_pilot pilot;
    char unknown[20];
    sd_palette pal;
    sd_sprite *photo;
    sd_chr_enemy *enemies[MAX_CHR_ENEMIES];
} sd_chr_file;

int sd_chr_create(sd_chr_file *chr);
int sd_chr_load(sd_chr_file *chr, const char *filename);
int sd_chr_save(sd_chr_file *chr, const char *filename);
void sd_chr_free(sd_chr_file *chr);

#ifdef __cplusplus
}
#endif

#endif // _SD_CHR_H
