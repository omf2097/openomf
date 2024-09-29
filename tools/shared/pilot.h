#ifndef PILOT_H
#define PILOT_H

#include "formats/pilot.h"

void print_bytes(char *buf, int len, int line, int padding);
void print_pilot_info(sd_pilot *pilot);
void print_pilot_player_info(sd_pilot *pilot);
void print_pilot_array_row(sd_pilot *pilot, int i);
void print_pilot_array_header(void);

#endif // PILOT_H
