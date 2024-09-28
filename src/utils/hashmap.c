#include "utils/hashmap.h"
#include "utils/allocator.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define FNV_32_PRIME ((uint32_t)0x01000193)
#define FNV1_32_INIT ((uint32_t)2166136261)
#define SHRINK_LIMIT 4
#define ENLARGE_LIMIT 1024

uint32_t fnv_32a_buf(const void *buf, unsigned int len, unsigned int max_size) {
    unsigned char *bp = (unsigned char *)buf;
    unsigned char *be = bp + len;
    uint32_t val = FNV1_32_INIT;
    while(bp < be) {
        val ^= (uint32_t)*bp++;
        val *= FNV_32_PRIME;
    }
    return val % max_size;
}

/** \brief Creates a new hashmap
 *
 * \param hm Allocated hashmap pointer
 * \param initial_capacity Size of the hashmap.
 */
void hashmap_create(hashmap *hm) {
    hm->capacity = SHRINK_LIMIT;
    hm->buckets = omf_calloc(hashmap_size(hm), sizeof(hashmap_node *));
    hm->reserved = 0;
}

static void hashmap_resize(hashmap *hm, unsigned int new_size) {
    // Do not resize if equal size was requested
    if(new_size == hm->capacity || new_size < 1)
        return;

    // Allocate and zero out a new memory blocks for the resized bucket list
    hashmap_node **new_buckets = omf_calloc(new_size, sizeof(hashmap_node *));

    // Rehash
    hashmap_node *node = NULL;
    hashmap_node *this = NULL;
    unsigned int index;
    for(unsigned int i = 0; i < hashmap_size(hm); i++) {
        node = hm->buckets[i];
        while(node != NULL) {
            this = node;
            node = node->next;

            // Recalculate index, and prepend the new index to the bucket list
            index = fnv_32a_buf(this->pair.key, this->pair.keylen, new_size);
            this->next = new_buckets[index];
            new_buckets[index] = this;
        }
    }

    // Free old bucket list and assign new list and size of the hashmap
    omf_free(hm->buckets);
    hm->buckets = new_buckets;
    hm->capacity = new_size;
}

static void hashmap_enlarge_check(hashmap *hm) {
    if(hm->capacity >= ENLARGE_LIMIT)
        return;
    unsigned int q = hm->capacity - (hm->capacity >> 2);
    if(hm->reserved > q) {
        hashmap_resize(hm, hm->capacity << 1);
    }
}

static void hashmap_shrink_check(hashmap *hm) {
    if(hm->capacity <= SHRINK_LIMIT)
        return;
    unsigned int q = hm->capacity >> 2;
    if(hm->reserved < q) {
        hashmap_resize(hm, hm->capacity >> 1);
    }
}

/** \brief Clears hashmap entries
 *
 * This clears the hashmap of all entries. All contents will be freed.
 * After this, the hashmap size will be 0.
 *
 * \param hm Hashmap to clear
 */
void hashmap_clear(hashmap *hm) {
    hashmap_node *node = NULL;
    hashmap_node *tmp = NULL;
    for(unsigned int i = 0; i < hashmap_size(hm); i++) {
        node = hm->buckets[i];
        while(node != NULL) {
            tmp = node;
            node = node->next;
            omf_free(tmp->pair.key);
            omf_free(tmp->pair.val);
            omf_free(tmp);
            hm->reserved--;
        }
        hm->buckets[i] = NULL;
    }
}

/** \brief Free hashmap
 *
 * Frees the hashmap. All contents will be freed and hashmap will be deallocated.
 * Any use of this hashmap after this will lead to undefined behaviour.
 *
 * \param hm Hashmap to free
 */
void hashmap_free(hashmap *hm) {
    hashmap_clear(hm);
    omf_free(hm->buckets);
    hm->capacity = 0;
    hm->reserved = 0;
}

/** \brief Gets hashmap size
 *
 * Returns the hashmap size. This is the amount of hashmap allocated buckets.
 *
 * \param hm Hashmap
 * \return Amount of hashmap buckets
 */
unsigned int hashmap_size(const hashmap *hm) {
    return hm->capacity;
}

/** \brief Gets hashmap reserved buckets
 *
 * Returns the amount of items in the hashmap. Note that the item count
 * can be larger than the bucket count, if the hashmap is full enough.
 * If there are a lot more items than buckets, you should really consider
 * growing your hashmap bucket count ...
 *
 * \param hm Hashmap
 * \return Amount of items in the hashmap
 */
unsigned int hashmap_reserved(const hashmap *hm) {
    return hm->reserved;
}

