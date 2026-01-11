/**
 * @file list.h
 * @brief Doubly-linked list implementation.
 * @details A generic doubly-linked list that stores copies of data.
 *          Supports forward and backward iteration, and insertion/deletion at any position.
 */

#ifndef LIST_H
#define LIST_H

#include "iterator.h"
#include <stddef.h>

typedef struct list_node list_node;
typedef struct list list;

/**
 * @brief Callback function type for freeing node data.
 * @details Called when a node is removed from the list.
 */
typedef void (*list_node_free_cb)(void *data);

/**
 * @brief A node in the doubly-linked list.
 */
struct list_node {
    list_node *prev; ///< Previous node in the list
    list_node *next; ///< Next node in the list
    void *data;      ///< Pointer to the stored data
};

/**
 * @brief List container structure.
 */
struct list {
    list_node *first;       ///< First node in the list
    list_node *last;        ///< Last node in the list
    unsigned int size;      ///< Number of nodes in the list
    list_node_free_cb free; ///< Optional callback to free node data
};

/**
 * @brief Initialize an empty list.
 * @param list List structure to initialize
 */
void list_create(list *list);

/**
 * @brief Free all nodes in the list.
 * @details Calls the free callback (if set) on each node's data before freeing.
 * @param list List to free
 */
void list_free(list *list);

/**
 * @brief Add an element at the beginning of the list.
 * @details The data is copied into a newly allocated node.
 * @param list List to modify
 * @param ptr Pointer to data to copy
 * @param size Size of the data in bytes
 */
void list_prepend(list *list, const void *ptr, size_t size);

/**
 * @brief Add an element at the end of the list.
 * @details The data is copied into a newly allocated node.
 * @param list List to modify
 * @param ptr Pointer to data to copy
 * @param size Size of the data in bytes
 */
void list_append(list *list, const void *ptr, size_t size);

/**
 * @brief Delete the element at the current iterator position.
 * @details The iterator remains valid and points to the next/previous element.
 * @param list List to modify
 * @param iter Iterator pointing to the element to delete
 */
void list_delete(list *list, iterator *iter);

/**
 * @brief Initialize a forward iterator for the list.
 * @details Use iter_next() to advance through the list.
 * @param list List to iterate over
 * @param iter Iterator structure to initialize
 */
void list_iter_begin(const list *list, iterator *iter);

/**
 * @brief Initialize a reverse iterator for the list.
 * @details Use iter_prev() to move backwards through the list.
 * @param list List to iterate over
 * @param iter Iterator structure to initialize
 */
void list_iter_end(const list *list, iterator *iter);

/**
 * @brief Insert an element after the current iterator position.
 * @details The data is copied into a newly allocated node.
 * @param iter Iterator indicating insertion position
 * @param ptr Pointer to data to copy
 * @param size Size of the data in bytes
 */
void list_iter_append(iterator *iter, const void *ptr, size_t size);

/**
 * @brief Get the element at a given index.
 * @details This is O(n) - for frequent random access, use vector instead.
 * @param list List to query
 * @param i Index of the element (0-based)
 * @return Pointer to the element data, or NULL if index is out of bounds
 */
void *list_get(const list *list, unsigned int i);

/**
 * @brief Set the callback function for freeing node data.
 * @param list List to modify
 * @param cb Callback function to call when nodes are freed
 */
void list_set_node_free_cb(list *list, list_node_free_cb cb);

/**
 * @brief Get the number of elements in the list.
 * @param list List to query
 * @return Number of elements
 */
static inline unsigned int list_size(const list *list) {
    return list->size;
}

#endif // LIST_H
