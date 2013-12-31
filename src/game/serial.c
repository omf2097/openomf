#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h> // for htonl and friends
#include "game/serial.h"
#include "utils/log.h"
#include <stdio.h>


// taken from http://stackoverflow.com/questions/10620601/portable-serialisation-of-ieee754-floating-point-values
float htonf(float val) {
    uint32_t rep;
    memcpy(&rep, &val, sizeof rep);
    rep = htonl(rep);
    memcpy(&val, &rep, sizeof rep);
    return val;
}

float ntohf(float val) {
    uint32_t rep;
    memcpy(&rep, &val, sizeof rep);
    rep = ntohl(rep);
    memcpy(&val, &rep, sizeof rep);
    return val;
}

// TODO we should probably have some reasonable initial size, so we don't realloc like crazy
void serial_create(serial *s) {
    s->len = 0;
    s->rpos = 0;
    s->data = NULL;
}

// TODO: Optimize writing
void serial_write(serial *s, const char *buf, int len) {
    if(s->data == NULL) {
        s->data = malloc(len);
        memcpy(s->data, buf, len);
    } else {
        s->data = realloc(s->data, s->len + len);
        memcpy(s->data + s->len, buf, len);
    }
    s->len += len;
}

void serial_write_int8(serial *s, int8_t v) {
    serial_write(s, (char*)&v, sizeof(v));
}

void serial_write_int16(serial *s, int16_t v) {
    int16_t t = htons(v);
    serial_write(s, (char*)&t, sizeof(t));
}

void serial_write_int32(serial *s, int32_t v) {
    int32_t t = htonl(v);
    serial_write(s, (char*)&t, sizeof(t));
}

void serial_write_float(serial *s, float v) {
    float t = htonf(v);
    serial_write(s, (char*)&t, sizeof(t));
}

void serial_free(serial *s) {
    if(s->data != NULL) {
        free(s->data);
        s->data = NULL;
        s->len = 0;
        s->rpos = 0;
    }
}

size_t serial_len(serial *s) {
    return s->len;
}

void serial_read_reset(serial *s) {
    s->rpos = 0;
}

void serial_read(serial *s, char *buf, int len) {
    if(len + s->rpos  > s->len) {
        len = s->len - s->rpos;
    }
    memcpy(buf, s->data+s->rpos, len);
    s->rpos += len;
    if (s->rpos > s->len) {
        s->rpos = s->len;
    }
}

int8_t serial_read_int8(serial *s) {
    int8_t v;
    serial_read(s, (char*)&v, sizeof(v));
    return v;
}

int16_t serial_read_int16(serial *s) {
    int16_t v;
    serial_read(s, (char*)&v, sizeof(v));
    return ntohs(v);
}

int32_t serial_read_int32(serial *s) {
    int32_t v;
    serial_read(s, (char*)&v, sizeof(v));
    return ntohl(v);
}

float serial_read_float(serial *s) {
    float v;
    serial_read(s, (char*)&v, sizeof(v));
    return ntohf(v);
}
