#include "console/console.h"
#include "video/video.h"
#include "game/menu/menu_background.h"
#include "game/scene.h"
#include "utils/log.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define HISTORY_MAX 100
#define BUFFER_INC(b) (((b) + 1) % sizeof(con->output))
#define BUFFER_DEC(b) (((b) + sizeof(con->output) - 1) % sizeof(con->output))

console *con = NULL;

int strtoint(char *input, int *output) {
    char *end;
    *output = (int)strtol(input, &end, 10);
    if(!*end) {
        return 1;
    } else {
        return 0;
    }
}
int sort_command_by_name(const void *a, const void *b) {
    hashmap_pair *const *h1 = a;
    hashmap_pair *const *h2 = b;
    return strcmp((*h1)->key, (*h2)->key);
}

// Handle console commands
int console_clear_quit(scene *scene, void *userdata, int argc, char **argv) {
    con->output[0] = '\0';
    con->output_head = 0;
    con->output_tail = 0;
    con->output_pos = 0;
    con->output_overflowing = 0;
    return 0;
}

int console_cmd_quit(scene *scene, void *userdata, int argc, char **argv) {
    scene->next_id = SCENE_CREDITS;
    return 0;
}

int console_cmd_help(scene *scene, void *userdata, int argc, char **argv) {
    // print list of commands
    vector sorted;
    iterator it;
    hashmap_pair *pair, **ppair;

    vector_create(&sorted, sizeof(hashmap_pair*));
    hashmap_iter_begin(&con->cmds, &it);
    while((pair = iter_next(&it)) != NULL) {
        vector_append(&sorted, &pair);
    }
    vector_sort(&sorted, &sort_command_by_name);
    vector_iter_begin(&sorted, &it);
    while((ppair = iter_next(&it)) != NULL) {
        char *name = (*ppair)->key;
        command *cmd = (*ppair)->val;
        console_output_add(name);
        console_output_add(" - ");
        console_output_addline(cmd->doc);
    }
    vector_free(&sorted);
    return 0;
}

int console_cmd_scene(scene *scene, void *userdata, int argc, char **argv) {
    // change scene
    if(argc == 2) {
        int i;
        if(strtoint(argv[1], &i)) {
            if(scene_is_valid(i)) {
                scene->next_id = i;
                return 0;
            }
        }
    }
    return 1;
}

int console_cmd_har(scene *scene, void *userdata, int argc, char **argv) {
    // change har
    if(argc == 2) {
        int i;
        if(strtoint(argv[1], &i)) {
            har *h = scene->player1.har;
            int hx = h->phy.pos.x;
            int hy = h->phy.pos.y;
            int hd = h->direction;
            if(h != NULL) {
                har_free(h);
                free(h);
                scene->player1.har = NULL;
            }
            h = malloc(sizeof(har));
            har_load(h, scene->bk->palettes[0], i, hx, hy, hd);
            scene_set_player1_har(scene, h);
            scene->player1.ctrl->har = h;
            return 0;
        }
    }
    return 1;
}

int console_cd_debug(scene *scene, void *userdata, int argc, char **argv) {
    if (scene->player1.har) {
        scene->player1.har->cd_debug_enabled = 1;
    }

    if (scene->player2.har) {
        scene->player2.har->cd_debug_enabled = 1;
    }
    return 0;
}

int make_argv(char *p, char **argv) {
    // split line into argv, warning: does not handle quoted strings
    int argc = 0;
    while(isspace(*p)) { ++p; }
    while(*p) {
        if(argv != NULL) argv[argc] = p;
        while(*p && !isspace(*p)) { ++p; }
        if(argv != NULL && *p) *p++ = '\0';
        while(isspace(*p)) { ++p; }
        ++argc;
    }
    return argc;
}

void console_add_history(const char *input, unsigned int len) {
    iterator it;
    if(list_size(&con->history) == HISTORY_MAX) {
        list_iter_end(&con->history, &it);
        list_delete(&con->history, &it);
    }
    list_prepend(&con->history, input, len);
    con->histpos = -1;
}

