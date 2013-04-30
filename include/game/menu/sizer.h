#ifndef _SIZER_H
#define _SIZER_H

typedef struct sizer_t sizer;

/*
* This should be the basic sizer prototype.
*/
struct sizer_t {
    void *obj;
    void (*render)(void *obj);
    void (*event)(void *obj);
};

#endif // _SIZER_H