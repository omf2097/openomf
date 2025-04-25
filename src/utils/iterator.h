#ifndef ITERATOR_H
#define ITERATOR_H

typedef struct iterator_t iterator;

struct iterator_t {
    const void *data;
    void *vnow;
    int inow;
    int ended;
    void *(*next)(iterator *);
    void *(*prev)(iterator *);
    void *(*peek)(iterator *);
};

void *iter_next(iterator *iterator);
void *iter_peek(iterator *iterator);
void *iter_prev(iterator *iterator);

#define foreach(iterator, item) while((item = iter_next(&iterator)) != NULL)
#define foreach_reverse(iterator, item) while((item = iter_prev(&iterator)) != NULL)

#endif // ITERATOR_H
