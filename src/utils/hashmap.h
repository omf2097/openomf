/**
 * @file hashmap.h
 * @brief Generic hashmap implementation.
 * @details A hash table that maps arbitrary binary keys to arbitrary binary values.
 *          Uses FNV-1a hashing and separate chaining for collision resolution.
 *          The hashmap automatically resizes when load becomes high.
 */

#ifndef HASHMAP_H
#define HASHMAP_H

#include "utils/iterator.h"
#include <string.h>

typedef struct hashmap_pair hashmap_pair;
typedef struct hashmap_node hashmap_node;
typedef struct hashmap hashmap;

/**
 * @brief Callback function type for cleaning up values.
 * @details Called when a value is removed from the hashmap (during delete, clear, or free).
 *          This callback should clean up any resources owned by the value (e.g. nested
 *          pointers, file handles), but must NOT free the value pointer itself - the
 *          hashmap will free the value storage after the callback returns.
 */
typedef void (*hashmap_free_cb)(void *);

/**
 * @brief A key-value pair stored in the hashmap.
 */
struct hashmap_pair {
    unsigned int key_len;   ///< Length of the key in bytes
    unsigned int value_len; ///< Length of the value in bytes
    void *key;              ///< Pointer to the key data
    void *value;            ///< Pointer to the value data
};

/**
 * @brief Internal node structure for the hashmap bucket chains.
 */
struct hashmap_node {
    hashmap_pair pair;  ///< The key-value pair
    hashmap_node *next; ///< Next node in the chain
    unsigned int hash;  ///< Cached hash for fast comparison and resize
};

/**
 * @brief Hashmap container structure.
 */
struct hashmap {
    hashmap_node **buckets;  ///< Array of bucket chain heads
    unsigned int capacity;   ///< Number of buckets
    unsigned int reserved;   ///< Number of stored key-value pairs
    hashmap_free_cb free_cb; ///< Optional callback to free values
};

/**
 * @brief Create a new hashmap.
 * @param hm Hashmap structure to initialize
 */
void hashmap_create(hashmap *hm);

/**
 * @brief Create a new hashmap with a value cleanup callback.
 * @details The callback is invoked when values are removed (during delete, clear, or free).
 *          It should release any resources owned by the value (e.g. nested pointers),
 *          but must NOT free the value pointer itself - the hashmap handles that.
 * @param hm Hashmap structure to initialize
 * @param free_cb Callback function to clean up removed values
 */
void hashmap_create_cb(hashmap *hm, hashmap_free_cb free_cb);

/**
 * @brief Free all memory used by the hashmap.
 * @details All contents will be freed and the hashmap will be reset.
 *          Any use of this hashmap after this will lead to undefined behavior.
 * @param hashmap Hashmap to free
 */
void hashmap_free(hashmap *hashmap);

/**
 * @brief Put an item into the hashmap.
 * @details The contents of both key and value memory blocks will be copied.
 *          However, any memory pointed to by them will NOT be copied.
 *          If the key already exists, the value is replaced.
 * @param hm Hashmap to modify
 * @param key Pointer to key data
 * @param key_len Length of the key in bytes
 * @param val Pointer to value data
 * @param value_len Length of the value in bytes
 * @return Pointer to the stored value (inside the hashmap)
 */
void *hashmap_put(hashmap *hm, const void *key, unsigned int key_len, const void *val, unsigned int value_len);

/**
 * @brief Get an item from the hashmap.
 * @param hm Hashmap to query
 * @param key Pointer to key data
 * @param key_len Length of the key in bytes
 * @param value Output: pointer to the stored value (or NULL if not found)
 * @param value_len Output: length of the stored value (or 0 if not found). Can be NULL.
 * @return 0 on success, 1 if key was not found
 */
int hashmap_get(hashmap *hm, const void *key, unsigned int key_len, void **value, unsigned int *value_len);

