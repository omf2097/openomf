#ifndef _SD_CHR_H
#define _SD_CHR_H

#include "palette.h"
#include "sprite.h"

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
    char name[17];
    uint16_t wins;
    uint16_t losses;
    uint8_t rank;
    uint8_t har;
    uint8_t arm_power;
    uint8_t leg_power;
    uint8_t arm_speed;
    uint8_t leg_speed;
    uint8_t armor;
    uint8_t stun_resistance;
    uint8_t power;
    uint8_t agility;
    uint8_t endurance;
    uint32_t credits;
    uint8_t color_1;
    uint8_t color_2;
    uint8_t color_3;
    char trn_name[13];
    char trn_desc[30];
    char trn_image[13];
    uint8_t difficulty;
    char enhancements[11];
    uint16_t enemies_inc_unranked;
    uint16_t enemies_ex_unranked;
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
