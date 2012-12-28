#ifndef _READER_H
#define _READER_H

#include <stdint.h>

typedef struct sd_reader_t sd_reader;

/**
  * Open file for reading.
  */
sd_reader* sd_reader_open(const char *file);

/**
  * Close file.
  */
void sd_reader_close(sd_reader *reader);

/**
  * Check if there is more data to be read.
  */
int sd_reader_ok(sd_reader *reader);

long sf_reader_pos(sd_reader *reader);

int sd_read_buf(sd_reader *reader, char *buf, int len);
int sd_peek_buf(sd_reader *reader, char *buf, int len);

uint8_t sd_read_ubyte(sd_reader *reader);
uint16_t sd_read_uword(sd_reader *reader);
uint32_t sd_read_udword(sd_reader *reader);
int8_t sd_read_byte(sd_reader *reader);
int16_t sd_read_word(sd_reader *reader);
int32_t sd_read_dword(sd_reader *reader);

uint8_t sd_peek_ubyte(sd_reader *reader);
uint16_t sd_peek_uword(sd_reader *reader);
uint32_t sd_peek_udword(sd_reader *reader);
int8_t sd_peek_byte(sd_reader *reader);
int16_t sd_peek_word(sd_reader *reader);
int32_t sd_peek_dword(sd_reader *reader);

/**
  * Compare following nbytes amount of data and given buffer. Does not advance file pointer.
  */
int sd_match(sd_reader *reader, char *buf, unsigned int nbytes);

/**
  * Skip following nbytes amount of data.
  */
void sd_skip(sd_reader *reader, unsigned int nbytes);

#endif // _READER_H
