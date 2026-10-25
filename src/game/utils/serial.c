#include <stdlib.h>
#include <string.h>

#include "game/utils/serial.h"
#include "utils/allocator.h"
#include "utils/crash.h"
#include <stdio.h>

#define SERIAL_BUF_RESIZE_INC 64

void serial_create(serial *s) {
    s->len = SERIAL_BUF_RESIZE_INC;
    s->wpos = 0;
    s->rpos = 0;
    s->data = omf_calloc(s->len, 1);
}

void serial_create_from(serial *s, const char *buf, size_t len) {
    s->len = len + SERIAL_BUF_RESIZE_INC;
    s->wpos = len;
    s->rpos = 0;
    s->data = omf_calloc(s->len, 1);
    memcpy(s->data, buf, len);
}

void serial_copy(serial *dst, const serial *src) {
    dst->len = src->len;
    dst->wpos = src->wpos;
    dst->rpos = src->rpos;
    dst->data = omf_calloc(dst->len, 1);
    memcpy(dst->data, src->data, dst->len);
}

serial *serial_calloc_copy(const serial *src) {
    serial *dst = omf_calloc(1, sizeof(serial));
    serial_copy(dst, src);
    return dst;
}

void serial_write(serial *s, const char *buf, size_t len) {
    if(s->len < (s->wpos + len)) {
        size_t new_len = s->len + len + SERIAL_BUF_RESIZE_INC;
        s->data = omf_realloc(s->data, new_len);
        s->len = new_len;
    }

    memcpy(s->data + s->wpos, buf, len);
    s->wpos += len;
}

void serial_write_int8(serial *s, int8_t v) {
    serial_write(s, (char *)&v, sizeof(v));
}

void serial_write_uint8(serial *s, uint8_t v) {
    serial_write(s, (char *)&v, sizeof(v));
}

void serial_free(serial *s) {
    omf_free(s->data);
    s->len = 0;
    s->rpos = 0;
    s->wpos = 0;
}

size_t serial_len(serial *s) {
    return s->wpos;
}

void serial_read_reset(serial *s) {
    s->rpos = 0;
}

void serial_read(serial *s, char *buf, size_t len) {
    if(len + s->rpos > s->wpos) {
        len = s->wpos - s->rpos;
    }
    memcpy(buf, s->data + s->rpos, len);
    s->rpos += len;
}

void serial_write_str(serial *s, const str *src) {
    if(str_size(src) > 255) {
        crash("Invalid serial write -- string must be shorter than 256 bytes!");
    }
    serial_write_uint8(s, (uint8_t)str_size(src));
    serial_write(s, str_c(src), str_size(src));
}

void serial_write_bytes(serial *s, const uint8_t *buf, const size_t max) {
    for(size_t i = 0; i < max && buf[i]; i++) {
        serial_write_uint8(s, buf[i]);
    }
    serial_write_uint8(s, 0);
}

void serial_read_bytes(serial *s, uint8_t *buf, const size_t max) {
    size_t i = 0;
    uint8_t b;
    memset(buf, 0, max);
    while(s->rpos < s->wpos && (b = serial_read_uint8(s)) != 0) {
        if(i < max) {
            buf[i++] = b;
        }
    }
}

void serial_read_str(serial *s, str *dst) {
    const uint8_t len = serial_read_uint8(s);
    char buf[256 + 1];
    serial_read(s, buf, len);
    buf[len] = '\0';
    str_set_c(dst, buf);
}

int8_t serial_read_int8(serial *s) {
    int8_t v;
    serial_read(s, (char *)&v, sizeof(v));
    return v;
}
uint8_t serial_read_uint8(serial *s) {
    uint8_t v;
    serial_read(s, (char *)&v, sizeof(v));
    return v;
}

#if defined(BIG_ENDIAN_BUILD)

void serial_write_int16(serial *s, int16_t v) {
    uint16_t u = (uint16_t)v;
    uint8_t b[sizeof(v)];
    b[0] = (uint8_t)u;
    b[1] = (uint8_t)(u >> 8);
    serial_write(s, (char *)b, sizeof(b));
}

void serial_write_int32(serial *s, int32_t v) {
    uint32_t u = (uint32_t)v;
    uint8_t b[sizeof(v)];
    b[0] = (uint8_t)u;
    b[1] = (uint8_t)(u >> 8);
    b[2] = (uint8_t)(u >> 16);
    b[3] = (uint8_t)(u >> 24);
    serial_write(s, (char *)b, sizeof(b));
}

void serial_write_uint32(serial *s, uint32_t v) {
    uint8_t b[sizeof(v)];
    b[0] = (uint8_t)v;
    b[1] = (uint8_t)(v >> 8);
    b[2] = (uint8_t)(v >> 16);
    b[3] = (uint8_t)(v >> 24);
    serial_write(s, (char *)b, sizeof(b));
}

void serial_write_float(serial *s, float v) {
    uint32_t u;
    uint8_t b[sizeof(v)];
    memcpy(&u, &v, sizeof(u));
    b[0] = (uint8_t)u;
    b[1] = (uint8_t)(u >> 8);
    b[2] = (uint8_t)(u >> 16);
    b[3] = (uint8_t)(u >> 24);
    serial_write(s, (char *)b, sizeof(b));
}

int16_t serial_read_int16(serial *s) {
    uint8_t b[sizeof(int16_t)];
    serial_read(s, (char *)b, sizeof(b));
    return (int16_t)((uint16_t)b[0] | ((uint16_t)b[1] << 8));
}

uint16_t serial_read_uint16(serial *s) {
    uint8_t b[sizeof(uint16_t)];
    serial_read(s, (char *)b, sizeof(b));
    return (uint16_t)b[0] | ((uint16_t)b[1] << 8);
}

int32_t serial_read_int32(serial *s) {
    uint8_t b[sizeof(int32_t)];
    serial_read(s, (char *)b, sizeof(b));
    return (int32_t)((uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24));
}

uint32_t serial_read_uint32(serial *s) {
    uint8_t b[sizeof(uint32_t)];
    serial_read(s, (char *)b, sizeof(b));
    return (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}

float serial_read_float(serial *s) {
    uint8_t b[sizeof(float)];
    uint32_t u;
    float v;
    serial_read(s, (char *)b, sizeof(b));
    u = (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
    memcpy(&v, &u, sizeof(v));
    return v;
}

#else

void serial_write_int16(serial *s, int16_t v) {
    serial_write(s, (char *)&v, sizeof(v));
}

void serial_write_int32(serial *s, int32_t v) {
    serial_write(s, (char *)&v, sizeof(v));
}

void serial_write_uint32(serial *s, uint32_t v) {
    serial_write(s, (char *)&v, sizeof(v));
}

void serial_write_float(serial *s, float v) {
    serial_write(s, (char *)&v, sizeof(v));
}

int16_t serial_read_int16(serial *s) {
    int16_t v;
    serial_read(s, (char *)&v, sizeof(v));
    return v;
}

uint16_t serial_read_uint16(serial *s) {
    uint16_t v;
    serial_read(s, (char *)&v, sizeof(v));
    return v;
}

int32_t serial_read_int32(serial *s) {
    int32_t v;
    serial_read(s, (char *)&v, sizeof(v));
    return v;
}

uint32_t serial_read_uint32(serial *s) {
    uint32_t v;
    serial_read(s, (char *)&v, sizeof(v));
    return v;
}

float serial_read_float(serial *s) {
    float v;
    serial_read(s, (char *)&v, sizeof(v));
    return v;
}

#endif
