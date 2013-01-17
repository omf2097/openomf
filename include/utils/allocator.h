#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

#include <stddef.h>

typedef struct allocator_t {
    void* (*cmalloc)(size_t size);
    void  (*cfree)(void *ptr);
    void* (*crealloc)(void *ptr, size_t size);
} allocator;

#endif // _ALLOCATOR_H