/** \brief Puts an item to the hashmap
 *
 * Puts a new item to the hashmap. Note that the
 * contents of the value memory block will be copied. However,
 * any memory _pointed to_ by it will NOT be copied. So be careful!
 *
 * If auto-resizing is on, this will check if the hashmap needs
 * to be increased in size. If yes, size will be doubled and a full
 * rehashing operation will be run. This will take time!
 *
 * This function does NOT automatically decrease size, even if HASHMAP_AUTO_DEC
 * flag is enabled.
 *
 * \param hm Hashmap
 * \param key Pointer to key memory block
 * \param keylen Length of the key memory block
 * \param val Pointer to value memory block
 * \param vallen Length of the value memory block
 * \return Returns a pointer to the newly reserved hashmap pair.
 */
void *hashmap_put(hashmap *hm, const void *key, unsigned int keylen, const void *val, unsigned int vallen) {
    unsigned int index = fnv_32a_buf(key, keylen, hm->capacity);
    hashmap_node *root = hm->buckets[index];
    hashmap_node *seek = root;

    // See if the key already exists in the buckets list
    int found = 0;
    while(seek) {
        if(seek->pair.keylen == keylen && memcmp(seek->pair.key, key, keylen) == 0) {
            found = 1;
            break;
        }
        seek = seek->next;
    }

    if(found) {
        // The key is already in the hashmap, so just realloc and reset the contents.
        seek->pair.val = omf_realloc(seek->pair.val, vallen);
        memcpy(seek->pair.val, val, vallen);
        seek->pair.vallen = vallen;

        hashmap_enlarge_check(hm);
        return seek->pair.val;
    } else {
        // Key is not yet in the hashmap, so create a new node and set it
        // as the first entry in the buckets list.
        hashmap_node *node = omf_calloc(1, sizeof(hashmap_node));
        node->pair.keylen = keylen;
        node->pair.vallen = vallen;
        node->pair.key = omf_calloc(1, keylen);
        node->pair.val = omf_calloc(1, vallen);
        memcpy(node->pair.key, key, keylen);
        memcpy(node->pair.val, val, vallen);

        node->next = root;
        hm->buckets[index] = node;
        hm->reserved++;

        hashmap_enlarge_check(hm);
        return node->pair.val;
    }
}

/** \brief Deletes an item from the hashmap
 *
 * Deletes an item from the hashmap. Note: Using this function inside an
 * iterator may lead to weird behavior. If you wish to delete inside an
 * iterator, please use hashmap_delete.
 *
 * If autoresizing is on, this will check if the hashmap needs
 * to be decreased in size. If yes, size will be cut in half and a full
 * rehashing operation will be run. This will take time!
 *
 * This function does NOT automatically increase size, even if HASHMAP_AUTO_INC
 * flag is enabled.
 *
 * \param hm Hashmap
 * \param key Pointer to key memory block
 * \param keylen Length of the key memory block
 * \return Returns 0 on success, 1 on error (not found).
 */
int hashmap_del(hashmap *hm, const void *key, unsigned int keylen) {
    unsigned int index = fnv_32a_buf(key, keylen, hm->capacity);

    // Get node
    hashmap_node *node = hm->buckets[index];
    hashmap_node *prev = NULL;
    if(node == NULL)
        return 1;

    // Find the node we want to delete
    int found = 0;
    while(node) {
        if(node->pair.keylen == keylen && memcmp(node->pair.key, key, keylen) == 0) {
            found = 1;
            break;
        }
        prev = node;
        node = node->next;
    }

    // If node was found, handle delete
    if(found) {
        if(prev != NULL) {
            // If node is not first in chain, set correct links
            prev->next = node->next;
        } else {
            // If node is first in chain, set possible next entry as first
            hm->buckets[index] = node->next;
        }
        omf_free(node->pair.key);
        omf_free(node->pair.val);
        omf_free(node);
        hm->reserved--;

        hashmap_shrink_check(hm);
        return 0;
    }
    return 1;
}

/** \brief Gets an item from the hashmap
 *
 * Returns an item from the hashmap.
 *
 * \param hm Hashmap
 * \param key Pointer to key memory block
 * \param keylen Length of the key memory block
 * \param val Pointer to value hashmap memory block
 * \param vallen Length of the hashmap value memory block
 * \return Returns 0 on success, 1 on error (not found).
 */
int hashmap_get(hashmap *hm, const void *key, unsigned int keylen, void **val, unsigned int *vallen) {
    unsigned int index = fnv_32a_buf(key, keylen, hm->capacity);

    // Set defaults for error cases
    *val = NULL;
    *vallen = 0;

    // Get node
    hashmap_node *node = hm->buckets[index];
    if(node == NULL)
        return 1;

    // Find the node we want
    while(node) {
        if(node->pair.keylen == keylen && memcmp(node->pair.key, key, keylen) == 0) {
            *val = node->pair.val;
            *vallen = node->pair.vallen;
            return 0;
        }
        node = node->next;
    }

    return 1;
}

