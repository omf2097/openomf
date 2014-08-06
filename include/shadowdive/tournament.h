#ifndef _SD_TOURNAMENT_H
#define _SD_TOURNAMENT_H

#ifdef __cplusplus 
extern "C" {
#endif

#include "palette.h"
#include "sprite.h"

#define MAX_TRN_ENEMIES 256
#define MAX_TRN_LOCALES 10

enum {
    TRN_LANG_ENGLISH = 0,
    TRN_LANG_GERMAN,
    TRN_LANG_FRENCH,
    TRN_LANG_SPANISH,
    TRN_LANG_MEXICAN,
    TRN_LANG_ITALIAN,
    TRN_LANG_POLISH,
    TRN_LANG_RUSSIAN,
    TRN_LANG_UNDEF_1,
    TRN_LANG_UNDEF_2
};

typedef struct sd_tournament_enemy_t {
    uint32_t unknown_a;
    char name[18];
    uint16_t wins;
    uint16_t losses;
    uint16_t robot_id;
    char stats[8];
    uint16_t offense;
    uint16_t defense;
    uint32_t money;
    uint8_t color_1;
    uint8_t color_2;
    uint8_t color_3;
    char unk_block_a[107];
    uint16_t force_arena;
    char unk_block_b[3];
    uint8_t movement;
    char unk_block_c[6];
    char enhancements[11];
    uint8_t flags;
    uint16_t reqs[5];
    uint16_t attitude[3];
    char unk_block_d[6];
    uint16_t ap_throw;
    uint16_t ap_special;
    uint16_t ap_jump;
    uint16_t ap_high;
    uint16_t ap_low;
    uint16_t ap_middle;
    uint16_t pref_jump;
    uint16_t pref_fwd;
    uint16_t pref_back;
    char unk_block_e[4];
    float learning;
    float forget;
    char unk_block_f[24];
    uint32_t winnings;
    char unk_block_g[166];
    uint16_t photo_id;

    char *quote[MAX_TRN_LOCALES];
} sd_tournament_enemy;

typedef struct sd_tournament_locale_t {
    sd_sprite *logo;
    char *title;
    char *description;
    char *end_texts[11][10];
} sd_tournament_locale;

typedef struct sd_tournament_file_t {
    int16_t enemy_count;
    char bk_name[14];
    float winnings_multiplier;
    int32_t registration_free; 
    int32_t assumed_initial_value; 
    int32_t tournament_id;
    char *pic_file;

    sd_tournament_enemy *enemies[MAX_TRN_ENEMIES];
    sd_tournament_locale *locales[MAX_TRN_LOCALES];

    sd_palette pal;
} sd_tournament_file;

int sd_tournament_create(sd_tournament_file *trn);
int sd_tournament_load(sd_tournament_file *trn, const char *filename);
int sd_tournament_save(sd_tournament_file *trn, const char *filename);
void sd_tournament_delete(sd_tournament_file *trn);

#ifdef __cplusplus
}
#endif

#endif // _SD_TOURNAMENT_H
