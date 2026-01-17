/**
 * @file array.h
 * @brief Sparse array data structure with pointer storage.
 * @details Implements a dynamically growing sparse array that stores void pointers.
 *          Elements can be set at arbitrary indices, and the array automatically
 *          grows to accommodate them. NULL entries are skipped during iteration.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef ARRAY_H
#define ARRAY_H

#include "iterator.h"

/**
 * @brief Sparse array structure storing void pointers.
 * @details The array grows automatically when elements are set beyond current capacity.
 *          Unset elements are NULL and skipped during iteration.
 */
typedef struct array {
    unsigned int allocated_size; ///< Current allocated capacity
    unsigned int filled;         ///< Number of non-NULL entries
    void **data;                 ///< Array of void pointers
} array;

/**
 * @brief Initialize a new array.
 * @details Allocates initial storage for the array. Must be freed with array_free().
 * @param array Array structure to initialize
 */
void array_create(array *array);

/**
 * @brief Free all memory used by the array.
 * @details Does not free the pointed-to data, only the array structure itself.
 * @param array Array to free
 */
void array_free(array *array);

/**
 * @brief Set a value at the given index.
 * @details If the index is beyond current capacity, the array grows automatically.
 *          Setting a NULL value decrements the filled count if there was a non-NULL value.
 * @param array Array to modify
 * @param key Index at which to store the value
 * @param ptr Pointer value to store (can be NULL)
 */
void array_set(array *array, unsigned int key, const void *ptr);

/**
 * @brief Get the value at the given index.
 * @param array Array to read from
 * @param key Index to retrieve
 * @return The stored pointer, or NULL if index is out of bounds or unset
 */
void *array_get(const array *array, unsigned int key);

/**
 * @brief Initialize a forward iterator for the array.
 * @details The iterator skips NULL entries. Use iter_next() to advance.
 * @param array Array to iterate over
 * @param iterator Iterator structure to initialize
 */
void array_iter_begin(const array *array, iterator *iterator);

/**
 * @brief Initialize a reverse iterator for the array.
 * @details The iterator skips NULL entries. Use iter_prev() to move backwards.
 * @param array Array to iterate over
 * @param iterator Iterator structure to initialize
 */
void array_iter_end(const array *array, iterator *iterator);

#endif // ARRAY_H
