/**
 * @file smallbuffer.h
 * @brief A container for arbitrary bytes.
 * @details Tracks capacity, supports small buffer optimization.
 * @copyright MIT License
 * @date 2025
 * @author Magnus Larsen
 */

#ifndef SMALLBUFFER_H
#define SMALLBUFFER_H

#include <stddef.h>
#include <string.h>

typedef struct smallbuffer {
    // private fields, do not access.
    ptrdiff_t priv_cap;
    union {
        char *priv_alloc;
        char priv_inline[sizeof(char *)];
    };
} smallbuffer;

/**
 * @brief Create an empty smallbuffer
 *
 * @details Alternatively, one can memset the structure to zero,
 * which also creates a valid empty smallbuffer.
 */
inline static void smallbuffer_create(smallbuffer *sb) {
    sb->priv_cap = 0;
}

/**
 * @brief Frees a smallbuffer, leaving its capacity at zero.
 * @details The smallbuffer is left in a valid state (empty), so this function is idempotent.
 */
void smallbuffer_free(smallbuffer *);

/**
 * @brief Changes the capacity of a smallbuffer.
 * @details Existing buffer contents will be preserved insofar as they fit,
 *    and padded with trailing nul bytes as required.
 * @param capacity desired capacity (can be zero).
 */
void smallbuffer_realloc(smallbuffer *, size_t capacity);

/**
 * @brief Changes the capacity of a smallbuffer.
 * @details Existing buffer contents will be preserved insofar as they fit,
 *    and padded with trailing nul bytes as required.
 * @param capacity desired capacity
 * @param sizeof_smallbuffer The amount of bytes allocated for the smallbuffer.
 *     Must be greater than the real sizeof(smallbuffer).
 */
void smallbuffer_realloc_with_custom_selfsize(smallbuffer *, size_t capacity, size_t sizeof_smallbuffer);

/**
 * @brief Access the contents (data pointer) of the smallbuffer.
 */
char *smallbuffer_data(smallbuffer *);

/**
 * @brief Returns the capacity of the smallbuffer, as set by smallbuffer_realloc.
 */
size_t smallbuffer_capacity(smallbuffer *);

#endif // SMALLBUFFER_H
