#ifndef MEMWRITER_H
#define MEMWRITER_H

#include <stdint.h>
#include "formats/internal/writer.h"

typedef struct memwriter_t {
    char *buf;
    long real_len; // Current buffer size
    long data_len; // Current data size in buffer
} memwriter;

memwriter* memwriter_open();
void memwriter_save(const memwriter *src, sd_writer *dst);
void memwriter_close(memwriter *writer);
long memwriter_pos(const memwriter *writer);
void memwriter_xor(memwriter *writer, uint8_t key);

void memwrite_buf(memwriter *writer, const char *buf, int len);
void memwrite_ubyte(memwriter *writer, uint8_t value);
void memwrite_uword(memwriter *writer, uint16_t value);
void memwrite_udword(memwriter *writer, uint32_t value);
void memwrite_byte(memwriter *writer, int8_t value);
void memwrite_word(memwriter *writer, int16_t value);
void memwrite_dword(memwriter *writer, int32_t value);
void memwrite_float(memwriter *writer, float value);
void memwrite_fill(memwriter *writer, char content, int len);

#endif // MEMWRITER_H
