#ifndef _SD_WRITER_H
#define _SD_WRITER_H

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
long sd_writer_pos(const sd_writer *writer);

int sd_writer_seek_start(const sd_writer *writer, long offset);
int sd_writer_seek_cur(const sd_writer *writer, long offset);
int sd_writer_seek_end(const sd_writer *writer, long offset);

/**
  * Write a buffer to file.
  */
int sd_write_buf(const sd_writer *writer, const char *buf, int len);

int sd_write_fprintf(const sd_writer *writer, const char *format, ...);

void sd_write_ubyte(const sd_writer *writer, uint8_t data);
void sd_write_uword(const sd_writer *writer, uint16_t data);
void sd_write_udword(const sd_writer *writer, uint32_t data);
void sd_write_byte(const sd_writer *writer, int8_t data);
void sd_write_word(const sd_writer *writer, int16_t data);
void sd_write_dword(const sd_writer *writer, int32_t data);
void sd_write_float(const sd_writer *writer, float data);

/**
 * Writes len bytes of filler content
 */
void sd_write_fill(const sd_writer *writer, char content, int len);

void sd_write_variable_str(const sd_writer *w, const char *str);

#endif // _SD_WRITER_H
