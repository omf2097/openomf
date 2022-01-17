#ifndef CONVERSIONS_H
#define CONVERSIONS_H

#include <stdint.h>

uint8_t conv_ubyte(const char *data);
int8_t conv_byte(const char *data);
uint16_t conv_uword(const char *data);
int16_t conv_word(const char *data);
uint32_t conv_udword(const char *data);
int32_t conv_dword(const char *data);
float conv_float(const char *data);

#endif // CONVERSIONS_H
