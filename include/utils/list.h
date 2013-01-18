#ifndef _LIST_H
#define _LIST_H

#include "iterator.h"
#include "allocator.h"

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
    allocator alloc;
} list;

void list_create(list *list);
void list_create_with_allocator(list *list, allocator alloc);
void list_free(list *list);
void list_prepend(list *list, const void *ptr, int size);
void list_append(list *list, const void *ptr, int size);
void list_delete(list *list, iterator *iter);
unsigned int list_size(list *list);
void list_iter_begin(list *list, iterator *iter);
void list_iter_end(list *list, iterator *iter);

#endif