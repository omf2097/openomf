#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <enet/enet.h>
#include <controller/keyboard.h>
#include <controller/net_controller.h>
#include "console/console.h"
#include "game/menu/menu_background.h"
#include "game/protos/scene.h"
#include "game/game_state.h"
#include "game/scenes/arena.h"
#include "game/settings.h"
#include "game/text/text.h"
#include "resources/ids.h"
#include "utils/log.h"
#include "utils/list.h"
#include "utils/hashmap.h"
#include "video/video.h"

#define HISTORY_MAX 100
#define BUFFER_INC(b) (((b) + 1) % sizeof(con->output))
#define BUFFER_DEC(b) (((b) + sizeof(con->output) - 1) % sizeof(con->output))

typedef struct console_t {
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
} console;

typedef struct command_t {
    command_func func;
    void *userdata;
    const char *doc;
} command;

// Console State
static console *con = NULL;

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
int console_cmd_history(scene *scene, void *userdata, int argc, char **argv) {
    iterator it;
    char *input;
    char buf[sizeof(con->input)];
    int i = 1;

    list_iter_begin(&con->history, &it);
    while((input = iter_next(&it)) != NULL) {
        sprintf(buf, "%d. %s", i, input);
        console_output_addline(buf);
        i++;
    }
    return 0;
}

int console_cmd_clear(scene *scene, void *userdata, int argc, char **argv) {
    con->output[0] = '\0';
    con->output_head = 0;
    con->output_tail = 0;
    con->output_pos = 0;
    con->output_overflowing = 0;
    return 0;
}

int console_cmd_quit(scene *scene, void *userdata, int argc, char **argv) {
    game_state_set_next(scene->gs, SCENE_CREDITS);
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
            if(is_scene(i)) {
                game_state_set_next(scene->gs, i);
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
            if(i < 0 || i > 10) {
                return 1;
            }

            game_player *player = game_state_get_player(scene->gs, 0);

            object *har_obj = game_player_get_har(player);
            object *obj = malloc(sizeof(object));
            vec2i pos = object_get_pos(har_obj);
            int hd = object_get_direction(har_obj);
            object_create(obj, scene->gs, pos, vec2f_create(0,0));
            player->har_id = HAR_JAGUAR + i;
            if (scene_load_har(scene, 0, player->har_id)) {
                return 1;
            }

            object_set_palette(obj, arena_get_player_palette(scene, 0), 0);

            if(har_create(obj, scene->af_data[0], hd, player->har_id, player->pilot_id, 0)) {
                return 1;
            }

#ifdef DEBUGMODE
            // Copy debugging setting over
            har *new_har = object_get_userdata(obj);
            har *old_har = object_get_userdata(har_obj);
            new_har->debug_enabled = old_har->debug_enabled;
#endif

            // Set HAR to controller and game_player
            game_state_add_object(scene->gs, obj, RENDER_LAYER_MIDDLE);

            game_state_del_object(scene->gs, har_obj);

            // Set HAR for player
            game_player_set_har(player, obj);
            game_player_get_ctrl(player)->har = obj;
 
            return 0;
        }
    }
    return 1;
}

int console_cmd_win(scene *scene, void *userdata, int argc, char **argv) {
    if(argc == 1) {
        game_player *player = game_state_get_player(scene->gs, 1);
        object *har_obj = game_player_get_har(player);
        har *har = object_get_userdata(har_obj);
        har->health = 0;
        return 0;
    }
    return 1;
}

int console_cmd_lose(scene *scene, void *userdata, int argc, char **argv) {
    if(argc == 1) {
        game_player *player = game_state_get_player(scene->gs, 0);
        object *har_obj = game_player_get_har(player);
        har *har = object_get_userdata(har_obj);
        har->health = 0;
        return 0;
    }
    return 1;
}

