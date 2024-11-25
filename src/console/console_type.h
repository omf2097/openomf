#ifndef CONSOLE_TYPE_H
#define CONSOLE_TYPE_H

#include "game/protos/scene.h"
#include <SDL.h>
#include <stdbool.h>

typedef struct console {
    font font;
    list history;
    int hist_pos;
    int hist_pos_changed;
    char output[4810];
    unsigned int output_head;
    unsigned int output_tail;
    unsigned int output_pos;
    int output_overflowing;
    char input[41];
    surface background1;
    surface background2;
    bool is_open;
    bool owns_input;
    int y_pos;
    text_object text_cache[2];
    hashmap cmds; // string -> command
} console;

typedef struct command {
    command_func func;
    const char *doc;
} command;

// Console State
extern console *con;

#endif // CONSOLE_TYPE_H
