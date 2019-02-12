#ifndef _SD_MEMREADER_H
#define _SD_MEMREADER_H

#include <stdint.h>
#include "shadowdive/internal/reader.h"

typedef struct sd_mreader_t {
    char *buf;
    int owned;
    long len;
    long pos;
} sd_mreader;

sd_mreader* sd_mreader_open(char *buf, long len);
sd_mreader* sd_mreader_open_from_reader(sd_reader *reader, int len);
void sd_mreader_close(sd_mreader *reader);
long sd_mreader_size(const sd_mreader *reader);
long sd_mreader_pos(const sd_mreader *reader);
void sd_mreader_xor(sd_mreader *reader, uint8_t key);

int sd_mread_buf(sd_mreader *reader, char *buf, int len);
uint8_t sd_mread_ubyte(sd_mreader *reader);
uint16_t sd_mread_uword(sd_mreader *reader);
uint32_t sd_mread_udword(sd_mreader *reader);
int8_t sd_mread_byte(sd_mreader *reader);
int16_t sd_mread_word(sd_mreader *reader);
int32_t sd_mread_dword(sd_mreader *reader);
float sd_mread_float(sd_mreader *reader);
void sd_mskip(sd_mreader *reader, unsigned int nbytes);

#endif // _SD_MEMREADER_H
