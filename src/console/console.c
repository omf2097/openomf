#include "console/console.h"
#include "console/console_type.h"
#include "game/gui/menu_background.h"
#include "game/utils/settings.h"
#include "utils/allocator.h"
#include "video/video.h"
#include <stdio.h>

#define HISTORY_MAX 100
#define BUFFER_INC(b) (((b) + 1) % sizeof(con->output))
#define BUFFER_DEC(b) (((b) + sizeof(con->output) - 1) % sizeof(con->output))

#define CURSOR_STR "\x7f"

// Console State
console *con = NULL;

// defined in console_cmd.c
void console_init_cmd(void);

int make_argv(char *p, char **argv) {
    // split line into argv, warning: does not handle quoted strings
    int argc = 0;
    while(isspace(*p)) {
        ++p;
    }
    while(*p) {
        if(argv != NULL) {
            argv[argc] = p;
        }
        while(*p && !isspace(*p)) {
            ++p;
        }
        if(argv != NULL && *p) {
            *p++ = '\0';
        }
        while(isspace(*p)) {
            ++p;
        }
        ++argc;
    }
    return argc;
}

void console_add_history(const char *input, unsigned int len) {
    iterator it;
    list_iter_begin(&con->history, &it);

    char *input2 = iter_next(&it);
    if(input2 != NULL) {
        if(strcmp(input, input2) != 0) {
            if(list_size(&con->history) == HISTORY_MAX) {
                list_iter_end(&con->history, &it);
                list_delete(&con->history, &it);
            }
            list_prepend(&con->history, input, len);
        }
    } else if(input2 == NULL) {
        list_prepend(&con->history, input, len);
    }
    con->histpos = -1;
}

void console_handle_line(game_state *gs) {
    if(con->input[0] == '\0') {
        console_output_addline(">");
    } else {
        char input_copy[sizeof(con->input)];
        memcpy(input_copy, con->input, sizeof(con->input));
        int argc = make_argv(con->input, NULL);
        if(argc > 0) {
            char *argv[argc];
            void *val = 0;
            unsigned int len;
            make_argv(con->input, argv);
            if(!hashmap_sget(&con->cmds, argv[0], &val, &len)) {
                command *cmd = val;
                int err = cmd->func(gs, argc, argv);
                if(err == 0) {
                    console_output_add("> ");
                    console_output_add(argv[0]);
                    console_output_addline(" SUCCESS");
                } else {
                    char buf[12];
                    snprintf(buf, 12, "%d", err);
                    console_output_add("> ");
                    console_output_add(argv[0]);
                    console_output_add(" ERROR:");
                    console_output_addline(buf);
                }
                console_add_history(input_copy, sizeof(input_copy));
            } else {
                console_output_add("> ");
                console_output_add(argv[0]);
                console_output_addline(" NOT RECOGNIZED");
            }
        } else {
            console_output_addline(">");
        }
    }
}

void console_output_scroll_to_end(void) {
    // iterate the output buffer backward to count up to 16 lines or 480 chars, whichever comes first
    unsigned int lines = 0;
    unsigned int si;
    if(con->output_overflowing) {
        si = BUFFER_DEC(con->output_head);
        si = BUFFER_DEC(si);
    } else if(con->output_tail == con->output_head) {
        si = con->output_tail;
    } else {
        si = BUFFER_DEC(con->output_tail);
    }

    while(si != con->output_head) {
        if(con->output[si] == '\n') {
            lines++;

            if(lines >= 16) {
                si = BUFFER_INC(si);
                break;
            }
        }
        si = BUFFER_DEC(si);
    }

    con->output_pos = si;
}

void console_output_scroll_up(unsigned int lines) {
    unsigned int l = 0;
    if(con->output_pos != con->output_head) {
        con->output_pos = BUFFER_DEC(con->output_pos);
    }
    while(con->output_pos != con->output_head) {
        con->output_pos = BUFFER_DEC(con->output_pos);
        if(con->output[con->output_pos] == '\n') {
            l++;
            if(l == lines) {
                con->output_pos = BUFFER_INC(con->output_pos);
                break;
            }
        }
    }
}

