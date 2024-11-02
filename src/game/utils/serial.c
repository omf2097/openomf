#include <stdlib.h>
#include <string.h>
#if defined(WIN32) || defined(_WIN32)
#include <winsock.h> // for htonl and friends
#else
#include <arpa/inet.h> // for htonl and friends
#endif
#include "game/utils/serial.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include <stdio.h>

#define SERIAL_BUF_RESIZE_INC 64

// taken from http://stackoverflow.com/questions/10620601/portable-serialisation-of-ieee754-floating-point-values
static uint32_t serial_htonf(float val) {
    uint32_t rep;
    memcpy(&rep, &val, sizeof rep);
    rep = htonl(rep);
    return rep;
}

static float serial_ntohf(uint32_t rep) {
    rep = ntohl(rep);
    float val;
    memcpy(&val, &rep, sizeof rep);
    return val;
}

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

void serial_write_int16(serial *s, int16_t v) {
    int16_t t = htons(v);
    serial_write(s, (char *)&t, sizeof(t));
}

void serial_write_int32(serial *s, int32_t v) {
    int32_t t = htonl(v);
    serial_write(s, (char *)&t, sizeof(t));
}

void serial_write_uint32(serial *s, uint32_t v) {
    uint32_t t = htonl(v);
    serial_write(s, (char *)&t, sizeof(t));
}

void serial_write_float(serial *s, float v) {
    uint32_t t = serial_htonf(v);
    serial_write(s, (char *)&t, sizeof(t));
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

int8_t serial_read_int8(serial *s) {
    int8_t v;
    serial_read(s, (char *)&v, sizeof(v));
    return v;
}

int16_t serial_read_int16(serial *s) {
    int16_t v;
    serial_read(s, (char *)&v, sizeof(v));
    return ntohs(v);
}

uint16_t serial_read_uint16(serial *s) {
    uint16_t v;
    serial_read(s, (char *)&v, sizeof(v));
    return ntohs(v);
}

int32_t serial_read_int32(serial *s) {
    int32_t v;
    serial_read(s, (char *)&v, sizeof(v));
    return ntohl(v);
}

uint32_t serial_read_uint32(serial *s) {
    uint32_t v;
    serial_read(s, (char *)&v, sizeof(v));
    return ntohl(v);
}

float serial_read_float(serial *s) {
    uint32_t v;
    serial_read(s, (char *)&v, sizeof(v));
    return serial_ntohf(v);
}
