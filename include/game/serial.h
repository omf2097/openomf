#ifndef _SERIAL_H
#define _SERIAL_H

#include <stddef.h>

typedef struct serial_t {
    size_t len;
    size_t rpos;
    char *data;
} serial;

void serial_create(serial *s);
void serial_write(serial *s, const char *buf, int len);
void serial_write_int(serial *s, int v);
void serial_write_long(serial *s, long v);
void serial_write_float(serial *s, float v);
size_t serial_len(serial *s);
void serial_read(serial *s, char *buf, int len);
void serial_free(serial *s);
void serial_read_reset(serial *s);
int serial_read_int(serial *s);
long serial_read_long(serial *s);
float serial_read_float(serial *s);

#endif // _SERIAL_H