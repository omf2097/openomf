#include "utils/list.h"
#include <stdlib.h>

void list_create(list *list) {
    list->first = 0;
    list->last = 0;
    list->size = 0;
}

void list_free(list *list) {
    if(list->size == 0) return;
    list_node *next, *now;
    now = list->first;
    while(now != 0) {
        next = now->next;
        free(now);
        now = next;
    }
    list->size = 0;
}

void list_push_first(list *list, void *ptr) {
    list_node *node = (list_node*)malloc(sizeof(list_node));
    node->next = list->first;
    node->prev = 0;
    node->data = ptr;
    if(list->first) { list->first->prev = node; }
    if(!list->last) { list->last = node; }
    list->first = node;
    list->size++;
}

void list_push_last(list *list, void *ptr) {
    list_node *node = (list_node*)malloc(sizeof(list_node));
    node->next = 0;
    node->prev = list->last;
    node->data = ptr;
    if(list->last) { list->last->next = node; }
    if(!list->first) { list->first = node; }
    list->last = node;
    list->size++;
}

void list_delete(list *list, list_iterator *iterator) {
    list_node *node = iterator->this;
    if(node->prev) { node->prev->next = node->next; }
    if(node->next) { node->next->prev = node->prev; }
    if(node == list->first) { list->first = node->next; }
    if(node == list->last)  { list->last = node->prev;  }
    iterator->this = 0;
    iterator->next = node->next;
    free(node);
    list->size--;
}

unsigned int list_size(list *list) {
    return list->size;
}

void list_iter(list *list, list_iterator *iterator) {
    iterator->this = 0;
    iterator->next = list->first;
}

void* list_next(list_iterator *iterator) {
    iterator->this = iterator->next;
    if(iterator->this) {
        iterator->next = iterator->this->next;
    }
    if(iterator->this) {
        return iterator->this->data;
    }
    return 0;
}