/**
 * @brief Delete an item from the hashmap by key.
 * @details Do not use this function inside an iterator loop; use hashmap_delete() instead.
 * @param hm Hashmap to modify
 * @param key Pointer to key data
 * @param key_len Length of the key in bytes
 * @return 0 on success, 1 if key was not found
 */
int hashmap_del(hashmap *hm, const void *key, unsigned int key_len);

/**
 * @brief Initialize an iterator for the hashmap.
 * @details Use iter_next() to iterate over hashmap_pair pointers.
 * @param hashmap Hashmap to iterate over
 * @param iter Iterator structure to initialize
 */
void hashmap_iter_begin(const hashmap *hashmap, iterator *iter);

/**
 * @brief Delete an item from the hashmap during iteration.
 * @details This function is iterator-safe and should be used instead of hashmap_del()
 *          when deleting items while iterating.
 * @param hashmap Hashmap to modify
 * @param iter Iterator pointing to the item to delete
 * @return 0 on success, 1 on error
 */
int hashmap_delete(hashmap *hashmap, iterator *iter);

/**
 * @brief Remove all entries from the hashmap.
 * @details All contents will be freed. After this, the hashmap size will be 0.
 * @param hashmap Hashmap to clear
 */
void hashmap_clear(hashmap *hashmap);

/**
 * @brief Get the bucket capacity of the hashmap.
 * @param hm Hashmap to query
 * @return Number of buckets in the hashmap
 */
static inline unsigned int hashmap_size(const hashmap *hm) {
    return hm->capacity;
}

/**
 * @brief Get the number of stored entries in the hashmap.
 * @param hm Hashmap to query
 * @return Number of key-value pairs in the hashmap
 */
static inline unsigned int hashmap_reserved(const hashmap *hm) {
    return hm->reserved;
}

/**
 * @brief Put a string key with any value into the hashmap.
 * @param hm Hashmap to modify
 * @param key Null-terminated string key
 * @param value Pointer to value data
 * @param value_len Length of the value in bytes
 */
static inline void hashmap_put_str(hashmap *hm, const char *key, void *value, unsigned int value_len) {
    hashmap_put(hm, key, strlen(key) + 1, value, value_len);
}

/**
 * @brief Put an integer key with any value into the hashmap.
 * @param hm Hashmap to modify
 * @param key Integer key
 * @param value Pointer to value data
 * @param value_len Length of the value in bytes
 */
static inline void hashmap_put_int(hashmap *hm, unsigned int key, void *value, unsigned int value_len) {
    hashmap_put(hm, (char *)&key, sizeof(unsigned int), value, value_len);
}

/**
 * @brief Get a value by string key.
 * @param hm Hashmap to query
 * @param key Null-terminated string key
 * @param value Output: pointer to the stored value
 * @param value_len Output: length of the stored value. Can be NULL.
 * @return 0 on success, 1 if key was not found
 */
static inline int hashmap_get_str(hashmap *hm, const char *key, void **value, unsigned int *value_len) {
    return hashmap_get(hm, (void *)key, strlen(key) + 1, value, value_len);
}

/**
 * @brief Get a value by integer key.
 * @param hm Hashmap to query
 * @param key Integer key
 * @param value Output: pointer to the stored value
 * @param value_len Output: length of the stored value. Can be NULL.
 * @return 0 on success, 1 if key was not found
 */
static inline int hashmap_get_int(hashmap *hm, unsigned int key, void **value, unsigned int *value_len) {
    return hashmap_get(hm, (void *)&key, sizeof(unsigned int), value, value_len);
}

/**
 * @brief Delete an entry by string key.
 * @param hm Hashmap to modify
 * @param key Null-terminated string key
 */
static inline void hashmap_del_str(hashmap *hm, const char *key) {
    hashmap_del(hm, key, strlen(key) + 1);
}

/**
 * @brief Delete an entry by integer key.
 * @param hm Hashmap to modify
 * @param key Integer key
 */
static inline void hashmap_del_int(hashmap *hm, unsigned int key) {
    hashmap_del(hm, (char *)&key, sizeof(unsigned int));
}

#endif // HASHMAP_H
