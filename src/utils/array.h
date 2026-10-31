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
 * @brief Callback function type for freeing elements.
 * @details Called when an element is removed from the array. Note that the callback function must also free the memory
 *          used by the object (omf_free()) !
 */
typedef void (*array_free_cb)(void *);

/**
 * @brief Sparse array structure storing void pointers.
 * @details The array grows automatically when elements are set beyond current capacity.
 *          Unset elements are NULL and skipped during iteration.
 */
typedef struct array {
    unsigned int allocated_size; ///< Current allocated capacity
    unsigned int filled;         ///< Number of non-NULL entries
    void **data;                 ///< Array of void pointers
    array_free_cb free_cb;       ///< Optional callback to free elements
} array;

/**
 * @brief Initialize a new array.
 * @details Allocates initial storage for the array. Must be freed with array_free().
 * @param array Array structure to initialize
 */
void array_create(array *array);

/**
 * @brief Initialize a new array with a free callback.
 * @details The callback is called for each non-NULL element when the array is freed.
 * @param array Array structure to initialize
 * @param free_cb Callback function for freeing elements
 */
void array_create_cb(array *array, array_free_cb free_cb);

/**
 * @brief Initialize a new array with a specified initial capacity.
 * @param array Array structure to initialize
 * @param initial_size Initial capacity (number of slots)
 */
void array_create_with_size(array *array, unsigned int initial_size);

/**
 * @brief Initialize a new array with initial capacity and a free callback.
 * @param array Array structure to initialize
 * @param initial_size Initial capacity (number of slots)
 * @param free_cb Callback function to free elements
 */
void array_create_with_size_cb(array *array, unsigned int initial_size, array_free_cb free_cb);

/**
 * @brief Free all memory used by the array.
 * @details If a free callback was set, it is called on each non-NULL element first.
 *          Otherwise, only the array structure itself is freed, not the pointed-to data.
 * @param array Array to free
 */
void array_free(array *array);

/**
 * @brief Set a value at the given index.
 * @details If the index is beyond current capacity, the array grows automatically.
 *          Setting a NULL value decrements the filled count if there was a non-NULL value.
 *          Overwriting an existing value does not call the free callback; the caller is
 *          responsible for freeing any value being replaced.
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
 * @brief Delete the value at the given index.
 * @details If a free callback was set, it is called for the deleted element.
 * @param array Array to modify
 * @param key Index of the element to delete
 * @return 0 on success, 1 if the index is out of bounds or already unset
 */
int array_delete_at(array *array, unsigned int key);

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
