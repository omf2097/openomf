#ifndef _LIST_H
#define _LIST_H

typedef struct list_node_t list_node;

typedef struct list_node_t {
    list_node *prev;
    list_node *next;
    void *data;
} list_node;

typedef struct list_t {
    list_node *first;
    list_node *last;
    unsigned int size;
} list;

typedef struct list_iterator_t {
    list_node *this;
    list_node *next;
} list_iterator;

void list_create(list *list);
void list_free(list *list);
void list_push_first(list *list, void *ptr);
void list_push_last(list *list, void *ptr);
void list_delete(list *list, list_iterator *iterator);
unsigned int list_size(list *list);
void  list_iter(list *list, list_iterator *iterator);
void* list_next(list_iterator *iterator);

#endif