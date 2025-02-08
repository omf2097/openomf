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
void *hashmap_put(hashmap *hm, const void *key, unsigned int key_len, const void *val, unsigned int value_len);
int hashmap_get(hashmap *hm, const void *key, unsigned int key_len, void **value, unsigned int *value_len);
int hashmap_del(hashmap *hm, const void *key, unsigned int key_len);
void hashmap_iter_begin(const hashmap *hashmap, iterator *iter);
int hashmap_delete(hashmap *hashmap, iterator *iter);
void hashmap_clear(hashmap *hashmap);

static inline unsigned int hashmap_size(const hashmap *hm) {
    return hm->capacity;
}

static inline unsigned int hashmap_reserved(const hashmap *hm) {
    return hm->reserved;
}

static inline void hashmap_sput(hashmap *hm, const char *key, void *value, unsigned int value_len) {
    hashmap_put(hm, key, strlen(key) + 1, value, value_len);
}

static inline void hashmap_iput(hashmap *hm, unsigned int key, void *value, unsigned int value_len) {
    hashmap_put(hm, (char *)&key, sizeof(unsigned int), value, value_len);
}

static inline int hashmap_sget(hashmap *hm, const char *key, void **value, unsigned int *value_len) {
    return hashmap_get(hm, (void *)key, strlen(key) + 1, value, value_len);
}

static inline int hashmap_iget(hashmap *hm, unsigned int key, void **value, unsigned int *value_len) {
    return hashmap_get(hm, (void *)&key, sizeof(unsigned int), value, value_len);
}

static inline void hashmap_sdel(hashmap *hm, const char *key) {
    hashmap_del(hm, key, strlen(key) + 1);
}

static inline void hashmap_idel(hashmap *hm, unsigned int key) {
    hashmap_del(hm, (char *)&key, sizeof(unsigned int));
}

#endif // HASHMAP_H
