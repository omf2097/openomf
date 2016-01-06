#include "utils/hashmap.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define FNV_32_PRIME ((uint32_t)0x01000193)
#define FNV1_32_INIT ((uint32_t)2166136261)
#define TINY_MASK(x) (((uint32_t)1<<(x))-1)
#define BUCKETS_SIZE(x) (pow(2,(x)))

#define AUTO_INC_CHECK() \
    if(hm->flags & HASHMAP_AUTO_INC \
        && hm->buckets_x < hm->buckets_x_max \
        && hashmap_get_pressure(hm) > hm->max_pressure) \
    { \
        hashmap_resize(hm, hm->buckets_x+1); \
    }

#define AUTO_DEC_CHECK() \
    if(hm->flags & HASHMAP_AUTO_DEC \
        && hm->buckets_x > hm->buckets_x_min \
        && hashmap_get_pressure(hm) > hm->max_pressure) \
    { \
        hashmap_resize(hm, hm->buckets_x-1); \
    }

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
  * \param hm Allocated hashmap pointer
  * \param n_size Size of the hashmap. Final size will be pow(2, n_size)
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
  * \param hm Allocated hashmap pointer
  * \param n_size Size of the hashmap. Final size will be pow(2, n_size)
  */
void hashmap_create(hashmap *hm, int n_size) {
    allocator alloc;
    alloc.cmalloc = malloc;
    alloc.crealloc = realloc;
    alloc.cfree = free;
    hashmap_create_with_allocator(hm, n_size, alloc);
}

/** \brief Set hashmap options
  *
  * Used to set hashmap resizing options and pressures.
  *
  * Available flags:
  * - HASHMAP_AUTO_INC: Enables automatic hashmap size increasing (set pressure via max_pressure)
  * - HASHMAP_AUTO_DEC: Enables automatic hashmap size decreasing (set pressure via min_pressure)
  *
  * Decent default for pressures might be 0.25 for minimum and 0.75 for maximum. These can and should
  * of course be tweaked as necessary, since resizing is a rather expensive operation and should be
  * avoided if necessary.
  * 
  * buckets_min and buckets_max define the minimum and maximum amount of buckets available in the
  * hashmap.
  *
  * min_pressure and max_pressure will only be used if HASHMAP_AUTO_INC and HASHMAP_AUTO_DEC are set,
  * respectively. Same with buckets_min and buckets_max.
  *
  * \param hm Allocated hashmap pointer
  * \param flags Feature flags (eg. HASHMAP_AUTO_INC|HASHMAP_AUTO_DEC)
  * \param min_pressure Minimum pressure value for auto-resize (eg. 0.25)
  * \param max_pressure Maximum pressure value for auto-resize (eg. 0.75)
  * \param buckets_min Minimum amount of buckets (must be >= 2)
  * \param buckets_max Maximum amount of buckets
  */
void hashmap_set_opts(hashmap *hm, 
    unsigned int flags,
    float min_pressure, float max_pressure,
    int buckets_min, int buckets_max)
{
    hm->flags = flags & 0x3;
    hm->min_pressure = min_pressure;
    hm->max_pressure = max_pressure;
    hm->buckets_x_min = buckets_min >= 2 ? buckets_min : 2;
    hm->buckets_x_max = buckets_max;
}

/** \brief Get current hashmap pressure
  *
  * Simply gets the current fill pressure for the hashmap.
  * Calculated as reserved_buckets / total_buckets.
  *
  * \param hm Allocated hashmap pointer
  */
float hashmap_get_pressure(hashmap *hm) {
    return ((float)hm->reserved) / ((float)BUCKETS_SIZE(hm->buckets_x));
}

/** \brief Runs pressure checks and resizes if necessary
  *
  * This function checks if either the minimun or maximum pressure checks are on
  * and if the pressure values have been exceeded. If they are, resize operation
  * will be run.
  *
  * \param hm Allocated hashmap pointer
  */
void hashmap_autoresize(hashmap *hm) {
    AUTO_INC_CHECK()
    AUTO_DEC_CHECK()
}

/** \brief Resizes the hashmap
  *
  * Note! This is a very naive implementation. During the rehashing, a separate
  * memory area is reserved, so memory usage will temporarily jump!
  *
  * \todo Make a better resize function
  *
  * \param hm Allocated hashmap pointer
  * \param n_size Size of the hashmap. Final size will be pow(2, n_size)
  */
int hashmap_resize(hashmap *hm, int n_size) {
    // Do not resize if equal size was requested
    if(n_size == hm->buckets_x) {
        return 1;
    }

    // Allocate and zero out a new memory blocks for the resized bucket list
    size_t new_size = BUCKETS_SIZE(n_size) * sizeof(hashmap_node*);
    hashmap_node **new_buckets = hm->alloc.cmalloc(new_size);
    memset(new_buckets, 0, new_size);

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
            index = fnv_32a_buf(this->pair.key, this->pair.keylen, n_size);
            this->next = new_buckets[index];
            new_buckets[index] = this;
        }
    }

    // Free old bucket list and assign new list and size of the hashmap
    free(hm->buckets);
    hm->buckets = new_buckets;
    hm->buckets_x = n_size;
    return 0;
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
  * If autoresizing is on, this will first check if the hashmap needs
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
void* hashmap_put(hashmap *hm,
                  const void *key, unsigned int keylen,
                  const void *val, unsigned int vallen)
{
    AUTO_INC_CHECK()

    unsigned int index = fnv_32a_buf(key, keylen, hm->buckets_x);
    hashmap_node *root = hm->buckets[index];
    hashmap_node *seek = root;

    // See if the key already exists in the buckets list
    int found = 0;
    while(seek) {
        if(seek->pair.keylen == keylen) {
            if(memcmp(seek->pair.key, key, keylen) == 0) {
                found = 1;
                break;
            }
        }
        seek = seek->next;
    }

    if(found) {
        // The key is already in the hashmap, so just realloc and reset the contents.
        seek->pair.val = hm->alloc.crealloc(seek->pair.val, vallen);
        memcpy(seek->pair.val, val, vallen);
        seek->pair.vallen = vallen;
        return seek->pair.val;
    } else {
        // Key is not yet in the hashmap, so create a new node and set it
        // as the first entry in the buckets list.
        hashmap_node *node = hm->alloc.cmalloc(sizeof(hashmap_node));
        node->pair.keylen = keylen;
        node->pair.vallen = vallen;
        node->pair.key = hm->alloc.cmalloc(keylen);
        node->pair.val = hm->alloc.cmalloc(vallen);
        memcpy(node->pair.key, key, keylen);
        memcpy(node->pair.val, val, vallen);

        node->next = root;
        hm->buckets[index] = node;
        hm->reserved++;

        return node->pair.val;
    }
}


/** \brief Deletes an item from the hashmap
  *
  * Deletes an item from the hashmap. Note: Using this function inside an
  * iterator may lead to weird behavior. If you wish to delete inside an
  * iterator, please use hashmap_delete.
  *
  * If autoresizing is on, this will first check if the hashmap needs
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
    AUTO_DEC_CHECK()

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
