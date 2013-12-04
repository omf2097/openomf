#ifndef _TOURNAMENT_H
#define _TOURNAMENT_H

#include <stdint.h>
#include <float.h>

#ifdef __cplusplus 
extern "C" {
#endif

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

    char *english_quote;
} sd_tournament_enemy;

typedef struct sd_tournament_file_t {
    int16_t enemy_count;
    int32_t victory_text_offset; // ???
    char bk_name[14];
    float winnings_multiplier;
    int32_t registration_free; 
    int32_t assumed_initial_value; 
    int32_t tournament_id; 

    sd_tournament_enemy **enemies;

    // Offsets section starts at 300 (0x12C) ?
    // Data section starts at 300 + enemy_count * 4
    // Player info starts at offset_list[0] and ends at offset_list[player_count+1]
    // From this point, maybe image data ?
    // Parts of data section may be XORred ?
    // Victory text section at victory_text_offset

} sd_tournament_file;

sd_tournament_file* sd_tournament_create();
int sd_tournament_load(sd_tournament_file *trn, const char *filename);
int sd_tournament_save(sd_tournament_file *trn, const char *filename);
void sd_tournament_delete(sd_tournament_file *trn);

#ifdef __cplusplus
}
#endif

#endif // _TOURNAMENT_H
