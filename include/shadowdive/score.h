#ifndef _SD_SCORE_H
#define _SD_SCORE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SD_SCORE_PAGES 4
#define SD_SCORE_ENTRIES 20

typedef struct {
    uint32_t score;
    char name[16]; // NULL terminated
    uint32_t har_id:6;
    uint32_t pilot_id:6;
    uint32_t padding:20;
} sd_score_entry;

typedef struct {
    sd_score_entry scores[SD_SCORE_PAGES][SD_SCORE_ENTRIES];
} sd_score;

int sd_score_create(sd_score *score);
void sd_score_free(sd_score *score);
int sd_score_load(sd_score *score, const char *filename);
int sd_score_save(sd_score *score, const char *filename);
sd_score_entry* sd_score_get(sd_score *score, int page, int entry_id);

#ifdef __cplusplus
}
#endif

#endif // _SD_SCORE_H
