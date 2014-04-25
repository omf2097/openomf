#ifndef _SD_SCORE_H
#define _SD_SCORE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sd_score_entry_t {
    uint32_t score;
    char name[16]; // NULL terminated
    uint32_t har_id:6;
    uint32_t pilot_id:6;
    uint32_t padding:20;
} sd_score_entry;

typedef struct sd_score_t {
    sd_score_entry scores[4][20];
} sd_score;

sd_score* sd_score_create();
void sd_score_delete(sd_score *score);
int sd_score_load(sd_score *score, const char *filename);
void sd_score_save(sd_score *score, const char *filename);

#ifdef __cplusplus
}
#endif

#endif // _SD_SCORE_H
