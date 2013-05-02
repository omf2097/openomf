#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include "game/har.h"

enum {
    CTRL_TYPE_KEYBOARD,
    CTRL_TYPE_GAMEPAD,
    CTRL_TYPE_NETWORK,
};

typedef struct controller_t controller;

struct controller_t {
    har *har;
    void *data;
    int type;
};

#endif // _CONTROLLER_H