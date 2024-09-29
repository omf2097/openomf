#ifndef CONSOLE_H
#define CONSOLE_H

#include "game/game_state_type.h"
#include <SDL.h>

// return 0 on success, otherwise return error code
typedef int (*command_func)(game_state *scene, int argc, char **argv);

int console_init(void);
void console_close(void);
void console_event(game_state *scene, SDL_Event *event);
void console_render(void);
void console_tick(void);
void console_add_cmd(const char *name, command_func func, const char *doc);
void console_remove_cmd(const char *name);

void console_output_add(const char *text);
void console_output_addline(const char *text);

int console_window_is_open(void);
void console_window_open(void);
void console_window_close(void);

#endif // CONSOLE_H
