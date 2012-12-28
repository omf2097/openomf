#ifndef _WRITER_H
#define _WRITER_H

#include <stdint.h>

typedef struct sd_writer_t sd_writer;

/**
  * Open file for writing. If file exists, it will be overwritten.
  */
sd_writer* sd_writer_open(const char *file);

/**
  * Close file.
  */
void sd_writer_close(sd_writer *writer);

/**
  * Returns the position of the file pointer
  */
long sd_writer_pos(sd_writer *writer);

/**
  * Write a buffer to file.
  */
int sd_write_buf(sd_writer *writer, char *buf, int len);

void sd_write_ubyte(sd_writer *writer, uint8_t data);
void sd_write_uword(sd_writer *writer, uint16_t data);
void sd_write_udword(sd_writer *writer, uint32_t data);
void sd_write_byte(sd_writer *writer, int8_t data);
void sd_write_word(sd_writer *writer, int16_t data);
void sd_write_dword(sd_writer *writer, int32_t data);

#endif // _WRITER_H
