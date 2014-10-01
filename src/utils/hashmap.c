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

/** \brief Creates a new hashmap with an allocator
  *
  * Creates a new hashmap. This is just like hashmap_create, but
  * allows the user to define the memory allocation functions.
  *
  * \param hm Allocated memory pointer
  * \param n_size Size of the hashmap. Final size will be pow(w, n_size)
  * \param alloc Allocation functions
  */
void hashmap_create_with_allocator(hashmap *hm, int n_size, allocator alloc) {
    hm->alloc = alloc;
    hm->buckets_x = n_size;
    size_t b_size = hashmap_size(hm) * sizeof(hashmap_node*);
    hm->buckets = hm->alloc.cmalloc(b_size);
    memset(hm->buckets, 0, b_size);
    hm->reserved = 0;
}


/** \brief Creates a new hashmap
  *
  * Creates a new hashmap. Note that the size parameter doesn't mean bucket count,
  * but the bucket count is actually calculated pow(2, n_size). So for example value
  * 8 means 256 buckets, and 9 would be 512 buckets.
  *
  * \todo Make a better create function
  *
  * \param hm Allocated memory pointer
  * \param n_size Size of the hashmap. Final size will be pow(w, n_size)
  */
void hashmap_create(hashmap *hm, int n_size) {
    allocator alloc;
    alloc.cmalloc = malloc;
    alloc.crealloc = realloc;
    alloc.cfree = free;
    hashmap_create_with_allocator(hm, n_size, alloc);
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
            hm->alloc.cfree(tmp->pair.key);
            hm->alloc.cfree(tmp->pair.val);
            hm->alloc.cfree(tmp);
            hm->reserved--;
        }
        hm->buckets[i] = NULL;
    }
}

/** \brief Free hashmap
  *
  * Frees the hasmap. All contents will be freed and hashmap will be deallocated.
  * Any use of this hashmap after this will lead to undefined behaviour.
  *
  * \param hm Hashmap to free
  */
void hashmap_free(hashmap *hm) {
    hashmap_clear(hm);
    hm->alloc.cfree(hm->buckets);
    hm->buckets = NULL;
    hm->buckets_x = 0;
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
    return BUCKETS_SIZE(hm->buckets_x);
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
  * \param hm Hashmap
  * \param key Pointer to key memory block
  * \param keylen Length of the key memory block
  * \param val Pointer to value memory block
  * \param vallen Length of the value memory block
  * \return Returns a pointer to the newly reserved hashmap pair.
  */
void* hashmap_put(hashmap *hm,
                  const void *key, unsigned int keylen,
                  const void *val, unsigned int vallen) {
    unsigned int index = fnv_32a_buf(key, keylen, hm->buckets_x);

    // Create new node, copy data
    hashmap_node *node = hm->alloc.cmalloc(sizeof(hashmap_node));
    node->pair.keylen = keylen;
    node->pair.vallen = vallen;
    node->pair.key = hm->alloc.cmalloc(keylen);
    node->pair.val = hm->alloc.cmalloc(vallen);
    memcpy(node->pair.key, key, keylen);
    memcpy(node->pair.val, val, vallen);

    // Set new node as first
    node->next = hm->buckets[index];
    hm->buckets[index] = node;
    hm->reserved++;

    // Return a pointer to the newly allocated value
    // for convenience
    return node->pair.val;
}


/** \brief Deletes an item from the hashmap
  *
  * Deletes an item from the hashmap. Note: Using this function inside an
  * iterator may lead to weird behavior. If you wish to delete inside an
  * iterator, please use hashmap_delete.
  *
  * \param hm Hashmap
  * \param key Pointer to key memory block
  * \param keylen Length of the key memory block
  * \return Returns 0 on success, 1 on error (not found).
  */
int hashmap_del(hashmap *hm, const void *key, unsigned int keylen) {
    unsigned int index = fnv_32a_buf(key, keylen, hm->buckets_x);

    // Get node
    hashmap_node *node = hm->buckets[index];
    hashmap_node *prev = NULL;
    if(node == NULL) return 1;

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
            hm->buckets[index] = node->next;
        }
        hm->alloc.cfree(node->pair.key);
        hm->alloc.cfree(node->pair.val);
        hm->alloc.cfree(node);
        hm->reserved--;
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
    unsigned int index = fnv_32a_buf(key, keylen, hm->buckets_x);

    // Set defaults for error cases
    *val = NULL;
    *vallen = 0;

    // Get node
    hashmap_node *node = hm->buckets[index];
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

/** \brief Deletes an item from the hashmap by iterator key
  *
  * Deletes an item from the hashmap by a matching iterator key.
  * This function is iterator safe. In theory, this function
  * should not fail, as the iterable value should exist.
  *
  * \param hm Hashmap
  * \param iter Iterator
  * \return Returns 0 on success, 1 on error (not found).
  */
int hashmap_delete(hashmap *hm, iterator *iter) {
    int index = iter->inow - 1;

    // Find correct node
    hashmap_node *node = hm->buckets[index];
    hashmap_node *prev = NULL;
    hashmap_node *seek = iter->vnow;
    if(node == NULL || seek == NULL) return 1;

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
        hm->alloc.cfree(node->pair.key);
        hm->alloc.cfree(node->pair.val);
        hm->alloc.cfree(node);
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

void* hashmap_iter_next(iterator *iter) {
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

void hashmap_iter_begin(const hashmap *hm, iterator *iter) {
    iter->data = hm;
    iter->vnow = NULL;
    iter->inow = 0;
    iter->next = hashmap_iter_next;
    iter->prev = NULL;
    iter->ended = (hm->reserved == 0);
}
