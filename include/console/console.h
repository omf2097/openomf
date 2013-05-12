#ifndef _CONSOLE_H
#define _CONSOLE_H

#include "game/text/text.h"
#include "utils/list.h"
#include "utils/hashmap.h"
#include <SDL2/SDL.h>

typedef struct scene_t scene;
typedef struct console_t console;
typedef struct command_t command;

typedef void(*command_func)(scene *scene, void *userdata, int argc, char **argv);

struct console_t {
    font font;
    list history;
    int histpos;
    int histpos_changed;
    char output[481];
    unsigned int output_head, output_tail;
    int output_overflowing;
    char input[41];
    texture background;
    int isopen;
    int ypos;
    unsigned int ticks, dir;
    hashmap cmds; // string -> command
};

struct command_t {
    command_func func;
    void *userdata;
    const char *doc;
};

int console_init();
void console_close();
void console_event(scene *scene, SDL_Event *event);
void console_render();
void console_tick();
void console_add_cmd(const char *name, command_func func, const char *doc);
void console_set_userdata(const char *name, void *userdata);
void *console_get_userdata(const char *name);

int console_window_is_open();
void console_window_open();
void console_window_close();

#endif // _CONSOLE_H
