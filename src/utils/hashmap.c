#include "utils/hashmap.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define FNV_32_PRIME ((uint32_t)0x01000193)
#define FNV1_32_INIT ((uint32_t)2166136261)
#define TINY_MASK(x) (((uint32_t)1<<(x))-1)
#define BUCKETS_SIZE(x) (pow(2,(x)))

// The rest

uint32_t fnv_32a_buf(const void *buf, unsigned int len, unsigned int x) {
    unsigned char *bp = (unsigned char *)buf;
    unsigned char *be = bp + len;
    uint32_t hval = FNV1_32_INIT;
    while(bp < be) {
        hval ^= (uint32_t)*bp++;
        hval *= FNV_32_PRIME;
    }
    return (((hval >> x) ^ hval) & TINY_MASK(x));
}

void hashmap_create_with_allocator(hashmap *hm, int n_size, allocator alloc) {
    hm->alloc = alloc;
    hm->buckets_x = n_size;
    hm->buckets = hm->alloc.cmalloc(BUCKETS_SIZE(hm->buckets_x) * sizeof(hashmap_bucket));
    for(int i = 0; i < BUCKETS_SIZE(hm->buckets_x); i++) {
        hm->buckets[i].first = NULL;
    }
    hm->reserved = 0;
}

void hashmap_create(hashmap *hm, int n_size) {
    allocator alloc;
    alloc.cmalloc = malloc;
    alloc.crealloc = realloc;
    alloc.cfree = free;
    hashmap_create_with_allocator(hm, n_size, alloc);
}

void hashmap_clear(hashmap *hm) {
    hashmap_node *node = NULL;
    hashmap_node *tmp = NULL;
    for(unsigned int i = 0; i < BUCKETS_SIZE(hm->buckets_x); i++) {
        node = hm->buckets[i].first;
        while(node != NULL) {
            tmp = node;
            node = node->next;
            hm->alloc.cfree(tmp->pair.key);
            hm->alloc.cfree(tmp->pair.val);
            hm->alloc.cfree(tmp);
            hm->reserved--;
        }
        hm->buckets[i].first = NULL;
    }
}

void hashmap_free(hashmap *hm) {
    hashmap_clear(hm);
    hm->alloc.cfree(hm->buckets);
    hm->buckets = NULL;
    hm->buckets_x = 0;
    hm->reserved = 0;
}

unsigned int hashmap_size(hashmap *hm) {
    return BUCKETS_SIZE(hm->buckets_x);
}

unsigned int hashmap_reserved(hashmap *hm) {
    return hm->reserved;
}

void* hashmap_put(hashmap *hm, const void *key, unsigned int keylen, void *val, unsigned int vallen) {
    unsigned int index = fnv_32a_buf(key, keylen, hm->buckets_x);

    // Create new node, copy data
    hashmap_node *node = (hashmap_node*)hm->alloc.cmalloc(sizeof(hashmap_node));
    node->pair.keylen = keylen;
    node->pair.vallen = vallen;
    node->pair.key = hm->alloc.cmalloc(keylen);
    node->pair.val = hm->alloc.cmalloc(vallen);
    memcpy(node->pair.key, key, keylen);
    memcpy(node->pair.val, val, vallen);
    
    // Set new node as first
    node->next = hm->buckets[index].first;
    hm->buckets[index].first = node;
    hm->reserved++;

    // Return a pointer to the newly allocated value
    // for convenience
    return node->pair.val;
}

void hashmap_del(hashmap *hm, const void *key, unsigned int keylen) {
    unsigned int index = fnv_32a_buf(key, keylen, hm->buckets_x);
    
    // Get node
    hashmap_node *node = hm->buckets[index].first;
    hashmap_node *prev = NULL;
    if(node == NULL) return;
    
    // Find the node we want to delete
    int found = 0;
    while(node) {
        if(node->pair.keylen == keylen) {
            if(memcmp(node->pair.key, key, keylen) == 0) {
                found = 1;
                break;
            }
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
            hm->buckets[index].first = node->next;
        }
        hm->alloc.cfree(node->pair.key);
        hm->alloc.cfree(node->pair.val);
        hm->alloc.cfree(node);
        hm->reserved--;
    }
}

