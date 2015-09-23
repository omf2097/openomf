/*! \file
 * \brief Setup file handler
 * \details Functions and structs for reading, writing and modifying OMF:2097 setup (SETUP.CFG) files.
 * \copyright MIT license.
 * \date 2013-2015
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */

#ifndef _SD_SETUP_H
#define _SD_SETUP_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint16_t unk:5;
	uint16_t power_1:3;
	uint16_t unk2:2;
	uint16_t power_2:3;
	uint16_t unk3:3;
} __attribute__((packed)) gflags0;

typedef struct {
	uint8_t unk:2;
	uint8_t shadows:2;
	uint8_t hazards:1;
	uint8_t hyper_mode:1;
	uint8_t screen_shakes:1;
	uint8_t animations:1;
} __attribute__((packed)) gflags1;

typedef struct {
	uint8_t palette_animation:1;
	uint8_t unk2:2;
	uint8_t snow_checking:1;
	uint8_t unk3:4;
} __attribute__((packed)) gflags2;

typedef struct {
	uint32_t stereo_reversed:1;
	uint32_t match_count:2;
	uint32_t unk:29;
} __attribute__((packed)) gflags3;

typedef struct {
	uint8_t game_speed;
	char unknown_a[211];
	uint16_t unknown_b;
	uint16_t unknown_c;
	uint16_t unknown_d;
	char unknown_e[18];
	uint8_t difficulty;
	uint8_t unknown_g;
	uint16_t unknown_h;
	uint16_t unknown_i;
	uint16_t unknown_j;
	uint16_t unknown_k;
	uint16_t unknown_l;
	gflags0 general_flags_0;
	gflags1 general_flags_1;
	gflags2 general_flags_2;
	gflags3 general_flags_3;
	uint8_t sound_volume;
	uint8_t music_volume;
	char unknown_r[38];
} __attribute__((packed)) sd_setup_file;

int sd_setup_create(sd_setup_file *setup);
void sd_setup_free(sd_setup_file *setup);
int sd_setup_load(sd_setup_file *setup, const char *file);

#ifdef __cplusplus
}
#endif

#endif // _SD_SETUP_H
