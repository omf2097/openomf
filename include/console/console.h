#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <SDL2/SDL.h>
#include "utils/list.h"
#include "utils/hashmap.h"
#include "game/text/text.h"
#include "game/scene.h"

typedef struct console_t console;
typedef struct command_t command;

// return 0 on success, otherwise return error code
typedef int(*command_func)(scene *scene, void *userdata, int argc, char **argv);

struct console_t {
    font font;
    list history;
    int histpos;
    int histpos_changed;
    char output[4810];
    unsigned int output_head, output_tail, output_pos;
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
void console_remove_cmd(const char *name);
void console_set_userdata(const char *name, void *userdata);
void *console_get_userdata(const char *name);

void console_output_add(const char *text);
void console_output_addline(const char *text);

int console_window_is_open();
void console_window_open();
void console_window_close();

#endif // _CONSOLE_H
