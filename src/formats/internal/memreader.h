#ifndef MEMREADER_H
#define MEMREADER_H

#include "formats/internal/reader.h"
#include <stdint.h>

typedef struct memreader_t {
    char *buf;
    int owned;
    long len;
    long pos;
} memreader;

memreader *memreader_open(char *buf, long len);
memreader *memreader_open_from_reader(sd_reader *reader, int len);
void memreader_close(memreader *reader);
long memreader_size(const memreader *reader);
long memreader_pos(const memreader *reader);
void memreader_xor(memreader *reader, uint8_t key);

int memread_buf(memreader *reader, char *buf, int len);
uint8_t memread_ubyte(memreader *reader);
uint16_t memread_uword(memreader *reader);
uint32_t memread_udword(memreader *reader);
int8_t memread_byte(memreader *reader);
int16_t memread_word(memreader *reader);
int32_t memread_dword(memreader *reader);
float memread_float(memreader *reader);
void sd_mskip(memreader *reader, unsigned int nbytes);

#endif // MEMREADER_H
