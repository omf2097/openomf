/**
 * @brief Just a dumb buffer object
 */

#ifndef BLOB_H
#define BLOB_H

#include <stddef.h>

typedef struct blob {
    unsigned char *data;
    size_t size;
} blob;

/**
 * Allocate the blob object and initialize its contents.
 * @return Mew initialized blob object
 * @param size How much space to preallocate for the data
 */
blob *blob_create(size_t size);

/**
 * Resize the blob object contents to the new size
 * @param b Existing blob object
 * @param size New size of the buffer
 */
void blob_resize(blob *b, size_t size);

/**
 * Set blob contents, and resize as necessary.
 * @param b Data blob to modify
 * @param src Data to set
 * @param len Length of data
 */
void blob_set(blob *b, const char *src, size_t len);

/**
 * Free the blob object and its contents
 * @param b Allocated blob object
 */
void blob_free(blob *b);

#endif // BLOB_H
