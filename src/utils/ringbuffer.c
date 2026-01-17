#include <string.h>

#include "utils/allocator.h"
#include "utils/ringbuffer.h"

void rb_create(ring_buffer *rb, size_t size) {
    memset(rb, 0, sizeof(ring_buffer));
    rb->size = size;
    rb->data = omf_calloc(1, size);
}

void rb_free(ring_buffer *rb) {
    if(rb == NULL) {
        return;
    }
    omf_free(rb->data);
}

size_t rb_write(ring_buffer *rb, const char *data, size_t len) {
    len = (len > (rb->size - rb->len)) ? (rb->size - rb->len) : len;
    if(len == 0) {
        return 0;
    }
    if(len + rb->w_pos > rb->size) {
        const size_t k = (len + rb->w_pos) % rb->size;
        memcpy((rb->data + rb->w_pos), data, len - k);
        memcpy(rb->data, data + (len - k), k);
    } else {
        memcpy((rb->data + rb->w_pos), data, len);
    }
    rb->len += len;
    rb->w_pos += len;
    if(rb->w_pos >= rb->size) {
        rb->w_pos = rb->w_pos % rb->size;
    }
    return len;
}

static void rb_read_data(const ring_buffer *rb, char *data, const size_t len) {
    if(len + rb->r_pos > rb->size) {
        const size_t k = (len + rb->r_pos) % rb->size;
        memcpy(data, rb->data + rb->r_pos, len - k);
        memcpy(data + (len - k), rb->data, k);
    } else {
        memcpy(data, rb->data + rb->r_pos, len);
    }
}

size_t rb_read(ring_buffer *rb, char *data, size_t len) {
    len = (len > rb->len) ? rb->len : len;
    if(len == 0) {
        return 0;
    }
    rb_read_data(rb, data, len);
    rb->len -= len;
    rb->r_pos += len;
    if(rb->r_pos >= rb->size) {
        rb->r_pos = rb->r_pos % rb->size;
    }
    return len;
}

size_t rb_peek(const ring_buffer *rb, char *data, size_t len) {
    len = (len > rb->len) ? rb->len : len;
    if(len == 0) {
        return 0;
    }
    rb_read_data(rb, data, len);
    return len;
}

size_t rb_skip(ring_buffer *rb, size_t len) {
    len = (len > rb->len) ? rb->len : len;
    if(len == 0) {
        return 0;
    }
    rb->len -= len;
    rb->r_pos += len;
    if(rb->r_pos >= rb->size) {
        rb->r_pos = rb->r_pos % rb->size;
    }
    return len;
}

size_t rb_length(const ring_buffer *rb) {
    return rb->len;
}

size_t rb_size(const ring_buffer *rb) {
    return rb->size;
}
