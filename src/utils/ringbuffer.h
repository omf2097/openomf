#ifndef RINGBUFFER_H
#define RINGBUFFER_H

typedef struct ring_buffer {
    size_t size;
    size_t len;
    size_t w_pos;
    size_t r_pos;
    char *data;
} ring_buffer;

void rb_create(ring_buffer *rb, size_t size);
void rb_free(ring_buffer *rb);

size_t rb_write(ring_buffer *rb, const char *data, size_t len);
size_t rb_read(ring_buffer *rb, char *data, size_t len);
size_t rb_peek(const ring_buffer *rb, char *data, size_t len);
size_t rb_skip(ring_buffer *rb, size_t len);
size_t rb_length(const ring_buffer *rb);
size_t rb_size(const ring_buffer *rb);

#endif // RINGBUFFER_H