void console_output_scroll_down(unsigned int lines) {
    unsigned int l = 0;
    while(con->output_pos != con->output_tail) {
        con->output_pos = BUFFER_INC(con->output_pos);
        if(con->output[con->output_pos] == '\n') {
            l++;
            if(l == lines) {
                con->output_pos = BUFFER_INC(con->output_pos);
                break;
            }
        }
    }
}

void console_output_add(const char *text) {
    size_t len = strlen(text);
    for(size_t i = 0; i < len; ++i) {
        char c = text[i];
        con->output[con->output_tail] = c;
        con->output_tail = BUFFER_INC(con->output_tail);
        if(con->output_tail == con->output_head) {
            // buffer is overflowing
            con->output_head = BUFFER_INC(con->output_head);
            con->output_overflowing = 1;
        }
    }

    console_output_scroll_to_end();
}
void console_output_addline(const char *text) {
    console_output_add(text);
    console_output_add("\n");
}

void console_output_render(void) {
    int x = 0;
    int y = 0;
    unsigned int lines = 0;
    const color textcolor = color_create(121, 121, 121, 255);
    for(unsigned int i = con->output_pos; i != con->output_tail && lines < 15; i = BUFFER_INC(i)) {

        char c = con->output[i];
        if(c == '\n') {
            x = 0;
            y += font_small.h;
            lines++;
        } else {
            // TODO add word wrapping?
            font_render_char(&font_small, c, x, y + con->ypos - 100, textcolor);
            x += font_small.w;
        }
    }
}

int console_init(void) {
    if(con != NULL)
        return 1;
    con = omf_calloc(1, sizeof(console));
    con->isopen = 0;
    con->ownsinput = 0;
    con->ypos = 0;
    con->ticks = 0;
    con->dir = 0;
    con->input[0] = '\0';
    con->output[0] = '\0';
    con->output_head = 0;
    con->output_tail = 0;
    con->output_pos = 0;
    con->output_overflowing = 0;
    con->histpos = -1;
    con->histpos_changed = 0;
    list_create(&con->history);
    hashmap_create(&con->cmds);
    menu_background_create(&con->background, 322, 101);

    console_init_cmd();

    // Print the header
    for(int i = 0; i < 37; i++) {
        console_output_add(CURSOR_STR);
    }
    console_output_addline("");
    console_output_addline(CURSOR_STR "                                   " CURSOR_STR "\n" CURSOR_STR
                                      " OpenOMF Debug Console Cheat Sheet " CURSOR_STR "\n" CURSOR_STR
                                      "                                   " CURSOR_STR "\n" CURSOR_STR
                                      " PageUp - Scroll Up                " CURSOR_STR "\n" CURSOR_STR
                                      " PageDn - Scroll Down              " CURSOR_STR "\n" CURSOR_STR
                                      " Up     - Reissue Previous Command " CURSOR_STR "\n" CURSOR_STR
                                      " Down   - Reissue Next Command     " CURSOR_STR "\n" CURSOR_STR
                                      " Enter  - Execute Current Command  " CURSOR_STR "\n" CURSOR_STR
                                      " --------------------------------- " CURSOR_STR "\n" CURSOR_STR
                                      " Type in Help to explore more      " CURSOR_STR "\n" CURSOR_STR
                                      " --------------------------------- " CURSOR_STR "\n" CURSOR_STR
                                      "                                   " CURSOR_STR);
    for(int i = 0; i < 37; i++) {
        console_output_add(CURSOR_STR);
    }
    console_output_addline("\n");

    return 0;
}

void console_close(void) {
    surface_free(&con->background);
    list_free(&con->history);
    hashmap_free(&con->cmds);
    omf_free(con);
}

