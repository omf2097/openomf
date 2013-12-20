#include <stdlib.h>
#include <string.h>
#include "game/serial.h"

void serial_create(serial *s) {
    s->len = 0;
    s->data = NULL;
}

void serial_write(serial *s, char *buf, int len) {
    if(s->data == NULL) {
        s->data = malloc(len);
        memcpy(s->data, buf, len);
    } else {
        s->data = realloc(s->data, s->len + len);
        memcpy(s->data + s->len, buf, len);
    }
}

void serial_free(serial *s) {
    if(s->data != NULL) {
        free(s->data);
    }
}

size_t serial_len(serial *s) {
    return s->len;
}

void serial_read(serial *s, char *buf, int len) {
    if(len > s->len) {
        len = s->len;
    }
    memcpy(buf, s->data, len);
}