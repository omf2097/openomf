#ifndef _INPUTHANDLER_H
#define _INPUTHANDLER_H

#include "game/har.h"

typedef struct inputhandler_t inputhandler;

struct inputhandler_t {
    har *har;
    void *obj;
    void (*handle)(void *obj, SDL_Event *event);
    void (*free)(void *obj);
};

void inputhandler_free(inputhandler *handler);
void inputhandler_handle(inputhandler *handler, SDL_Event *event);
void inputhandler_attach_har(inputhandler *handler, har *har);

#endif // _INPUTHANDLER_H