#ifndef _SERIAL_H
#define _SERIAL_H

#include <stddef.h>

typedef struct serial_t {
    size_t len;
    char *data;
} serial;

void serial_create(serial *s);
void serial_write(serial *s, char *buf, int len);
size_t serial_len(serial *s);
void serial_read(serial *s, char *buf, int len);
void serial_free(serial *s);

#endif // _SERIAL_H