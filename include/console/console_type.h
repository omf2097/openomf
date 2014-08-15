#ifndef _CONSOLE_TYPE_H
#define _CONSOLE_TYPE_H

#include <SDL2/SDL.h>
#include "game/protos/scene.h"

typedef struct console_t {
    font font;
    list history;
    int histpos;
    int histpos_changed;
    char output[4810];
    unsigned int output_head, output_tail, output_pos;
    int output_overflowing;
    char input[41];
    surface background;
    int isopen;
    int ownsinput;
    int ypos;
    unsigned int ticks, dir;
    hashmap cmds; // string -> command
} console;

typedef struct command_t {
    command_func func;
    const char *doc;
} command;

// Console State
extern console *con;

#endif // _CONSOLE_TYPE_H
