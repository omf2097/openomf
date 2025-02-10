#include "utils/iterator.h"
#include <assert.h>

void *iter_next(iterator *iter) {
    assert(iter->next);
    if(iter->ended)
        return NULL;
    return iter->next(iter);
}

void *iter_peek(iterator *iter) {
    assert(iter->peek);
    if(iter->ended)
        return NULL;
    return iter->peek(iter);
}

void *iter_prev(iterator *iter) {
    assert(iter->prev);
    if(iter->ended)
        return NULL;
    return iter->prev(iter);
}
