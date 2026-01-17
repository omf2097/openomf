/**
 * @file ringbuffer.h
 * @brief Ring buffer implementation.
 * @details A fixed-size ring buffer for byte data. When the buffer is full, writes will fail (no automatic
 * overwriting).
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stddef.h>

/**
 * @brief Ring buffer structure.
 */
typedef struct ring_buffer {
    size_t size;  ///< Total allocated size of the buffer
    size_t len;   ///< Current number of bytes in the buffer
    size_t w_pos; ///< Write position (next write location)
    size_t r_pos; ///< Read position (next read location)
    char *data;   ///< Pointer to the buffer data
} ring_buffer;

/**
 * @brief Create a ring buffer with the specified size.
 * @param rb Ring buffer structure to initialize
 * @param size Size of the buffer in bytes
 */
void rb_create(ring_buffer *rb, size_t size);

/**
 * @brief Free the ring buffer
 * @param rb Ring buffer to free
 */
void rb_free(ring_buffer *rb);

/**
 * @brief Write data to the ring buffer.
 * @details Writes as much data as will fit. Does not overwrite existing data.
 * @param rb Ring buffer to write to
 * @param data Data to write
 * @param len Number of bytes to write
 * @return Number of bytes actually written (may be less than len if buffer is full)
 */
size_t rb_write(ring_buffer *rb, const char *data, size_t len);

/**
 * @brief Read and remove data from the ring buffer.
 * @details Reads up to len bytes and removes them from the buffer.
 * @param rb Ring buffer to read from
 * @param data Buffer to receive the data
 * @param len Maximum number of bytes to read
 * @return Number of bytes actually read
 */
size_t rb_read(ring_buffer *rb, char *data, size_t len);

/**
 * @brief Peek at data in the ring buffer without removing it.
 * @param rb Ring buffer to peek from
 * @param data Buffer to receive the data
 * @param len Maximum number of bytes to peek
 * @return Number of bytes actually copied
 */
size_t rb_peek(const ring_buffer *rb, char *data, size_t len);

/**
 * @brief Discard data from the ring buffer.
 * @param rb Ring buffer to modify
 * @param len Number of bytes to skip
 * @return Number of bytes actually skipped
 */
size_t rb_skip(ring_buffer *rb, size_t len);

/**
 * @brief Get the number of bytes currently stored in the buffer.
 * @param rb Ring buffer to query
 * @return Number of bytes available for reading
 */
size_t rb_length(const ring_buffer *rb);

/**
 * @brief Get the total capacity of the ring buffer.
 * @param rb Ring buffer to query
 * @return Total size of the buffer in bytes
 */
size_t rb_size(const ring_buffer *rb);

#endif // RINGBUFFER_H
