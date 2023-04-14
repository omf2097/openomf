#ifndef HASHMAP_H
#define HASHMAP_H

#include "utils/iterator.h"

enum hashmap_flags
{
    HASHMAP_AUTO_INC = 0x1,
    HASHMAP_AUTO_DEC = 0x2
};

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
    unsigned int buckets_x;
    unsigned int buckets_x_min;
    unsigned int buckets_x_max;
    unsigned int reserved;
    float min_pressure;
    float max_pressure;
    unsigned int flags;
};

void hashmap_create(hashmap *hashmap, int n_size); // actual size will be 2^n_size
void hashmap_free(hashmap *hashmap);
void hashmap_set_opts(hashmap *hm, unsigned int flags, float min_pressure, float max_pressure, int buckets_min,
                      int buckets_max);
int hashmap_resize(hashmap *hm, int n_size);
float hashmap_get_pressure(hashmap *hm);
void hashmap_autoresize(hashmap *hm);
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
