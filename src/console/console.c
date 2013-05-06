#include "console/console.h"
#include "video/video.h"
#include "game/menu/menu_background.h"
#include "utils/log.h"
#include <stdlib.h>
#include <ctype.h>

#define HISTORY_MAX 100

console *con = NULL;

// Handle console commands
void console_cmd_quit(int argc, char **argv) {
    DEBUG("called %s", argv[0]);
    for(int i=1;i<argc;++i) {
        DEBUG("param %s", argv[i]);
    }
}

int make_argv(char *p, char **argv) {
    // split line into argv, warning: does not handle quoted strings
    int argc = 0;
    while(isspace(*p)) { ++p; }
    while(*p) {
        if(argv) argv[argc] = p;
        while(*p && !isspace(*p)) { ++p; }
        if(argv && *p) *p++ = '\0';
        while(isspace(*p)) { ++p; }
        ++argc;
    }
    return argc;
}

void console_handle_line() {
    int argc = make_argv(con->input, NULL);
    if(argc > 0) {
        char *argv[argc];
        void *val = 0;
        unsigned int len;
        make_argv(con->input, argv);
        if(!hashmap_sget(&con->cmds, argv[0], &val, &len)) {
            command_func *cmd = val;
            (*cmd)(argc, argv);
        }
    }
}

void console_add_history() {
    iterator it;
    if(list_size(&con->history) == HISTORY_MAX) {
        list_iter_end(&con->history, &it);
        list_delete(&con->history, &it);
    }
    list_prepend(&con->history, con->input, sizeof(con->input));
    con->histpos = -1;
}

int console_init() {
    if(con != NULL) return 1;
    con = malloc(sizeof(console));
    con->isopen = 0;
    con->ypos = 0;
    con->ticks = 0;
    con->input[0] = '\0';
    con->histpos = -1;
    list_create(&con->history);
    hashmap_create(&con->cmds, 8);
    menu_background_create(&con->background, 322, 101);
 
    // Add console commands
    console_add_cmd("quit", &console_cmd_quit);
 
    // Create font
    font_create(&con->font);
    if(font_load(&con->font, "resources/CHARSMAL.DAT", FONT_SMALL)) {
        PERROR("Error while loading small font!");
        font_free(&con->font);
        return 1;
    }
    return 0;
}

void console_close() {
    texture_free(&con->background);
    font_free(&con->font);
    list_free(&con->history);
    hashmap_free(&con->cmds);
    free(con);
}

void console_event(SDL_Event *e) {
    if (e->type == SDL_KEYDOWN) {
        unsigned char code = e->key.keysym.sym;
        unsigned char len = strlen(con->input);
        unsigned char *state = SDL_GetKeyboardState(NULL);
        /*if ((code >= SDLK_a && code <= SDLK_z) || (code >= SDLK_0 && code <= SDLK_9) || code == SDLK_SPACE || code == SDLK) {*/
        // SDLK_UP and SDLK_DOWN does not work here
        if(state[SDL_SCANCODE_UP]) {
            if(con->histpos < HISTORY_MAX && con->histpos < (signed int)(list_size(&con->history)-1)) {
                con->histpos++;
                con->histpos_changed = 1;
            }
        } else if(state[SDL_SCANCODE_DOWN]) {
            if(con->histpos > -1) {
                con->histpos--;
                con->histpos_changed = 1;
            }
        } else if(state[SDL_SCANCODE_LEFT]) {
            // TODO move cursor to the left
        } else if(state[SDL_SCANCODE_RIGHT]) {
            // TODO move cursor to the right
        } else if (state[SDL_SCANCODE_BACKSPACE] || state[SDL_SCANCODE_DELETE]) {
            if (len > 0) {
                con->input[len-1] = '\0';
            }
        } else if (state[SDL_SCANCODE_RETURN]) {
            // send the input somewhere and clear the input line
            if(con->input[0] != '\0') {
                console_add_history();
                console_handle_line();
                con->input[0] = '\0';
            }
        } else if (code >= 32 && code <= 126) {
            if (len < sizeof(con->input)-1) {
                con->input[len+1] = '\0';
                con->input[len] = code;
            }
        } 
    }
}

void console_render() {
    if (con->ypos > 0) {
        if(con->histpos != -1 && con->histpos_changed) {
            char *input = list_get(&con->history, con->histpos);
            memcpy(con->input, input, sizeof(con->input));
            con->histpos_changed = 0;
        }
        if(con->histpos == -1 && con->histpos_changed) {
            con->input[0] = '\0';
            con->histpos_changed = 0;
        }
        video_render_sprite(&con->background, -1, con->ypos - 101, BLEND_ALPHA_FULL);
        int t = con->ticks / 2;
        // input line
        font_render(&con->font, con->input, 0 , con->ypos - 7, 121, 121, 121);
        //cursor
        font_render(&con->font, "", strlen(con->input)*8 , con->ypos - 7, 121 - t, 121 - t, 121 - t);
    }
}

void console_tick() {
    if (con->isopen && con->ypos < 100) {
        con->ypos+=4;
    } else if (!con->isopen && con->ypos > 0) {
        con->ypos-=4;
    }
    if(!con->dir) {
        con->ticks++;
    } else {
        con->ticks--;
    }
    if(con->ticks > 120) {
        con->dir = 1;
    }
    if(con->ticks == 0) {
        con->dir = 0;
    }
}

void console_add_cmd(const char *name, command_func func) {
    hashmap_sput(&con->cmds, name, (void**)&func, sizeof(command_func*));
}

int console_window_is_open() {
    return con->isopen;
}

void console_window_open() {
    con->isopen = 1;
}

void console_window_close() {
    con->isopen = 0;
}