int hashmap_get(hashmap *hm, const void *key, unsigned int keylen, void **val, unsigned int *vallen) {
    unsigned int index = fnv_32a_buf(key, keylen, hm->buckets_x);
    
    // Set defaults for error cases
    *val = NULL;
    *vallen = 0;
    
    // Get node
    hashmap_node *node = hm->buckets[index].first;
    if(node == NULL) return 1;
    
    // Find the node we want
    while(node) {
        if(node->pair.keylen == keylen) {
            if(memcmp(node->pair.key, key, keylen) == 0) {
                *val = node->pair.val;
                *vallen = node->pair.vallen;
                return 0;
            }
        }
        node = node->next;
    }
    
    return 1;
}

void hashmap_sput(hashmap *hm, const char *key, void *value, unsigned int value_len) {
    hashmap_put(hm, key, strlen(key)+1, value, value_len);
}

void hashmap_iput(hashmap *hm, unsigned int key, void *value, unsigned int value_len) {
    hashmap_put(hm, (char*)&key, sizeof(unsigned int), value, value_len);
}

int hashmap_sget(hashmap *hm, const char *key, void **value, unsigned int *value_len) {
    return hashmap_get(hm, (void*)key, strlen(key)+1, value, value_len);
}

int hashmap_iget(hashmap *hm, unsigned int key, void **value, unsigned int *value_len) {
    return hashmap_get(hm, (void*)&key, sizeof(unsigned int), value, value_len);
}

void hashmap_sdel(hashmap *hm, const char *key) {
    hashmap_del(hm, key, strlen(key)+1);
}

void hashmap_idel(hashmap *hm, unsigned int key) {
    hashmap_del(hm, (char*)&key, sizeof(unsigned int));
}

void hashmap_delete(hashmap *hm, iterator *iter) {
    int index = iter->inow - 1;

    // Find correct node
    hashmap_node *node = hm->buckets[index].first;
    hashmap_node *prev = NULL;
    hashmap_node *seek = iter->vnow;
    if(node == NULL || seek == NULL) return;

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
        } else {
            hm->buckets[index].first = node->next;
        }

        // Make sure we have a good iterator
        iter->vnow = prev;

        // Alld one, free up memory.
        hm->alloc.cfree(node->pair.key);
        hm->alloc.cfree(node->pair.val);
        hm->alloc.cfree(node);
        hm->reserved--;
    }
}

static void _hashmap_seek_next(hashmap *hm, iterator *iter) {
    do {
        iter->vnow = hm->buckets[iter->inow++].first;
    } while(iter->vnow == NULL && iter->inow < BUCKETS_SIZE(hm->buckets_x));
    if(iter->inow >= BUCKETS_SIZE(hm->buckets_x)) {
        iter->ended = 1;
    }
}

static void* hashmap_iter_next(iterator *iter) {
    hashmap_node *tmp = NULL;
    hashmap *hm = (hashmap*)iter->data;

    // Find next non-empty bucket
    if(iter->vnow == NULL) {
        _hashmap_seek_next(hm, iter);
        if(iter->vnow == NULL) {
            return NULL;
        }
        tmp = (hashmap_node*)iter->vnow;
        return &tmp->pair;
    }

    // We already are in a non-empty bucket. See if it has any
    // other non-empty buckets in list. If it does, return it.
    tmp = (hashmap_node*)iter->vnow;
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
    tmp = (hashmap_node*)iter->vnow;
    return &tmp->pair;
}

void hashmap_iter_begin(hashmap *hm, iterator *iter) {
    iter->data = hm;
    iter->vnow = NULL;
    iter->inow = 0;
    iter->next = hashmap_iter_next;
    iter->prev = NULL;
    iter->ended = (hm->reserved == 0);
}
