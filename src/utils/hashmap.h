#ifndef HASHMAP_H
#define HASHMAP_H

#include "utils/iterator.h"

typedef struct hashmap_pair_t hashmap_pair;
typedef struct hashmap_node_t hashmap_node;
typedef struct hashmap_t hashmap;

struct hashmap_pair_t {
    unsigned int keylen, vallen;
    void *key, *val;
};

struct hashmap_node_t {
    hashmap_pair pair;
    hashmap_node *next;
};

struct hashmap_t {
    hashmap_node **buckets;
    unsigned int capacity;
    unsigned int reserved;
};

void hashmap_create(hashmap *hm);
void hashmap_free(hashmap *hashmap);
unsigned int hashmap_size(const hashmap *hashmap);
unsigned int hashmap_reserved(const hashmap *hashmap);
void *hashmap_put(hashmap *hm, const void *key, unsigned int keylen, const void *val, unsigned int vallen);
void hashmap_sput(hashmap *hashmap, const char *key, void *value, unsigned int value_len);
void hashmap_iput(hashmap *hashmap, unsigned int key, void *value, unsigned int value_len);
int hashmap_get(hashmap *hm, const void *key, unsigned int keylen, void **val, unsigned int *vallen);
int hashmap_sget(hashmap *hashmap, const char *key, void **value, unsigned int *value_len);
int hashmap_iget(hashmap *hashmap, unsigned int key, void **value, unsigned int *value_len);
int hashmap_del(hashmap *hm, const void *key, unsigned int keylen);
void hashmap_sdel(hashmap *hashmap, const char *key);
void hashmap_idel(hashmap *hashmap, unsigned int key);
void hashmap_iter_begin(const hashmap *hashmap, iterator *iter);
int hashmap_delete(hashmap *hashmap, iterator *iter);
void hashmap_clear(hashmap *hashmap);

#endif // HASHMAP_H
