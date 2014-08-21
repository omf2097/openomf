#ifndef _PILOT_H
#define _PILOT_H

#include <shadowdive/shadowdive.h>

void print_bytes(char *buf, int len, int line, int padding);
void print_pilot_info(sd_pilot *pilot);

#endif // _PILOT_H