/**
 * @file iterator.h
 * @brief Generic iterator interface for container types.
 * @details Provides a unified iterator interface used by various container types
 *          (array, hashmap, list, vector). The iterator stores function pointers
 *          for navigation, allowing each container to define its own traversal logic.
 */

#ifndef ITERATOR_H
#define ITERATOR_H

typedef struct iterator_t iterator;

/**
 * @brief Generic iterator structure.
 * @details Contains function pointers for navigation and state tracking.
 *          Initialize using container-specific functions (e.g., vector_iter_begin).
 */
struct iterator_t {
    const void *data;          ///< Pointer to the container being iterated
    void *vnow;                ///< Current element (container-specific)
    int inow;                  ///< Current index position
    int ended;                 ///< Non-zero if iteration has finished
    void *(*next)(iterator *); ///< Function to get next element
    void *(*prev)(iterator *); ///< Function to get previous element
    void *(*peek)(iterator *); ///< Function to peek at next element without advancing
};

/**
 * @brief Advance the iterator and return the next element.
 * @param iterator Iterator to advance
 * @return Pointer to the next element, or NULL if iteration ended
 */
void *iter_next(iterator *iterator);

/**
 * @brief Peek at the next element without advancing the iterator.
 * @param iterator Iterator to peek from
 * @return Pointer to the next element, or NULL if at the end
 */
void *iter_peek(iterator *iterator);

/**
 * @brief Move the iterator backwards and return the previous element.
 * @details Only works with iterators initialized with *_iter_end() functions.
 * @param iterator Iterator to move backwards
 * @return Pointer to the previous element, or NULL if at the beginning
 */
void *iter_prev(iterator *iterator);

/**
 * @brief Iterate forward through a container.
 * @details Usage: foreach(iter, item) { ... }
 * @param iterator An initialized iterator
 * @param item Variable to receive each element
 */
#define foreach(iterator, item) while((item = iter_next(&iterator)) != NULL)

/**
 * @brief Iterate backward through a container.
 * @details Usage: foreach_reverse(iter, item) { ... }
 *          Requires an iterator initialized with *_iter_end().
 * @param iterator An initialized iterator
 * @param item Variable to receive each element
 */
#define foreach_reverse(iterator, item) while((item = iter_prev(&iterator)) != NULL)

#endif // ITERATOR_H
