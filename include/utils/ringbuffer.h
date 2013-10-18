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

/**
  * Creates a new ringbuffer with the given size.
  * @param size Size for the new ringbuffer
  * @return Ringbuffer handle
  */
void rb_create(ringbuffer *rb, int size);

/**
  * Deletes the given ringbuffer.
  * @param rb Ringbuffer to be deleted.
  */
void rb_free(ringbuffer *rb);

/**
  * Writes to the given ringbuffer. If given length is larger than the amount
  * the ringbuffer can fit, only the data that fits will be written.
  * @param rb Ringbuffer to write to.
  * @param data Data to write
  * @param len Data length
  * @return Amount of data that was actually written.
  */
int rb_write(ringbuffer *rb, char* data, int len);

/**
  * Reads data from ringbuffer. If ringbuffer has less data than was requested,
  * only the available data will be read.
  * @param rb Ringbuffer to read from.
  * @param data Buffer to read into.
  * @param len How much data do we want
  * @return Amount of data that was actually read.
  */
int rb_read(ringbuffer *rb, char* data, int len);

/**
  * Peeks into the given ringbuffer. Technically same as rb_read, but does not advance
  * the internal position pointer. In other words, you may peek as many times as you wish,
  * and will always get the same data.
  * @param rb Ringbuffer to peek into.
  * @param data Buffer to read into
  * @param len How much data do we need
  * @return Amount of data actually read
  */
int rb_peek(ringbuffer *rb, char* data, int len);

/**
  * Advances the internal position counter by given amount. Note that counter can only be
  * advanced by the amount of unreadable data in ringbuffer.
  * @param rb Ringbuffer to handle
  * @param len How much should the position counter be increased
  * @return How much the position counter was actually increased.
  */
int rb_advance(ringbuffer *rb, int len);

/**
  * Returns the current length of the Ringbuffer. In other words, how much data
  * the ringbuffer contains
  * @param rb Ringbuffer to handle
  * @return Data in ringbuffer (in bytes).
  */
int rb_length(ringbuffer *rb);


/**
  * Returns the size of the ringbuffer. In other words, the maximum amount of data
  * the ringbuffer can hold.
  * @param rb Ringbuffer to handle
  * @return Size of the ringbuffer
  */
int rb_size(ringbuffer *rb);

#endif // _RINGBUFFER_H
