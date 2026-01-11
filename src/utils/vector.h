/**
 * @file vector.h
 * @brief Vector array implementation.
 * @details A generic dynamic array that stores fixed-size elements.
 *          Elements are stored contiguously in memory and the array grows
 *          automatically when capacity is exceeded. Supports forward and
 *          backward iteration.
 */

#ifndef VECTOR_H
#define VECTOR_H

#include "iterator.h"

/**
 * @brief Callback function type for freeing elements.
 * @details Called when an element is removed from the vector.
 */
typedef void (*vector_free_cb)(void *);

/**
 * @brief Dynamic array container structure.
 */
typedef struct vector {
    char *data;              ///< Pointer to the contiguous element storage
    unsigned int block_size; ///< Size of each element in bytes
    unsigned int blocks;     ///< Number of elements currently stored
    unsigned int reserved;   ///< Total capacity (number of elements that fit)
    vector_free_cb free_cb;  ///< Optional callback to free elements
} vector;

/**
 * @brief Comparison function type for sorting.
 * @details Should return negative if a < b, zero if a == b, positive if a > b.
 */
typedef int (*vector_compare_func)(const void *, const void *);

/**
 * @brief Create a new vector with the default initial capacity.
 * @param vector Vector structure to initialize
 * @param block_size Size of each element in bytes
 */
void vector_create(vector *vector, unsigned int block_size);

/**
 * @brief Create a new vector with a free callback.
 * @details The callback is called when elements are removed or the vector is freed.
 * @param vector Vector structure to initialize
 * @param block_size Size of each element in bytes
 * @param free_cb Callback function to free elements
 */
void vector_create_cb(vector *vector, unsigned int block_size, vector_free_cb free_cb);

/**
 * @brief Create a new vector with a specified initial capacity.
 * @param vector Vector structure to initialize
 * @param block_size Size of each element in bytes
 * @param initial_size Initial capacity (number of elements)
 */
void vector_create_with_size(vector *vector, unsigned int block_size, unsigned int initial_size);

/**
 * @brief Create a new vector with initial capacity and free callback.
 * @param vector Vector structure to initialize
 * @param block_size Size of each element in bytes
 * @param initial_size Initial capacity (number of elements)
 * @param free_cb Callback function to free elements
 */
void vector_create_with_size_cb(vector *vector, unsigned int block_size, unsigned int initial_size,
                                vector_free_cb free_cb);

/**
 * @brief Create a deep copy of a vector.
 * @details Copies all elements to a new vector. The free callback is also copied.
 * @param dst Destination vector (will be initialized)
 * @param src Source vector to copy
 */
void vector_clone(vector *dst, const vector *src);

/**
 * @brief Free all memory used by the vector.
 * @details Calls the free callback (if set) on each element before freeing.
 * @param vector Vector to free
 */
void vector_free(vector *vector);

/**
 * @brief Get a pointer to the element at the given index.
 * @param vector Vector to query
 * @param key Index of the element (0-based)
 * @return Pointer to the element, or NULL if index is out of bounds
 */
void *vector_get(const vector *vector, unsigned int key);

/**
 * @brief Set the value of an existing element.
 * @details The element at the given index is overwritten with the new value.
 * @param vector Vector to modify
 * @param key Index of the element to set
 * @param value Pointer to the value to copy
 * @return 0 on success, 1 if index is out of bounds
 */
int vector_set(vector *vector, unsigned int key, const void *value);

/**
 * @brief Append an element to the end of the vector.
 * @details The vector grows automatically if necessary.
 * @param vector Vector to modify
 * @param value Pointer to the value to copy
 */
void vector_append(vector *vector, const void *value);

/**
 * @brief Append space for an element and return a pointer to it.
 * @details Useful for constructing elements in-place.
 * @param vec Vector to modify
 * @return Pointer to the new (uninitialized) element
 */
void *vector_append_ptr(vector *vec);

/**
 * @brief Sort the vector using the given comparison function.
 * @details Uses qsort internally.
 * @param vector Vector to sort
 * @param cf Comparison function
 */
void vector_sort(vector *vector, vector_compare_func cf);

/**
 * @brief Delete the element at the given index.
 * @details Elements after the deleted one are shifted down.
 * @param vec Vector to modify
 * @param index Index of the element to delete
 * @return 0 on success, 1 on error
 */
int vector_delete_at(vector *vec, unsigned index);

/**
 * @brief Delete an element by swapping with the last element.
 * @details Faster than vector_delete_at() but does not preserve order.
 * @param vec Vector to modify
 * @param index Index of the element to delete
 * @return 0 on success, 1 on error
 */
int vector_swapdelete_at(vector *vec, unsigned index);

/**
 * @brief Delete the element at the current iterator position.
 * @details The iterator remains valid after deletion.
 * @param vector Vector to modify
 * @param iterator Iterator pointing to the element to delete
 * @return 0 on success, 1 on error
 */
int vector_delete(vector *vector, iterator *iterator);

/**
 * @brief Remove the last element from the vector.
 * @param vector Vector to modify
 */
void vector_pop(vector *vector);

/**
 * @brief Get a pointer to the last element.
 * @param vector Vector to query
 * @return Pointer to the last element, or NULL if empty
 */
void *vector_back(const vector *vector);

/**
 * @brief Initialize a forward iterator for the vector.
 * @details Use iter_next() to advance through the vector.
 * @param vector Vector to iterate over
 * @param iter Iterator structure to initialize
 */
void vector_iter_begin(const vector *vector, iterator *iter);

/**
 * @brief Initialize a reverse iterator for the vector.
 * @details Use iter_prev() to move backwards through the vector.
 * @param vector Vector to iterate over
 * @param iter Iterator structure to initialize
 */
void vector_iter_end(const vector *vector, iterator *iter);

/**
 * @brief Get the number of elements in the vector.
 * @param vec Vector to query
 * @return Number of elements
 */
static inline unsigned int vector_size(const vector *vec) {
    return vec->blocks;
}

/**
 * @brief Remove all elements from the vector.
 * @details Calls the free callback (if set) on each element.
 *          The capacity remains unchanged.
 * @param vec Vector to clear
 */
void vector_clear(vector *vec);

#endif // VECTOR_H
