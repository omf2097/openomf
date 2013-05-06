#ifndef _CONSOLE_H
#define _CONSOLE_H

#include "game/text/text.h"
#include "utils/list.h"
#include "utils/hashmap.h"
#include <SDL2/SDL.h>

typedef void(*command_func)(int argc, char **argv);
typedef struct console_t console;

struct console_t {
    font font;
    list history;
    int histpos;
    int histpos_changed;
    char input[41];
    texture background;
    int isopen;
    int ypos;
    unsigned int ticks, dir;
    hashmap cmds; // string -> command_func
};

int console_init();
void console_close();
void console_event(SDL_Event *event);
void console_render();
void console_tick();
void console_add_cmd(const char *name, command_func func);

int console_window_is_open();
void console_window_open();
void console_window_close();

#endif // _CONSOLE_H
