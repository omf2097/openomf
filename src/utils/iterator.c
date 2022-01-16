#include "utils/iterator.h"
#include <stdlib.h>

void *iter_next(iterator *iter) {
    if(iter->next == NULL)
        return NULL;
    if(iter->ended)
        return NULL;
    return iter->next(iter);
}

void *iter_prev(iterator *iter) {
    if(iter->prev == NULL)
        return NULL;
    if(iter->ended)
        return NULL;
    return iter->prev(iter);
}
