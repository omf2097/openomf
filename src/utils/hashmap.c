#include "utils/hashmap.h"
#include "utils/allocator.h"
#include <stdint.h>
#include <string.h>

#define FNV_32_PRIME ((uint32_t)0x01000193)
#define FNV1_32_INIT ((uint32_t)2166136261)
#define ENLARGE_LIMIT 1024
#define INITIAL_SIZE 4
#define HASH_INDEX(hash, capacity) ((hash) & ((capacity) - 1))

static uint32_t fnv_32a_hash(const void *buf, unsigned int len) {
    unsigned char *bp = (unsigned char *)buf;
    unsigned char *be = bp + len;
    uint32_t val = FNV1_32_INIT;
    while(bp < be) {
        val ^= (uint32_t)*bp++;
        val *= FNV_32_PRIME;
    }
    return val;
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
 * Resize the hashmap by doubling its capacity.
 *
 * This is mostly copied from Java hashmap. We use an optimized rehash algorithm
 * that exploits power-of-2 sizing.
 *
 * When the capacity grows, we always double, which gives us the old area (area 0) and the new area (area 1), both
 * the same size. Because the index is computed as `index = hash & (capacity - 1)`, we can use the capacity value
 * as a bit mask that signifies which area the node should be put when resizing.
 *
 * For example:
 * - Capacity = 4 (0b100), Hash mask = 3 (0x11)
 * - Capacity = 8 (0b1000), Hash mask = 7 (0b111)
 * - Capacity = 16 (0b10000), Hash mask = 15 (0b1111)
 *
 * So let's say the old capacity is 4, and our old hashes are all covered by the mask 0x11. Now the capacity grows
 * to double, so the mask is 0b111 and the the hashes bit 0b100 becomes significant. With that mask (the old capacity
 * value), we can check if the value belongs to the old area or the new area by doing as simple bitwise AND.
 * So, if (hash & 0x100 == 0), it goes to area 0. If (hash & 0x100 == 1), it goes to area 1.
 *
 * tl:dr; We don't need to rehash everything. Gotta go fast.
 */
static void hashmap_resize(hashmap *hm, unsigned int new_size) {
    if(new_size <= hm->capacity) {
        return;
    }

    unsigned int old_capacity = hm->capacity;
    const size_t new_bytes = new_size * sizeof(hashmap_node *);
    hm->buckets = omf_realloc(hm->buckets, new_bytes);
    memset(&hm->buckets[old_capacity], 0, (new_size - old_capacity) * sizeof(hashmap_node *));

    // For each old bucket, we split the chains into two new chains (one that moves, one that stays).
    hashmap_node *node = NULL;
    hashmap_node *next = NULL;
    hashmap_node **stay_tail = NULL;
    hashmap_node **move_tail = NULL;

    // Walk the chain roots up to the old capacity.
    for(unsigned int i = 0; i < old_capacity; i++) {
        hashmap_node *stay_list = NULL; // nodes that remain at index i
        hashmap_node *move_list = NULL; // nodes that move to index (i + old_capacity)
        stay_tail = &stay_list;         // Track the last entry of the staying list
        move_tail = &move_list;         // Track the last entry of the moving list

        // Walk down the node chain from the root, and split the nodes into the two lists.
        node = hm->buckets[i];
        while(node != NULL) {
            next = node->next;
            if(node->hash & old_capacity) {
                // Move to new bucket
                *move_tail = node;
                move_tail = &node->next;
            } else {
                // Stay in current bucket
                *stay_tail = node;
                stay_tail = &node->next;
            }
            node = next;
        }

        // last entries should be NULL
        *stay_tail = NULL;
        *move_tail = NULL;

        // Set the moving and staying lists (staying list replaces the old list).
        hm->buckets[i] = stay_list;
        hm->buckets[i + old_capacity] = move_list;
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
    const uint32_t hash = fnv_32a_hash(key, key_len);
    const unsigned int index = HASH_INDEX(hash, hm->capacity);
    hashmap_node *root = hm->buckets[index];
    hashmap_node *seek = root;

    // See if the key already exists in the buckets list
    while(seek) {
        if(seek->hash == hash && seek->pair.key_len == key_len && memcmp(seek->pair.key, key, key_len) == 0) {
            // The key is already in the hashmap, so just realloc and reset the contents.
            seek->pair.value = omf_realloc(seek->pair.value, value_len);
            memcpy(seek->pair.value, val, value_len);
            seek->pair.value_len = value_len;
            return seek->pair.value;
        }
        seek = seek->next;
    }

    // Key is not yet in the hashmap, so create a new node and set it
    // as the first entry in the buckets list.
    hashmap_node *node = omf_calloc(1, sizeof(hashmap_node) + key_len);
    node->pair.key_len = key_len;
    node->pair.value_len = value_len;
    node->pair.key = (char *)node + sizeof(hashmap_node);
    node->pair.value = omf_calloc(1, value_len);
    node->hash = hash;
    memcpy(node->pair.key, key, key_len);
    memcpy(node->pair.value, val, value_len);

    node->next = root;
    hm->buckets[index] = node;
    hm->reserved++;

    hashmap_enlarge_check(hm);
    return node->pair.value;
}

int hashmap_del(hashmap *hm, const void *key, unsigned int key_len) {
    const uint32_t hash = fnv_32a_hash(key, key_len);
    const unsigned int index = HASH_INDEX(hash, hm->capacity);

    // Get node
    hashmap_node *node = hm->buckets[index];
    hashmap_node *prev = NULL;
    if(node == NULL) {
        return 1;
    }

    // Find the node we want to delete
    while(node) {
        if(node->hash == hash && node->pair.key_len == key_len && memcmp(node->pair.key, key, key_len) == 0) {
            if(prev != NULL) {
                prev->next = node->next; // Node is not first in chain
            } else {
                hm->buckets[index] = node->next; // Node is first in chain
            }
            if(hm->free_cb != NULL) {
                hm->free_cb(node->pair.value);
            }
            omf_free(node->pair.value);
            omf_free(node);
            hm->reserved--;
            return 0;
        }
        prev = node;
        node = node->next;
    }
    return 1;
}

int hashmap_get(hashmap *hm, const void *key, unsigned int key_len, void **value, unsigned int *value_len) {
    const uint32_t hash = fnv_32a_hash(key, key_len);
    const unsigned int index = HASH_INDEX(hash, hm->capacity);

    // Set defaults for error cases
    *value = NULL;
    if(value_len != NULL) {
        *value_len = 0;
    }

    // Find the node we want
    hashmap_node *node = hm->buckets[index];
    while(node) {
        if(node->hash == hash && node->pair.key_len == key_len && memcmp(node->pair.key, key, key_len) == 0) {
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
        omf_free(node->pair.value);
        omf_free(node);
        hm->reserved--;
        return 0;
    }
    return 1;
}

static void _hashmap_seek_next(const hashmap *hm, iterator *iter) {
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
    const hashmap *hm = (const hashmap *)iter->data;

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
    // other nodes in the chain. If it does, return it.
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
