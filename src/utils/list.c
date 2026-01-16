#include "utils/list.h"
#include "utils/allocator.h"
#include <stdlib.h>
#include <string.h>

void list_create(list *list) {
    list->first = NULL;
    list->last = NULL;
    list->size = 0;
    list->free = NULL;
}

void list_free(list *list) {
    if(list->size == 0) {
        return;
    }
    list_node *next, *now;
    now = list->first;
    while(now != NULL) {
        next = now->next;
        if(list->free) {
            list->free(now->data);
        }
        omf_free(now->data);
        omf_free(now);
        now = next;
    }
    list->size = 0;
}

void list_prepend(list *list, const void *ptr, size_t size) {
    list_node *node = (list_node *)omf_calloc(1, sizeof(list_node));
    node->next = list->first;
    node->prev = NULL;
    node->data = omf_calloc(size, 1);
    memcpy(node->data, (const char *)ptr, size);
    if(list->first) {
        list->first->prev = node;
    }
    if(!list->last) {
        list->last = node;
    }
    list->first = node;
    list->size++;
}

void list_append(list *list, const void *ptr, size_t size) {
    list_node *node = (list_node *)omf_calloc(1, sizeof(list_node));
    node->next = NULL;
    node->prev = list->last;
    node->data = omf_calloc(size, 1);
    memcpy(node->data, (const char *)ptr, size);
    if(list->last) {
        list->last->next = node;
    }
    if(!list->first) {
        list->first = node;
    }
    list->last = node;
    list->size++;
}

void list_delete(list *list, iterator *iter) {
    list_node *node = (list_node *)iter->vnow;
    if(node == NULL && iter->prev == NULL) {
        node = list->first;
    }
    if(node == NULL && iter->next == NULL) {
        node = list->last;
    }
    if(node == NULL) {
        return;
    }
    if(node->prev != NULL) {
        node->prev->next = node->next;
    }
    if(node->next != NULL) {
        node->next->prev = node->prev;
    }
    if(node == list->first) {
        list->first = node->next;
    }
    if(node == list->last) {
        list->last = node->prev;
    }
    if(iter->next == NULL) {
        iter->vnow = node->next;
    } else {
        iter->vnow = node->prev;
    }
    if(list->free) {
        list->free(node->data);
    }
    omf_free(node->data);
    omf_free(node);
    list->size--;
}

void *list_iter_next(iterator *iter) {
    if(iter->vnow == NULL) {
        iter->vnow = ((list *)iter->data)->first;
    } else {
        iter->vnow = ((list_node *)iter->vnow)->next;
    }
    if(iter->vnow == NULL) {
        iter->ended = 1;
        return NULL;
    }
    return ((list_node *)iter->vnow)->data;
}

void *list_iter_prev(iterator *iter) {
    if(iter->vnow == NULL) {
        iter->vnow = ((list *)iter->data)->last;
    } else {
        iter->vnow = ((list_node *)iter->vnow)->prev;
    }
    if(iter->vnow == NULL) {
        iter->ended = 1;
        return NULL;
    }
    return ((list_node *)iter->vnow)->data;
}

void *list_iter_peek_next(iterator *iter) {
    void *now = NULL;
    if(iter->vnow == NULL) {
        now = ((list *)iter->data)->first;
    } else {
        now = ((list_node *)iter->vnow)->next;
    }
    if(now == NULL) {
        return NULL;
    }
    return ((list_node *)now)->data;
}

void list_iter_append(iterator *iter, const void *ptr, size_t size) {
    if(iter->vnow == NULL) {
        list_prepend((list *)iter->data, ptr, size);
    } else {
        list_node *vnow = ((list_node *)iter->vnow);
        list_node *vnext = vnow->next;
        if(vnext == NULL) {
            list_append((list *)iter->data, ptr, size);
        } else {
            list_node *node = (list_node *)omf_calloc(1, sizeof(list_node));
            list *l = (list *)iter->data;
            node->data = omf_calloc(size, 1);
            memcpy(node->data, (const char *)ptr, size);
            vnow->next = node;
            node->prev = vnow;
            vnext->prev = node;
            node->next = vnext;
            l->size++;
        }
    }
}

void *list_get(const list *list, unsigned int i) {
    unsigned int size = list_size(list);
    if(i >= size) {
        return NULL;
    }
    iterator it;
    void *data;

    // Small optimization -- if index is closer to the end, iterate backwards.
    if(i < size / 2) {
        list_iter_begin(list, &it);
        unsigned int n = 0;
        foreach(it, data) {
            if(i == n) {
                return data;
            }
            n++;
        }
    } else {
        list_iter_end(list, &it);
        unsigned int n = size - 1;
        foreach_reverse(it, data) {
            if(i == n) {
                return data;
            }
            n--;
        }
    }
    return NULL;
}

void list_iter_begin(const list *list, iterator *iter) {
    iter->data = list;
    iter->vnow = NULL;
    iter->inow = 0;
    iter->next = list_iter_next;
    iter->peek = list_iter_peek_next;
    iter->prev = NULL;
    iter->ended = (list->first == NULL);
}

void list_iter_end(const list *list, iterator *iter) {
    iter->data = list;
    iter->vnow = NULL;
    iter->inow = 0;
    iter->next = NULL;
    iter->peek = NULL;
    iter->prev = list_iter_prev;
    iter->ended = (list->first == NULL);
}

void list_set_node_free_cb(list *list, list_node_free_cb cb) {
    list->free = cb;
}
