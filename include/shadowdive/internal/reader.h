#ifndef _SD_READER_H
#define _SD_READER_H

#include <stdint.h>

typedef struct sd_reader_t sd_reader;

sd_reader* sd_reader_open(const char *file);
void sd_reader_close(sd_reader *reader);
int sd_reader_ok(sd_reader *reader);

long sd_reader_pos(sd_reader *reader);
long sd_reader_filesize(sd_reader *reader);
int sd_reader_set(sd_reader *reader, long pos);

int sd_read_buf(sd_reader *reader, char *buf, int len);
int sd_peek_buf(sd_reader *reader, char *buf, int len);

uint8_t sd_read_ubyte(sd_reader *reader);
uint16_t sd_read_uword(sd_reader *reader);
uint32_t sd_read_udword(sd_reader *reader);
int8_t sd_read_byte(sd_reader *reader);
int16_t sd_read_word(sd_reader *reader);
int32_t sd_read_dword(sd_reader *reader);
float sd_read_float(sd_reader *reader);

uint8_t sd_peek_ubyte(sd_reader *reader);
uint16_t sd_peek_uword(sd_reader *reader);
uint32_t sd_peek_udword(sd_reader *reader);
int8_t sd_peek_byte(sd_reader *reader);
int16_t sd_peek_word(sd_reader *reader);
int32_t sd_peek_dword(sd_reader *reader);
float sd_peek_float(sd_reader *reader);

/**
  * Compare following nbytes amount of data and given buffer. Does not advance file pointer.
  */
int sd_match(sd_reader *reader, char *buf, unsigned int nbytes);

/**
  * Skip following nbytes amount of data.
  */
void sd_skip(sd_reader *reader, unsigned int nbytes);

#endif // _SD_READER_H
