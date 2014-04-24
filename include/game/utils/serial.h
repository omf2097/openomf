#ifndef _SERIAL_H
#define _SERIAL_H

#include <stddef.h>
#include <stdint.h>

typedef struct serial_t {
    size_t len;
    size_t rpos;
    char *data;
} serial;

void serial_create(serial *s);
void serial_write(serial *s, const char *buf, int len);
void serial_write_int8(serial *s, int8_t v);
void serial_write_int16(serial *s, int16_t v);
void serial_write_int32(serial *s, int32_t v);
//void serial_write_int64(serial *s, int64_t v);
void serial_write_float(serial *s, float v);
size_t serial_len(serial *s);
void serial_read(serial *s, char *buf, int len);
void serial_free(serial *s);
void serial_read_reset(serial *s);
int8_t serial_read_int8(serial *s);
int16_t serial_read_int16(serial *s);
int32_t serial_read_int32(serial *s);
//int64_t serial_read_int64(serial *s);
long serial_read_long(serial *s);
float serial_read_float(serial *s);

#endif // _SERIAL_H