void hashmap_sput(hashmap *hm, const char *key, void *value, unsigned int value_len) {
    hashmap_put(hm, key, strlen(key) + 1, value, value_len);
}

void hashmap_iput(hashmap *hm, unsigned int key, void *value, unsigned int value_len) {
    hashmap_put(hm, (char *)&key, sizeof(unsigned int), value, value_len);
}

int hashmap_sget(hashmap *hm, const char *key, void **value, unsigned int *value_len) {
    return hashmap_get(hm, (void *)key, strlen(key) + 1, value, value_len);
}

int hashmap_iget(hashmap *hm, unsigned int key, void **value, unsigned int *value_len) {
    return hashmap_get(hm, (void *)&key, sizeof(unsigned int), value, value_len);
}

void hashmap_sdel(hashmap *hm, const char *key) {
    hashmap_del(hm, key, strlen(key) + 1);
}

void hashmap_idel(hashmap *hm, unsigned int key) {
    hashmap_del(hm, (char *)&key, sizeof(unsigned int));
}

/** \brief Deletes an item from the hashmap by iterator key
 *
 * Deletes an item from the hashmap by a matching iterator key.
 * This function is iterator safe. In theory, this function
 * should not fail, as the iterable value should exist.
 *
 * Note! This function does NOT autoresize at all, even if the flags
 * are enabled. This is to avoid trouble while iterating. If you do
 * a large amount of deletions here and suspect the minimum limit has been
 * reached, call hashmap_autoresize to run a check and resize operation.
 *
 * \param hm Hashmap
 * \param iter Iterator
 * \return Returns 0 on success, 1 on error (not found).
 */
int hashmap_delete(hashmap *hm, iterator *iter) {
    int index = iter->inow - 1;

    if(iter->ended) {
        return 1;
    }

    // Find correct node
    hashmap_node *node = hm->buckets[index];
    hashmap_node *prev = NULL;
    hashmap_node *seek = iter->vnow;
    if(node == NULL || seek == NULL)
        return 1;

    // Find the node we want to delete
    int found = 0;
    while(node) {
        if(node == seek) {
            found = 1;
            break;
        }
        prev = node;
        node = node->next;
    }

    // If node was found, handle delete
    if(found) {
        // If node is not first in chain, set correct links
        // Otherwise set possible next entry as first
        if(prev != NULL) {
            prev->next = node->next;
            iter->vnow = prev;
        } else {
            hm->buckets[index] = node->next;
            iter->vnow = NULL;
            iter->inow--;
        }

        // Alld one, free up memory.
        omf_free(node->pair.key);
        omf_free(node->pair.val);
        omf_free(node);
        hm->reserved--;
        return 0;
    }
    return 1;
}

static void _hashmap_seek_next(hashmap *hm, iterator *iter) {
    if(iter->inow >= hashmap_size(hm)) {
        iter->vnow = NULL;
        iter->ended = 1;
        return;
    }
    do {
        iter->vnow = hm->buckets[iter->inow++];
    } while(iter->vnow == NULL && iter->inow < hashmap_size(hm));
}

void *hashmap_iter_next(iterator *iter) {
    hashmap_node *tmp = NULL;
    hashmap *hm = (hashmap *)iter->data;

    // Find next non-empty bucket
    if(iter->vnow == NULL) {
        _hashmap_seek_next(hm, iter);
        if(iter->vnow == NULL) {
            return NULL;
        }
        tmp = (hashmap_node *)iter->vnow;
        return &tmp->pair;
    }

    // We already are in a non-empty bucket. See if it has any
    // other non-empty buckets in list. If it does, return it.
    tmp = (hashmap_node *)iter->vnow;
    if(tmp->next != NULL) {
        iter->vnow = tmp->next;
        return &tmp->next->pair;
    }

    // We are in the end of the list for this bucket.
    // Find next non-empty bucket.
    _hashmap_seek_next(hm, iter);
    if(iter->vnow == NULL) {
        return NULL;
    }
    tmp = (hashmap_node *)iter->vnow;
    return &tmp->pair;
}

void hashmap_iter_begin(const hashmap *hm, iterator *iter) {
    iter->data = hm;
    iter->vnow = NULL;
    iter->inow = 0;
    iter->next = hashmap_iter_next;
    iter->prev = NULL;
    iter->ended = (hm->reserved == 0);
}
