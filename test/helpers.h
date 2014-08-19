#ifndef _TEST_HELPERS_H
#define _TEST_HELPERS_H

#include <shadowdive/shadowdive.h>

void print_bytes(char *buf, int len, int line, int padding);
void print_pilot_info(sd_pilot *pilot);

#endif // _TEST_HELPERS_H