int console_cmd_stun(scene *scene, void *userdata, int argc, char **argv) {
    if(argc == 1) {
        game_player *player = game_state_get_player(scene->gs, 1);
        object *har_obj = game_player_get_har(player);
        har *har = object_get_userdata(har_obj);
        har->endurance = 0;
        har->state = STATE_RECOIL;
        return 0;
    }
    return 1;
}

#ifdef DEBUGMODE
int console_cd_debug(scene *scene, void *userdata, int argc, char **argv) {
    for(int i = 0; i < 2; i++) {
        har *har = object_get_userdata(game_state_get_player(scene->gs, i)->har);
        har->debug_enabled = !har->debug_enabled;
    }
    return 0;
}
#endif

/*int console_cmd_connect(scene *scene, void *userdata, int argc, char **argv) {
    ENetHost *client;
    ENetAddress address;
    ENetPeer *peer;
    ENetEvent event;
    if (argc == 2) {
        client = enet_host_create(NULL, 1, 2, 0, 0);
        if (client == NULL) {
            DEBUG("Failed to initialize ENet client");
            return 1;
        }
        enet_address_set_host(&address, argv[1]);
        address.port = 1337;

        peer = enet_host_connect(client, &address, 2, 0);

        if (peer == NULL) {
            DEBUG("Unable to connect to %s", argv[1]);
            return 1;
        }
        if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
            // send a fake ACT_STOP
            ENetPacket * packet = enet_packet_create("\n", 2,  ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send (peer, 0, packet);
            enet_host_flush(client);
            DEBUG("Connected to %s!", argv[1]);

            game_player *player1 = scene_get_game_player(scene, 0);
            game_player *player2 = scene_get_game_player(scene, 1);

            controller *player1_ctrl, *player2_ctrl;
            keyboard_keys *keys;

            player1->har_id = HAR_JAGUAR;
            player1->player_id = 0;
            player2->har_id = HAR_JAGUAR;
            player2->player_id = 0;

            player1_ctrl = malloc(sizeof(controller));
            controller_init(player1_ctrl);
            player1_ctrl->har = player1->har;
            player2_ctrl = malloc(sizeof(controller));
            controller_init(player2_ctrl);
            player2_ctrl->har = player2->har;

            // Player 1 controller -- Network
            net_controller_create(player1_ctrl, client, peer);
            scene_set_player1_ctrl(scene, player1_ctrl);

            // Player 2 controller -- Keyboard
            keys = malloc(sizeof(keyboard_keys));
            keys->up = SDL_SCANCODE_UP;
            keys->down = SDL_SCANCODE_DOWN;
            keys->left = SDL_SCANCODE_LEFT;
            keys->right = SDL_SCANCODE_RIGHT;
            keys->punch = SDL_SCANCODE_RETURN;
            keys->kick = SDL_SCANCODE_RSHIFT;
            keyboard_create(player2_ctrl, keys);
            scene_set_player2_ctrl(scene, player2_ctrl);

            scene_load_new_scene(scene, SCENE_ARENA0);
            return 0;
        } else {
            enet_peer_reset(peer);
            DEBUG("Connection to %s failed", argv[1]);
            return 1;
        }
    }
    return 1;
}

int console_cmd_listen(scene *scene, void *userdata, int argc, char **argv) {
    ENetAddress address;
    ENetHost *server;
    ENetEvent event;

    address.host = ENET_HOST_ANY;
    address.port = 1337;

    server = enet_host_create(&address, 1, 2, 0, 0);
    if (server == NULL) {
        DEBUG("failed to initialize ENET server");
        return 1;
    }

    if (enet_host_service(server, &event, 10000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        DEBUG("Client connected from %x:%u",  event.peer->address.host, event.peer->address.port);

        ENetPacket * packet = enet_packet_create("\n", 2,  ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send (event.peer, 0, packet);
        enet_host_flush(server);

        controller *player1_ctrl, *player2_ctrl;
        keyboard_keys *keys;

        scene->player1.player_id = 0;
        scene->player2.har_id = HAR_JAGUAR;
        scene->player2.player_id = 0;

        player1_ctrl = malloc(sizeof(controller));
        controller_init(player1_ctrl);
        player1_ctrl->har = scene->player1.har;
        player2_ctrl = malloc(sizeof(controller));
        controller_init(player2_ctrl);
        player2_ctrl->har = scene->player2.har;

        // Player 1 controller -- Keyboard
        keys = malloc(sizeof(keyboard_keys));
        keys->up = SDL_SCANCODE_UP;
        keys->down = SDL_SCANCODE_DOWN;
        keys->left = SDL_SCANCODE_LEFT;
        keys->right = SDL_SCANCODE_RIGHT;
        keys->punch = SDL_SCANCODE_RETURN;
        keys->kick = SDL_SCANCODE_RSHIFT;
        keyboard_create(player1_ctrl, keys);
        scene_set_player1_ctrl(scene, player1_ctrl);

        // Player 2 controller -- Network
        net_controller_create(player2_ctrl, server, event.peer);
        scene_set_player2_ctrl(scene, player2_ctrl);

        scene_load_new_scene(scene, SCENE_ARENA0);
        return 0;
    } else {
        DEBUG("No connections received");
        return 1;
    }
    return 1;
}*/

