#include <stdlib.h>
#include <string.h>
#include "game/serial.h"

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

// TODO: Add conversions (htons, etc.)!

void serial_write_int(serial *s, int v) {
    serial_write(s, (char*)&v, sizeof(v));
}

void serial_write_long(serial *s, long v) {
    serial_write(s, (char*)&v, sizeof(v));
}

void serial_write_float(serial *s, float v) {
    serial_write(s, (char*)&v, sizeof(v));
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

int serial_read_int(serial *s) {
    int v;
    serial_read(s, (char*)&v, sizeof(v));
    return v;
}

long serial_read_long(serial *s) {
    long v;
    serial_read(s, (char*)&v, sizeof(v));
    return v;
}

float serial_read_float(serial *s) {
    float v;
    serial_read(s, (char*)&v, sizeof(v));
    return v;
}
