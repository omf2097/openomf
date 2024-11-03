#ifndef CONSOLE_TYPE_H
#define CONSOLE_TYPE_H

#include "game/protos/scene.h"
#include <SDL.h>

typedef struct {
    font font;
    list history;
    int histpos;
    int histpos_changed;
    char output[4810];
    unsigned int output_head;
    unsigned int output_tail;
    unsigned int output_pos;
    int output_overflowing;
    char input[41];
    surface background1;
    surface background2;
    int isopen;
    int ownsinput;
    int ypos;
    hashmap cmds; // string -> command
} console;

typedef struct {
    command_func func;
    const char *doc;
} command;

// Console State
extern console *con;

#endif // CONSOLE_TYPE_H
