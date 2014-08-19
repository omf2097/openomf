#ifndef _SD_REC_H
#define _SD_REC_H

#include <stdint.h>
#include <stddef.h>
#include "shadowdive/pilot.h"

#ifdef __cplusplus 
extern "C" {
#endif

enum {
    REC_ACT_NONE =  0x0,
    REC_ACT_PUNCH = 0x1,
    REC_ACT_KICK  = 0x2,
    REC_ACT_A     = 0x4,
    REC_ACT_B     = 0x8,
    REC_ACT_C     = 0x10,
    REC_ACT_D     = 0x20,
    REC_ACT_E     = 0x40,
    REC_ACT_F     = 0x80,
}; 

typedef struct {
    uint32_t tick;
    uint8_t extra;
    uint8_t player_id;
    uint8_t action;
    char extra_data[7];
} sd_rec_move;

typedef struct {
    sd_pilot *pilots[2];

    uint32_t scores[2];
    int8_t unknown_a; // Is Fire or ice ? 0 = no, 1 = fire, 2 = ice ?
    int8_t unknown_b;
    int8_t unknown_c;

    int16_t unknown_d;
    int16_t unknown_e;
    int16_t unknown_f;
    int16_t unknown_g;
    int16_t unknown_h;
    int16_t unknown_i;
    int16_t unknown_j;
    int16_t unknown_k;
    int32_t unknown_l;

    int8_t unknown_m;

    char *raw;
    size_t rawsize;

    unsigned int move_count;
    sd_rec_move *moves;
} sd_rec_file;

int sd_rec_create(sd_rec_file *rec);
void sd_rec_free(sd_rec_file *rec);
int sd_rec_load(sd_rec_file *rec, const char *file);
int sd_rec_save(sd_rec_file *rec, const char *file);

#ifdef __cplusplus
}
#endif

#endif // _SD_REC_H
