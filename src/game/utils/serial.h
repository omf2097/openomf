#ifndef SERIAL_H
#define SERIAL_H

#include <stddef.h>
#include <stdint.h>

typedef struct serial_t {
    size_t len;
    size_t rpos;
    size_t wpos;
    char *data;
} serial;

void serial_create(serial *s);
void serial_create_from(serial *s, const char *buf, size_t len);
void serial_write(serial *s, const char *buf, size_t len);
void serial_write_int8(serial *s, int8_t v);
void serial_write_int16(serial *s, int16_t v);
void serial_write_int32(serial *s, int32_t v);
void serial_write_uint32(serial *s, uint32_t v);
void serial_write_float(serial *s, float v);
size_t serial_len(serial *s);
void serial_read(serial *s, char *buf, size_t len);
void serial_free(serial *s);
void serial_read_reset(serial *s);
int8_t serial_read_int8(serial *s);
int16_t serial_read_int16(serial *s);
uint16_t serial_read_uint16(serial *s);
int32_t serial_read_int32(serial *s);
uint32_t serial_read_uint32(serial *s);
long serial_read_long(serial *s);
float serial_read_float(serial *s);
void serial_copy(serial *dst, const serial *src);
serial *serial_calloc_copy(const serial *src);

#endif // SERIAL_H
