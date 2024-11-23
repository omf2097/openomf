#include "console/console.h"
#include "console/console_type.h"
#include "game/gui/menu_background.h"
#include "game/utils/settings.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/log.h"
#include "video/video.h"
#include <stdio.h>

#if !defined(WIN32) && !defined(_WIN32)
#include <fcntl.h>
#include <unistd.h>
#endif

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
    con->hist_pos = -1;
}

void console_handle_line(game_state *gs) {
    str_strip(&con->input);
    if(str_size(&con->input) == 0) {
        console_output_addline(">");
    } else {
        char input_copy[CONSOLE_LINE_MAX];
        strncpy_or_truncate(input_copy, str_c(&con->input), sizeof(input_copy));
        int argc = make_argv(input_copy, NULL);
        if(argc > 0) {
            char **argv = omf_calloc(argc, sizeof(char *));
            void *val = 0;
            unsigned int len;
            make_argv(input_copy, argv);
            if(!hashmap_sget(&con->cmds, argv[0], &val, &len)) {
                command *cmd = val;
                int err = cmd->func(gs, argc, argv);
                if(err == 0) {
                    console_output_add("> ");
                    console_output_add(argv[0]);
                    console_output_addline(" SUCCESS");
                    DEBUG("Console command %s succeeded", argv[0]);
                } else {
                    char buf[12];
                    snprintf(buf, 12, "%d", err);
                    console_output_add("> ");
                    console_output_add(argv[0]);
                    console_output_add(" ERROR:");
                    console_output_addline(buf);
                    DEBUG("Error in console command %s: %s", argv[0], buf);
                }
                console_add_history(input_copy, sizeof(input_copy));
            } else {
                console_output_add("> ");
                console_output_add(argv[0]);
                console_output_addline(" NOT RECOGNIZED");
                DEBUG("Console command %s not recognized", argv[0]);
            }
            omf_free(argv);
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
    text_settings tconf;
    text_defaults(&tconf);
    tconf.cforeground = TEXT_MEDIUM_GREEN;
    tconf.font = FONT_SMALL;
    for(unsigned int i = con->output_pos; i != con->output_tail && lines < 15; i = BUFFER_INC(i)) {

        char c = con->output[i];
        if(c == '\n') {
            x = 0;
            y += font_small.h;
            lines++;
        } else {
            // TODO add word wrapping?
            text_render_char_uncached(&tconf, TEXT_DEFAULT, x, y + con->y_pos - 100, c, false);
            x += font_small.w;
        }
    }
}

bool console_init(void) {
    if(con != NULL)
        return false;
    con = omf_calloc(1, sizeof(console));
    con->is_open = false;
    con->owns_input = false;
    con->y_pos = 0;
    str_create(&con->input);
    con->output[0] = '\0';
    con->output_head = 0;
    con->output_tail = 0;
    con->output_pos = 0;
    con->output_overflowing = 0;
    con->hist_pos = -1;
    con->hist_pos_changed = 0;
    list_create(&con->history);
    hashmap_create(&con->cmds);
    menu_transparent_bg_create(&con->background1, 322, 101);
    menu_background_create(&con->background2, 322, 101, MenuBackground);

    // Read from stdin needs to be nonblocking
#if !defined(WIN32) && !defined(_WIN32)
    fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
#endif

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

    return true;
}

void console_close(void) {
    surface_free(&con->background1);
    surface_free(&con->background2);
    list_free(&con->history);
    hashmap_free(&con->cmds);
    str_free(&con->input);
    omf_free(con);
}

void console_event(game_state *gs, SDL_Event *e) {
    if(e->type == SDL_TEXTINPUT) {
        size_t len = str_size(&con->input);
        if(strlen(e->text.text) == 1) {
            // make sure it is not a unicode sequence
            char c = e->text.text[0];
            // only allow ASCII through
            if(isprint(c) && len < CONSOLE_LINE_MAX - 1) {
                c = tolower(c);
                str_append_buf(&con->input, &c, 1);
            }
        }
    } else if(e->type == SDL_KEYDOWN) {
        size_t len = str_size(&con->input);
        unsigned char scancode = e->key.keysym.scancode;
        /*if ((code >= SDLK_a && code <= SDLK_z) || (code >= SDLK_0 && code <= SDLK_9) || code == SDLK_SPACE || code ==
         * SDLK) {*/
        // SDLK_UP and SDLK_DOWN does not work here
        if(scancode == SDL_SCANCODE_UP) {
            if(con->hist_pos < HISTORY_MAX && con->hist_pos < (signed int)(list_size(&con->history) - 1)) {
                con->hist_pos++;
                con->hist_pos_changed = 1;
            }
        } else if(scancode == SDL_SCANCODE_DOWN) {
            if(con->hist_pos > -1) {
                con->hist_pos--;
                con->hist_pos_changed = 1;
            }
        } else if(scancode == SDL_SCANCODE_LEFT) {
            // TODO move cursor to the left
        } else if(scancode == SDL_SCANCODE_RIGHT) {
            // TODO move cursor to the right
        } else if(scancode == SDL_SCANCODE_BACKSPACE || scancode == SDL_SCANCODE_DELETE) {
            if(len > 0) {
                str_truncate(&con->input, len - 1);
            }
        } else if(scancode == SDL_SCANCODE_RETURN || scancode == SDL_SCANCODE_KP_ENTER) {
            // send the input somewhere and clear the input line
            console_handle_line(gs);
            str_truncate(&con->input, 0);
        } else if(scancode == SDL_SCANCODE_PAGEUP) {
            console_output_scroll_up(1);
        } else if(scancode == SDL_SCANCODE_PAGEDOWN) {
            console_output_scroll_down(1);
        }
    }
}

void console_render(void) {
    if(con == NULL) {
        return;
    }

    static text_object text_cache[2] = {0};
    if(con->y_pos > 0) {
        if(con->hist_pos != -1 && con->hist_pos_changed) {
            const char *input = list_get(&con->history, con->hist_pos);
            str_free(&con->input);
            str_from_c(&con->input, input);
            con->hist_pos_changed = 0;
        }
        if(con->hist_pos == -1 && con->hist_pos_changed) {
            str_truncate(&con->input, 0);
            con->hist_pos_changed = 0;
        }
        video_draw_remap(&con->background1, -1, con->y_pos - 101, 4, 1, 0);
        video_draw(&con->background2, -1, con->y_pos - 101);
        text_settings tconf;
        text_defaults(&tconf);
        tconf.font = FONT_SMALL;
        // input line
        tconf.cforeground = TEXT_MEDIUM_GREEN;
        text_render(&text_cache[0], &tconf, TEXT_DEFAULT, 0, con->y_pos - 7, 300, 6, con->input);

        // cursor
        tconf.cforeground = TEXT_BLINKY_GREEN;
        text_render(&text_cache[1], &tconf, TEXT_DEFAULT, strlen(con->input) * font_small.w, con->y_pos - 7, 6, 6,
                    CURSOR_STR);
        console_output_render();
    }
}

static size_t get_stdin_line(char *line, size_t size) {
#if !defined(WIN32) && !defined(_WIN32)
    ssize_t n = read(0, line, size);
    if(n > 0) {
        line[n - 1] = '\0';
        return (size_t)(n - 1);
    }
#endif
    return 0;
}

void console_tick(game_state *gs) {
    char buf[CONSOLE_LINE_MAX];
    if(get_stdin_line(buf, sizeof(buf)) > 0) {
        str_free(&con->input);
        str_from_c(&con->input, buf);
        DEBUG("Console line from stdin: %s", str_c(&con->input));
        console_handle_line(gs);
        str_truncate(&con->input, 0);
    }
    if(con->is_open && con->y_pos < 100) {
        con->y_pos += 4;
        if(settings_get()->video.instant_console) {
            con->y_pos = 100;
        }
    } else if(!con->is_open && con->y_pos > 0) {
        con->y_pos -= 4;
        if(settings_get()->video.instant_console) {
            con->y_pos = 0;
        }
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

bool console_window_is_open(void) {
    return con->is_open;
}

void console_window_open(void) {
    if(!SDL_IsTextInputActive()) {
        SDL_StartTextInput();
        con->owns_input = true;
    } else {
        con->owns_input = false;
    }
    con->is_open = true;
}

void console_window_close(void) {
    if(con->owns_input) {
        SDL_StopTextInput();
    }
    con->is_open = false;
}
