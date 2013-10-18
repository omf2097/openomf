/*
 * Generic ringbuffer
 *
 * Copyright (c) 2011, Tuomas Virtanen
 * license: MIT; see LICENSE for details.
*/

#include "utils/ringbuffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void rb_create(ringbuffer* rb, int size) {
    rb->size = size;
    rb->len = 0;
    rb->wpos = 0;
    rb->rpos = 0;
    rb->data = malloc(size);
}

void rb_free(ringbuffer* rb) {
    if(rb == 0) return;
    free(rb->data);
}

int rb_write(ringbuffer *rb, char* data, int len) {
    int k;
    len = (len > (rb->size - rb->len)) ? (rb->size - rb->len) : len;
    if(rb->len < rb->size) {
        if(len + rb->wpos > rb->size) {
            k = (len + rb->wpos) % rb->size;
            memcpy((rb->data + rb->wpos), data, len - k);
            memcpy(rb->data, data+(len-k), k);
        } else {
            memcpy((rb->data + rb->wpos), data, len);
        }
        rb->len += len;
        rb->wpos += len;
        if(rb->wpos >= rb->size) {
            rb->wpos = rb->wpos % rb->size;
        }
        return len;
    }
    return 0;
}

int rb_read(ringbuffer *rb, char* data, int len) {
    int k;
    len = (len > rb->len) ? rb->len : len;
    if(rb->len > 0) {
        if(len + rb->rpos > rb->size) {
            k = (len + rb->rpos) % rb->size;
            memcpy(data, (rb->data + rb->rpos), len-k);
            memcpy(data+(len-k), (rb->data), k);
        } else {
            memcpy(data, (rb->data + rb->rpos), len);
        }
        rb->len -= len;
        rb->rpos += len;
        if(rb->rpos >= rb->size) {
            rb->rpos = rb->rpos % rb->size;
        }
        return len;
    }
    return 0;
}

int rb_peek(ringbuffer *rb, char* data, int len) {
    int k;
    len = (len > rb->len) ? rb->len : len;
    if(rb->len > 0) {
        if(len + rb->rpos > rb->size) {
            k = (len + rb->rpos) % rb->size;
            memcpy(data, (rb->data + rb->rpos), len-k);
            memcpy(data+(len-k), (rb->data), k);
        } else {
            memcpy(data, (rb->data + rb->rpos), len);
        }
        return len;
    }
    return 0;
}

int rb_advance(ringbuffer *rb, int len) {
    len = (len > rb->len) ? rb->len : len;
    if(rb->len > 0) {
        rb->len -= len;
        rb->rpos += len;
        if(rb->rpos >= rb->size) {
            rb->rpos = rb->rpos % rb->size;
        }
        return len;
    }
    return 0;
}

int rb_length(ringbuffer *rb) {
    return rb->len;
}

int rb_size(ringbuffer *rb) {
    return rb->size;
}
