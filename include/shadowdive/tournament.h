#ifndef _SD_TOURNAMENT_H
#define _SD_TOURNAMENT_H

#include "shadowdive/palette.h"
#include "shadowdive/sprite.h"
#include "shadowdive/pilot.h"

#ifdef __cplusplus 
extern "C" {
#endif

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

typedef struct {
    sd_sprite *logo;
    char *title;
    char *description;
    char *end_texts[11][10];
} sd_tournament_locale;

typedef struct {
    int16_t enemy_count;
    char bk_name[14];
    float winnings_multiplier;
    int32_t registration_free; 
    int32_t assumed_initial_value; 
    int32_t tournament_id;
    char *pic_file;

    sd_pilot *enemies[MAX_TRN_ENEMIES];
    sd_tournament_locale *locales[MAX_TRN_LOCALES];
    char *quotes[MAX_TRN_ENEMIES][MAX_TRN_LOCALES];

    sd_palette pal;
} sd_tournament_file;

int sd_tournament_create(sd_tournament_file *trn);
int sd_tournament_load(sd_tournament_file *trn, const char *filename);
int sd_tournament_save(sd_tournament_file *trn, const char *filename);
void sd_tournament_free(sd_tournament_file *trn);

#ifdef __cplusplus
}
#endif

#endif // _SD_TOURNAMENT_H
