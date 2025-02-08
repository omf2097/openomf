#ifndef HASHMAP_H
#define HASHMAP_H

#include "utils/iterator.h"

typedef struct hashmap_pair hashmap_pair;
typedef struct hashmap_node hashmap_node;
typedef struct hashmap hashmap;
typedef void (*hashmap_free_cb)(void *);

struct hashmap_pair {
    unsigned int key_len;
    unsigned int value_len;
    void *key;
    void *value;
};

struct hashmap_node {
    hashmap_pair pair;
    hashmap_node *next;
};

struct hashmap {
    hashmap_node **buckets;
    unsigned int capacity;
    unsigned int reserved;
    hashmap_free_cb free_cb;
};

void hashmap_create(hashmap *hm);
void hashmap_create_cb(hashmap *hm, hashmap_free_cb free_cb);
void hashmap_free(hashmap *hashmap);
unsigned int hashmap_size(const hashmap *hashmap);
unsigned int hashmap_reserved(const hashmap *hashmap);
void *hashmap_put(hashmap *hm, const void *key, unsigned int key_len, const void *val, unsigned int value_len);
void hashmap_sput(hashmap *hashmap, const char *key, void *value, unsigned int value_len);
void hashmap_iput(hashmap *hashmap, unsigned int key, void *value, unsigned int value_len);
int hashmap_get(hashmap *hm, const void *key, unsigned int key_len, void **value, unsigned int *value_len);
int hashmap_sget(hashmap *hashmap, const char *key, void **value, unsigned int *value_len);
int hashmap_iget(hashmap *hashmap, unsigned int key, void **value, unsigned int *value_len);
int hashmap_del(hashmap *hm, const void *key, unsigned int key_len);
void hashmap_sdel(hashmap *hashmap, const char *key);
void hashmap_idel(hashmap *hashmap, unsigned int key);
void hashmap_iter_begin(const hashmap *hashmap, iterator *iter);
int hashmap_delete(hashmap *hashmap, iterator *iter);
void hashmap_clear(hashmap *hashmap);

#endif // HASHMAP_H
