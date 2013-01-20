#ifndef _GAMEPADHANDLER_H
#define _GAMEPADHANDLER_H

#include "game/input/inputhandler.h"

typedef struct gamepadhandler_t gamepadhandler;

struct gamepadhandler_t {
    unsigned int kick_key;
    unsigned int punch_key;
    // ...
};

void gamepadhandler_create(inputhandler *handler);


#endif // _GAMEPADHANDLER_H