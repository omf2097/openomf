/*
 * Header file for generic ringbuffer in ringbuffer.c.
 *
 * Copyright (c) 2011, Tuomas Virtanen
 * license: MIT; see LICENSE for details.
*/

#ifndef _RINGBUFFER_H
#define _RINGBUFFER_H

typedef struct {
    int size;
    int len;
    int wpos, rpos;
    char* data;
} ringbuffer;

ringbuffer* rb_create(int size);
void rb_delete(ringbuffer* rb);
int rb_write(ringbuffer *rb, char* data, int len);
int rb_read(ringbuffer *rb, char* data, int len);
int rb_peek(ringbuffer *rb, char* data, int len);
int rb_advance(ringbuffer *rb, int len);
int rb_length(ringbuffer *rb);
int rb_size(ringbuffer *rb);

#endif // _RINGBUFFER_H
