#ifndef _CONSOLE_H
#define _CONSOLE_H

#include "game/text/text.h"
#include <SDL2/SDL.h>

typedef struct console_t console;

struct console_t {
    font font;
    char buffer[512];
    char input[41];
    texture background;
    int isopen;
    int ypos;
    unsigned int ticks, dir;
};

int console_init();
void console_close();
void console_event(SDL_Event *event);
void console_render();
void console_tick();

int console_window_is_open();
void console_window_open();
void console_window_close();

#endif // _CONSOLE_H
