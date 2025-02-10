#ifndef LIST_H
#define LIST_H

#include "iterator.h"
#include <stddef.h>

typedef struct list_node list_node;
typedef struct list list;
typedef void (*list_node_free_cb)(void *data);

struct list_node {
    list_node *prev;
    list_node *next;
    void *data;
};

struct list {
    list_node *first;
    list_node *last;
    unsigned int size;
    list_node_free_cb free;
};

void list_create(list *list);
void list_free(list *list);
void list_prepend(list *list, const void *ptr, size_t size);
void list_append(list *list, const void *ptr, size_t size);
void list_delete(list *list, iterator *iter);
void list_iter_begin(const list *list, iterator *iter);
void list_iter_end(const list *list, iterator *iter);
void list_iter_append(iterator *iter, const void *ptr, size_t size);
void *list_get(const list *list, unsigned int i);
void list_set_node_free_cb(list *list, list_node_free_cb cb);

static inline unsigned int list_size(const list *list) {
    return list->size;
}

#endif // LIST_H
