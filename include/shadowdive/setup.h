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
	char unknown_a[212];
	uint16_t unknown_b;
	uint16_t unknown_c;
	uint16_t unknown_d;
	char unknown_e[18];
	uint8_t unknown_f;
	uint8_t unknown_g;
	uint16_t unknown_h;
	uint16_t unknown_i;
	uint16_t unknown_j;
	uint16_t unknown_k;
	uint16_t unknown_l;
	uint16_t unknown_m;
	uint8_t general_flags;
	uint8_t unknown_n;
	uint32_t unknown_o;
	uint8_t unknown_p;
	uint8_t unknown_q;
	char unknown_r[38];
} __attribute__((packed)) sd_setup_file;

int sd_setup_create(sd_setup_file *setup);
void sd_setup_free(sd_setup_file *setup);
int sd_setup_load(sd_setup_file *setup, const char *file);

#ifdef __cplusplus
}
#endif

#endif // _SD_SETUP_H