void console_event(game_state *gs, SDL_Event *e) {
    if(e->type == SDL_TEXTINPUT) {
        size_t len = strlen(con->input);
        if(strlen(e->text.text) == 1) {
            // make sure it is not a unicode sequence
            unsigned char c = e->text.text[0];
            // only allow ASCII through
            if(isprint(c) && len < sizeof(con->input) - 1) {
                con->input[len + 1] = '\0';
                con->input[len] = tolower(c);
            }
        }
    } else if(e->type == SDL_KEYDOWN) {
        size_t len = strlen(con->input);
        unsigned char scancode = e->key.keysym.scancode;
        /*if ((code >= SDLK_a && code <= SDLK_z) || (code >= SDLK_0 && code <= SDLK_9) || code == SDLK_SPACE || code ==
         * SDLK) {*/
        // SDLK_UP and SDLK_DOWN does not work here
        if(scancode == SDL_SCANCODE_UP) {
            if(con->histpos < HISTORY_MAX && con->histpos < (signed int)(list_size(&con->history) - 1)) {
                con->histpos++;
                con->histpos_changed = 1;
            }
        } else if(scancode == SDL_SCANCODE_DOWN) {
            if(con->histpos > -1) {
                con->histpos--;
                con->histpos_changed = 1;
            }
        } else if(scancode == SDL_SCANCODE_LEFT) {
            // TODO move cursor to the left
        } else if(scancode == SDL_SCANCODE_RIGHT) {
            // TODO move cursor to the right
        } else if(scancode == SDL_SCANCODE_BACKSPACE || scancode == SDL_SCANCODE_DELETE) {
            if(len > 0) {
                con->input[len - 1] = '\0';
            }
        } else if(scancode == SDL_SCANCODE_RETURN || scancode == SDL_SCANCODE_KP_ENTER) {
            // send the input somewhere and clear the input line
            console_handle_line(gs);
            con->input[0] = '\0';
        } else if(scancode == SDL_SCANCODE_PAGEUP) {
            console_output_scroll_up(1);
        } else if(scancode == SDL_SCANCODE_PAGEDOWN) {
            console_output_scroll_down(1);
        }
    }
}

void console_render(void) {
    if(con->ypos > 0) {
        if(con->histpos != -1 && con->histpos_changed) {
            char *input = list_get(&con->history, con->histpos);
            memcpy(con->input, input, sizeof(con->input));
            con->histpos_changed = 0;
        }
        if(con->histpos == -1 && con->histpos_changed) {
            con->input[0] = '\0';
            con->histpos_changed = 0;
        }
        video_render_sprite(&con->background, -1, con->ypos - 101, BLEND_ALPHA, 0);
        int t = con->ticks / 2;
        // input line
        font_render(&font_small, con->input, 0, con->ypos - 7, color_create(121, 121, 121, 255));
        // cursor
        font_render(&font_small, CURSOR_STR, strlen(con->input) * font_small.w, con->ypos - 7,
                    color_create(121 - t, 121 - t, 121 - t, 255));
        console_output_render();
    }
}

void console_tick(void) {
    if(con->isopen && con->ypos < 100) {
        con->ypos += 4;
        if(settings_get()->video.instant_console) {
            con->ypos = 100;
        }
    } else if(!con->isopen && con->ypos > 0) {
        con->ypos -= 4;
        if(settings_get()->video.instant_console) {
            con->ypos = 0;
        }
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

void console_add_cmd(const char *name, command_func func, const char *doc) {
    command c;
    c.func = func;
    c.doc = doc;
    hashmap_sput(&con->cmds, name, &c, sizeof(command));
}

void console_remove_cmd(const char *name) {
    hashmap_sdel(&con->cmds, name);
}

int console_window_is_open(void) {
    return con->isopen;
}

void console_window_open(void) {
    if(!SDL_IsTextInputActive()) {
        SDL_StartTextInput();
        con->ownsinput = 1;
    } else {
        con->ownsinput = 0;
    }
    con->isopen = 1;
}

void console_window_close(void) {
    if(con->ownsinput) {
        SDL_StopTextInput();
    }
    con->isopen = 0;
}
