/* MIN_LIB_BUILD has no in-game console. */
#ifndef MIN_LIB_BUILD
#error "console stand-in is part of MIN_LIB_BUILD"
#endif

#include "console/console.h"
#include "utils/log.h"

bool console_init(void) {
    log_info("Console is disabled.");
    return true;
}

void console_close(void) {
}

void console_event(game_state *scene, SDL_Event *event) {
    (void)scene;
    (void)event;
}

void console_render(void) {
}

void console_tick(game_state *gs) {
    (void)gs;
}

void console_add_cmd(const char *name, command_func func, const char *doc) {
    (void)name;
    (void)func;
    (void)doc;
}

void console_remove_cmd(const char *name) {
    (void)name;
}

void console_output_add(const char *text) {
    (void)text;
}

void console_output_addline(const char *text) {
    (void)text;
}

bool console_window_is_open(void) {
    return false;
}

void console_window_open(void) {
}

void console_window_close(void) {
}
