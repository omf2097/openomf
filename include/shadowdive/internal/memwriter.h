#ifndef _SD_MEMWRITER_H
#define _SD_MEMWRITER_H

#include <stdint.h>
#include "shadowdive/internal/writer.h"

typedef struct sd_mwriter_t {
    char *buf;
    long real_len; // Current buffer size
    long data_len; // Current data size in buffer
} sd_mwriter;

sd_mwriter* sd_mwriter_open();
void sd_mwriter_save(sd_mwriter *src, sd_writer *dst);
void sd_mwriter_close(sd_mwriter *writer);
long sd_mwriter_pos(sd_mwriter *writer);
void sd_mwriter_xor(sd_mwriter *writer, uint8_t key);

void sd_mwrite_buf(sd_mwriter *writer, const char *buf, int len);
void sd_mwrite_ubyte(sd_mwriter *writer, uint8_t value);
void sd_mwrite_uword(sd_mwriter *writer, uint16_t value);
void sd_mwrite_udword(sd_mwriter *writer, uint32_t value);
void sd_mwrite_byte(sd_mwriter *writer, int8_t value);
void sd_mwrite_word(sd_mwriter *writer, int16_t value);
void sd_mwrite_dword(sd_mwriter *writer, int32_t value);
void sd_mwrite_float(sd_mwriter *writer, float value);
void sd_mwrite_fill(sd_mwriter *writer, char content, int len);

#endif // _SD_MEMWRITER_H
