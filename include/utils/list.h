#ifndef LIST_H
#define LIST_H

#include "iterator.h"
#include <stddef.h>

typedef struct list_node_t list_node;

struct list_node_t {
    list_node *prev;
    list_node *next;
    void *data;
};

typedef struct list_t {
    list_node *first;
    list_node *last;
    unsigned int size;
} list;

void list_create(list *list);
void list_free(list *list);
void list_prepend(list *list, const void *ptr, size_t size);
void list_append(list *list, const void *ptr, size_t size);
void list_delete(list *list, iterator *iter);
unsigned int list_size(const list *list);
void list_iter_begin(const list *list, iterator *iter);
void list_iter_end(const list *list, iterator *iter);
void *list_get(const list *list, unsigned int i);

#endif // LIST_H
