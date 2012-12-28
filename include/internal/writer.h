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

#endif // _WRITER_H
