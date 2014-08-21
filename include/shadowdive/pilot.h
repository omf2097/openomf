#ifndef _SD_PILOT_H
#define _SD_PILOT_H

#include <stdint.h>
#ifdef SD_USE_INTERNAL
    #include "shadowdive/internal/reader.h"
    #include "shadowdive/internal/writer.h"
#endif

#ifdef __cplusplus 
extern "C" {
#endif

typedef struct {
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
    uint8_t unk_flag_a;
    uint8_t flags;
    uint8_t unk_flag_b;
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
} sd_pilot;

int sd_pilot_create(sd_pilot *pilot);
void sd_pilot_free(sd_pilot *pilot);

#ifdef SD_USE_INTERNAL
int sd_pilot_load(sd_reader *reader, sd_pilot *pilot);
int sd_pilot_save(sd_writer *writer, const sd_pilot *pilot);
#endif

#ifdef __cplusplus
}
#endif

#endif // _SD_PILOT_H