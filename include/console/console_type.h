#ifndef _CONSOLE_TYPE_H
#define _CONSOLE_TYPE_H

#include <SDL.h>
#include "game/protos/scene.h"

typedef struct console_t {
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
    surface background;
    int isopen;
    int ownsinput;
    int ypos;
    unsigned int ticks;
    unsigned int dir;
    hashmap cmds; // string -> command
} console;

typedef struct command_t {
    command_func func;
    const char *doc;
} command;

// Console State
extern console *con;

#endif // _CONSOLE_TYPE_H
