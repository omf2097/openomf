#include "utils/hashmap.h"
#include "utils/allocator.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define FNV_32_PRIME ((uint32_t)0x01000193)
#define FNV1_32_INIT ((uint32_t)2166136261)
#define ENLARGE_LIMIT 1024
#define INITIAL_SIZE 4

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

void hashmap_create(hashmap *hm) {
    hm->buckets = omf_calloc(INITIAL_SIZE, sizeof(hashmap_node *));
    hm->reserved = 0;
    hm->capacity = INITIAL_SIZE;
    hm->free_cb = NULL;
}

void hashmap_create_cb(hashmap *hm, hashmap_free_cb free_cb) {
    hashmap_create(hm);
    hm->free_cb = free_cb;
}

/**
 * Resizes the hashmap to a new capacity.
 *
 * All existing key-value pairs are rehashed, so this has some CPU impact.
 */
static void hashmap_resize(hashmap *hm, unsigned int new_size) {
    if(new_size <= hm->capacity) {
        return;
    }

    // Allocate and zero out a new memory blocks for the resized bucket list
    size_t new_bytes = new_size * sizeof(hashmap_node *);
    hm->buckets = omf_realloc(hm->buckets, new_bytes);
    memset(&hm->buckets[hm->capacity], 0, (new_size - hm->capacity) * sizeof(hashmap_node *));

    // Rehash
    hashmap_node *node = NULL;
    hashmap_node *this = NULL;
    unsigned int index;
    for(unsigned int i = 0; i < hm->capacity; i++) {
        node = hm->buckets[i];
        hm->buckets[i] = NULL;
        while(node != NULL) {
            this = node;
            node = node->next;

            // Recalculate index, and prepend the new index to the bucket list
            index = fnv_32a_buf(this->pair.key, this->pair.key_len, new_size);
            this->next = hm->buckets[index];
            hm->buckets[index] = this;
        }
    }
    hm->capacity = new_size;
}

/**
 * Check if hashmap pressure is high enough for automatic resize, and resize if yes.
 */
static void hashmap_enlarge_check(hashmap *hm) {
    if(hm->capacity >= ENLARGE_LIMIT) {
        return;
    }
    unsigned int q = hm->capacity - (hm->capacity >> 2);
    if(hm->reserved > q) {
        hashmap_resize(hm, hm->capacity << 1);
    }
}

void hashmap_clear(hashmap *hm) {
    hashmap_node *node = NULL;
    hashmap_node *tmp = NULL;
    for(unsigned int i = 0; i < hashmap_size(hm); i++) {
        node = hm->buckets[i];
        while(node != NULL) {
            tmp = node;
            node = node->next;
            if(hm->free_cb != NULL) {
                hm->free_cb(tmp->pair.value);
            }
            omf_free(tmp->pair.key);
            omf_free(tmp->pair.value);
            omf_free(tmp);
        }
        hm->buckets[i] = NULL;
    }
    hm->reserved = 0;
}

void hashmap_free(hashmap *hm) {
    hashmap_clear(hm);
    omf_free(hm->buckets);
    hm->capacity = 0;
    hm->reserved = 0;
}

void *hashmap_put(hashmap *hm, const void *key, unsigned int key_len, const void *val, unsigned int value_len) {
    unsigned int index = fnv_32a_buf(key, key_len, hm->capacity);
    hashmap_node *root = hm->buckets[index];
    hashmap_node *seek = root;

    // See if the key already exists in the buckets list
    int found = 0;
    while(seek) {
        if(seek->pair.key_len == key_len && memcmp(seek->pair.key, key, key_len) == 0) {
            found = 1;
            break;
        }
        seek = seek->next;
    }

    if(found) {
        // The key is already in the hashmap, so just realloc and reset the contents.
        seek->pair.value = omf_realloc(seek->pair.value, value_len);
        memcpy(seek->pair.value, val, value_len);
        seek->pair.value_len = value_len;
        return seek->pair.value;
    } else {
        // Key is not yet in the hashmap, so create a new node and set it
        // as the first entry in the buckets list.
        hashmap_node *node = omf_calloc(1, sizeof(hashmap_node));
        node->pair.key_len = key_len;
        node->pair.value_len = value_len;
        node->pair.key = omf_calloc(1, key_len);
        node->pair.value = omf_calloc(1, value_len);
        memcpy(node->pair.key, key, key_len);
        memcpy(node->pair.value, val, value_len);

        node->next = root;
        hm->buckets[index] = node;
        hm->reserved++;

        hashmap_enlarge_check(hm);
        return node->pair.value;
    }
}

int hashmap_del(hashmap *hm, const void *key, unsigned int key_len) {
    unsigned int index = fnv_32a_buf(key, key_len, hm->capacity);

    // Get node
    hashmap_node *node = hm->buckets[index];
    hashmap_node *prev = NULL;
    if(node == NULL) {
        return 1;
    }

    // Find the node we want to delete
    int found = 0;
    while(node) {
        if(node->pair.key_len == key_len && memcmp(node->pair.key, key, key_len) == 0) {
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
        if(hm->free_cb != NULL) {
            hm->free_cb(node->pair.value);
        }
        omf_free(node->pair.key);
        omf_free(node->pair.value);
        omf_free(node);
        hm->reserved--;
        return 0;
    }
    return 1;
}

int hashmap_get(hashmap *hm, const void *key, unsigned int key_len, void **value, unsigned int *value_len) {
    unsigned int index = fnv_32a_buf(key, key_len, hm->capacity);

    // Set defaults for error cases
    *value = NULL;
    if(value_len != NULL) {
        *value_len = 0;
    }

    // Get node
    hashmap_node *node = hm->buckets[index];
    if(node == NULL) {
        return 1;
    }

    // Find the node we want
    while(node) {
        if(node->pair.key_len == key_len && memcmp(node->pair.key, key, key_len) == 0) {
            *value = node->pair.value;
            if(value_len != NULL) {
                *value_len = node->pair.value_len;
            }
            return 0;
        }
        node = node->next;
    }

    return 1;
}

int hashmap_delete(hashmap *hm, iterator *iter) {
    int index = iter->inow - 1;

    if(iter->ended) {
        return 1;
    }

    // Find correct node
    hashmap_node *node = hm->buckets[index];
    hashmap_node *prev = NULL;
    hashmap_node *seek = iter->vnow;
    if(node == NULL || seek == NULL) {
        return 1;
    }

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

        // Got one, free up memory.
        if(hm->free_cb != NULL) {
            hm->free_cb(node->pair.value);
        }
        omf_free(node->pair.key);
        omf_free(node->pair.value);
        omf_free(node);
        hm->reserved--;
        return 0;
    }
    return 1;
}

static void _hashmap_seek_next(hashmap *hm, iterator *iter) {
    if(iter->inow >= (int)hashmap_size(hm)) {
        iter->vnow = NULL;
        iter->ended = 1;
        return;
    }
    do {
        iter->vnow = hm->buckets[iter->inow++];
    } while(iter->vnow == NULL && iter->inow < (int)hashmap_size(hm));
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
    iter->peek = NULL;
    iter->prev = NULL;
    iter->ended = (hm->reserved == 0);
}
