/*! \file
 * \brief Setup file handler
 * \details Functions and structs for reading, writing and modifying OMF:2097 setup (SETUP.CFG) files.
 * \copyright MIT license.
 * \date 2013-2015
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */

#ifndef SD_SETUP_H
#define SD_SETUP_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint16_t unk : 2;
    uint16_t rehit_mode : 1;
    uint16_t def_throws : 1;
    uint16_t unk2 : 1;
    uint16_t power_1 : 3;
    uint16_t unk3 : 2;
    uint16_t power_2 : 3;
    uint16_t unk4 : 3;
} gflags0;
static_assert(2 == sizeof(gflags0), "gflags0 should pack into 2 bytes");

typedef struct {
    uint8_t knockdown : 2;
    uint8_t shadows : 2;
    uint8_t hazards : 1;
    uint8_t hyper_mode : 1;
    uint8_t screen_shakes : 1;
    uint8_t animations : 1;
} gflags1;
static_assert(1 == sizeof(gflags1), "gflags1 should pack into 1 byte");

typedef struct {
    uint8_t palette_animation : 1;
    uint8_t unk2 : 2;
    uint8_t snow_checking : 1;
    uint8_t unk3 : 4;
} gflags2;
static_assert(1 == sizeof(gflags2), "gflags2 should pack into 1 byte");

typedef struct {
    uint32_t stereo_reversed : 1;
    uint32_t match_count : 2;
    uint32_t unk : 29;
} gflags3;
static_assert(4 == sizeof(gflags3), "gflags3 should pack into 4 bytes");

typedef struct {
    uint8_t game_speed;
    char unknown_a[211];
    uint16_t unknown_b;
    uint16_t unknown_c;
    uint16_t unknown_d;
    char unknown_e[18];
    uint8_t difficulty;
    uint8_t unknown_g;
    uint16_t throw_range;
    uint16_t hit_pause;
    uint16_t block_damage;
    uint16_t vitality;
    uint16_t jump_height;
    gflags0 general_flags_0;
    gflags1 general_flags_1;
    gflags2 general_flags_2;
    gflags3 general_flags_3;
    uint8_t sound_volume;
    uint8_t music_volume;
    char unknown_r[38];
} sd_setup_file;
static_assert(296 == sizeof(sd_setup_file), "sd_setup_file should pack into 296 bytes");

int sd_setup_create(sd_setup_file *setup);
void sd_setup_free(sd_setup_file *setup);
int sd_setup_load(sd_setup_file *setup, const char *file);

#endif // SD_SETUP_H
