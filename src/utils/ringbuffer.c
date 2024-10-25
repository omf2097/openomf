#include <stdlib.h>
#include <string.h>

#include "utils/allocator.h"
#include "utils/ringbuffer.h"

/**
 * Creates a new ringbuffer with the given size.
 *
 * @param size Size for the new ringbuffer
 * @return Ringbuffer handle
 */
void rb_create(ring_buffer *rb, size_t size) {
    memset(rb, 0, sizeof(ring_buffer));
    rb->size = size;
    rb->data = omf_calloc(1, size);
}

/**
 * Frees the given ringbuffer.
 *
 * @param rb Ringbuffer to be deleted.
 */
void rb_free(ring_buffer *rb) {
    if(rb == NULL)
        return;
    omf_free(rb->data);
}

/**
 * Writes to the given ringbuffer. If given length is larger than the amount
 * the ringbuffer can fit, only the data that fits will be written.
 *
 * @param rb Ringbuffer to write to.
 * @param data Data to write
 * @param len Data length
 * @return Amount of data that was actually written.
 */
size_t rb_write(ring_buffer *rb, const char *data, size_t len) {
    int k;
    len = (len > (rb->size - rb->len)) ? (rb->size - rb->len) : len;
    if(rb->len < rb->size) {
        if(len + rb->w_pos > rb->size) {
            k = (len + rb->w_pos) % rb->size;
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
    return 0;
}

static void rb_read_data(const ring_buffer *rb, char *data, const size_t len) {
    int k;
    if(len + rb->r_pos > rb->size) {
        k = (len + rb->r_pos) % rb->size;
        memcpy(data, rb->data + rb->r_pos, len - k);
        memcpy(data + (len - k), rb->data, k);
    } else {
        memcpy(data, rb->data + rb->r_pos, len);
    }
}

/**
 * Reads data from ringbuffer. If ringbuffer has less data than was requested,
 * only the available data will be read.
 *
 * @param rb Ringbuffer to read from.
 * @param data Buffer to read into.
 * @param len How much data do we want
 * @return Amount of data that was actually read.
 */
size_t rb_read(ring_buffer *rb, char *data, size_t len) {
    len = (len > rb->len) ? rb->len : len;
    if(rb->len > 0) {
        rb_read_data(rb, data, len);
        rb->len -= len;
        rb->r_pos += len;
        if(rb->r_pos >= rb->size) {
            rb->r_pos = rb->r_pos % rb->size;
        }
        return len;
    }
    return 0;
}

/**
 * Peeks into the given ringbuffer. Technically same as rb_read, but does not advance
 * the internal position pointer. In other words, you may peek as many times as you wish,
 * and will always get the same data.
 *
 * @param rb Ringbuffer to peek into.
 * @param data Buffer to read into
 * @param len How much data do we need
 * @return Amount of data actually read
 */
size_t rb_peek(const ring_buffer *rb, char *data, size_t len) {
    len = (len > rb->len) ? rb->len : len;
    if(rb->len > 0) {
        rb_read_data(rb, data, len);
        return len;
    }
    return 0;
}

/**
 * Advances the internal position counter by given amount. Note that counter can only be
 * advanced by the amount of unreadable data in ringbuffer.
 *
 * @param rb Ringbuffer to handle
 * @param len How much should the position counter be increased
 * @return How much the position counter was actually increased.
 */
size_t rb_skip(ring_buffer *rb, size_t len) {
    len = (len > rb->len) ? rb->len : len;
    if(rb->len > 0) {
        rb->len -= len;
        rb->r_pos += len;
        if(rb->r_pos >= rb->size) {
            rb->r_pos = rb->r_pos % rb->size;
        }
        return len;
    }
    return 0;
}

/**
 * Returns the current length of the Ringbuffer. In other words, how much data
 * the ringbuffer contains
 *
 * @param rb Ringbuffer to handle
 * @return Data in ringbuffer (in bytes).
 */
size_t rb_length(const ring_buffer *rb) {
    return rb->len;
}

/**
 * Returns the size of the ringbuffer. In other words, the maximum amount of data
 * the ringbuffer can hold.
 *
 * @param rb Ringbuffer to handle
 * @return Size of the ringbuffer
 */
size_t rb_size(const ring_buffer *rb) {
    return rb->size;
}
