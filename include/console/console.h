#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <SDL2/SDL.h>
#include "game/protos/scene.h"

// return 0 on success, otherwise return error code
typedef int(*command_func)(scene *scene, void *userdata, int argc, char **argv);

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