int make_argv(char *p, char **argv) {
    // split line into argv, warning: does not handle quoted strings
    int argc = 0;
    while(isspace(*p)) { ++p; }
    while(*p) {
        if(argv != NULL) { argv[argc] = p; }
        while(*p && !isspace(*p)) { ++p; }
        if(argv != NULL && *p) { *p++ = '\0'; }
        while(isspace(*p)) { ++p; }
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
    const color textcolor = color_create(121, 121, 121, 255);
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
            font_render_char(&font_small, c, x, y+con->ypos-100, textcolor);
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
    con->dir = 0;
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
    console_add_cmd("h",     &console_cmd_history,  "show command history");
    console_add_cmd("clear", &console_cmd_clear,  "clear the console");
    console_add_cmd("cls",   &console_cmd_clear,  "clear the console");
    console_add_cmd("quit",  &console_cmd_quit,  "quit the game");
    console_add_cmd("exit",  &console_cmd_quit,  "quit the game");
    console_add_cmd("help",  &console_cmd_help,  "show all commands");
    console_add_cmd("scene", &console_cmd_scene, "change scene. usage: scene 1, scene 2, etc");
    console_add_cmd("har",   &console_cmd_har,   "change har. usage: har 1, har 2, etc");
    console_add_cmd("win",   &console_cmd_win,   "Set the other player's health to 0");
    console_add_cmd("lose",   &console_cmd_lose,   "Set your health to 0");
    console_add_cmd("stun",   &console_cmd_stun,   "Stun the other player");
#ifdef DEBUGMODE
    console_add_cmd("cd-debug", &console_cd_debug, "toggle collision detection debugging");
#endif
    /*console_add_cmd("connect", &console_cmd_connect,   "connect to provided IP");*/
    /*console_add_cmd("listen", &console_cmd_listen,   "wait for a network connection, times out after 5 seconds");*/
    
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
        const unsigned char *state = SDL_GetKeyboardState(NULL);
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
        font_render(&font_small, con->input, 0 , con->ypos - 7, color_create(121, 121, 121, 255));
        //cursor
        font_render(&font_small, "", strlen(con->input)*font_small.w , con->ypos - 7, color_create(121 - t, 121 - t, 121 - t, 255));
        console_output_render();
    }
}

void console_tick() {
    if (con->isopen && con->ypos < 100) {
        con->ypos+=4;
        if(settings_get()->video.instant_console) { con->ypos = 100; }
    } else if (!con->isopen && con->ypos > 0) {
        con->ypos-=4;
        if(settings_get()->video.instant_console) { con->ypos = 0; }
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

void console_remove_cmd(const char *name) {
    hashmap_sdel(&con->cmds, name);
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
