#ifndef _KEYBOARDHANDLER_H
#define _KEYBOARDHANDLER_H

#include "game/input/inputhandler.h"

typedef struct keyboardhandler_t keyboardhandler;

struct keyboardhandler_t {
    unsigned int kick_key;
    unsigned int punch_key;
    // ...
};

void keyboardhandler_create(inputhandler *handler);


#endif // _KEYBOARDHANDLER_H