void console_handle_line(scene *scene) {
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
                int err = cmd->func(scene, cmd->userdata, argc, argv);
                if(err == 0)
                {
                    console_output_add("> ");
                    console_output_add(argv[0]);
                    console_output_addline(" SUCCESS");
                } else {
                    char buf[12];
                    sprintf(buf, "%d", err);
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

void console_output_scroll_to_end() {
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

    for(;
        si != con->output_head;
        si = BUFFER_DEC(si)) {

        if(con->output[si] == '\n') {
            lines++;

            if(lines >= 16) {
                si = BUFFER_INC(si);
                break;
            }
        }
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
            if(l == lines){
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
            if(l == lines){
                con->output_pos = BUFFER_INC(con->output_pos);
                break;
            }
        }
    }
}

void console_output_add(const char *text) {
    size_t len = strlen(text);
    for(size_t i = 0;i < len;++i) {
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

void console_output_render() {
    int x = 0;
    int y = 0;
    unsigned int lines = 0;
    for(unsigned int i = con->output_pos;
        i != con->output_tail && lines < 15;
        i = BUFFER_INC(i)) {

        char c = con->output[i];
        if(c == '\n') {
            x = 0;
            y += font_small.h;
            lines++;
        } else {
            // TODO add word wrapping?
            font_render_char(&font_small, c, x, y+con->ypos-100, 121, 121, 121);
            x += font_small.w;
        }
    }
}

int console_init() {
    if(con != NULL) return 1;
    con = malloc(sizeof(console));
    con->isopen = 0;
    con->ypos = 0;
    con->ticks = 0;
    con->input[0] = '\0';
    con->output[0] = '\0';
    con->output_head = 0;
    con->output_tail = 0;
    con->output_pos = 0;
    con->output_overflowing = 0;
    con->histpos = -1;
    list_create(&con->history);
    hashmap_create(&con->cmds, 8);
    menu_background_create(&con->background, 322, 101);
 
    // Add console commands
    console_add_cmd("clear", &console_clear_quit,  "clear the console");
    console_add_cmd("cls",   &console_clear_quit,  "clear the console");
    console_add_cmd("quit",  &console_cmd_quit,  "quit the game");
    console_add_cmd("exit",  &console_cmd_quit,  "quit the game");
    console_add_cmd("help",  &console_cmd_help,  "show all commands");
    console_add_cmd("scene", &console_cmd_scene, "change scene. usage: scene 1, scene 2, etc");
    console_add_cmd("har",   &console_cmd_har,   "change har. usage: har 1, har 2, etc");
    console_add_cmd("cd-debug",&console_cd_debug,   "toggle collision detection debugging");
    
    return 0;
}

void console_close() {
    texture_free(&con->background);
    list_free(&con->history);
    hashmap_free(&con->cmds);
    free(con);
}

void console_event(scene *scene, SDL_Event *e) {
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
            console_handle_line(scene);
            con->input[0] = '\0';
        } else if(state[SDL_SCANCODE_PAGEUP]) {
            console_output_scroll_up(1);
        } else if(state[SDL_SCANCODE_PAGEDOWN]) {
            console_output_scroll_down(1);
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
        font_render(&font_small, con->input, 0 , con->ypos - 7, 121, 121, 121);
        //cursor
        font_render(&font_small, "", strlen(con->input)*font_small.w , con->ypos - 7, 121 - t, 121 - t, 121 - t);
        console_output_render();
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

void console_add_cmd(const char *name, command_func func, const char *doc) {
    command c;
    c.func = func;
    c.userdata = NULL;
    c.doc = doc;
    hashmap_sput(&con->cmds, name, &c, sizeof(command));
}

void console_set_userdata(const char *name, void *userdata) {
    void *val;
    unsigned int len;
    if(!hashmap_sget(&con->cmds, name, &val, &len)) {
        command *c = val;
        c->userdata = userdata;
    }
}

void *console_get_userdata(const char *name) {
    void *val;
    unsigned int len;
    if(!hashmap_sget(&con->cmds, name, &val, &len)) {
        return val;
    }
    return NULL;
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
