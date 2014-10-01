#ifndef _SD_READER_H
#define _SD_READER_H

#include <stdint.h>

typedef struct sd_reader_t sd_reader;

sd_reader* sd_reader_open(const char *file);
void sd_reader_close(sd_reader *reader);
int sd_reader_ok(const sd_reader *reader);

long sd_reader_pos(const sd_reader *reader);
long sd_reader_filesize(const sd_reader *reader);
int sd_reader_set(const sd_reader *reader, long pos);

int sd_read_buf(const sd_reader *reader, char *buf, int len);
int sd_peek_buf(const sd_reader *reader, char *buf, int len);

uint8_t sd_read_ubyte(const sd_reader *reader);
uint16_t sd_read_uword(const sd_reader *reader);
uint32_t sd_read_udword(const sd_reader *reader);
int8_t sd_read_byte(const sd_reader *reader);
int16_t sd_read_word(const sd_reader *reader);
int32_t sd_read_dword(const sd_reader *reader);
float sd_read_float(const sd_reader *reader);

uint8_t sd_peek_ubyte(const sd_reader *reader);
uint16_t sd_peek_uword(const sd_reader *reader);
uint32_t sd_peek_udword(const sd_reader *reader);
int8_t sd_peek_byte(const sd_reader *reader);
int16_t sd_peek_word(const sd_reader *reader);
int32_t sd_peek_dword(const sd_reader *reader);
float sd_peek_float(const sd_reader *reader);

int sd_read_scan(const sd_reader *reader, const char* format, ...);
int sd_read_line(const sd_reader *reader, char *buffer, int maxlen);

/**
  * Compare following nbytes amount of data and given buffer. Does not advance file pointer.
  */
int sd_match(const sd_reader *reader, char *buf, unsigned int nbytes);

/**
  * Skip following nbytes amount of data.
  */
void sd_skip(const sd_reader *reader, unsigned int nbytes);

char* sd_read_variable_str(const sd_reader *r);

#endif // _SD_READER_H
