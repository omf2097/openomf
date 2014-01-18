#include <stdio.h>
#include "game/scenes/arena.h"
#include "console/console.h"
#include "console/console_type.h"
#include "resources/ids.h"

// utils
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
#ifdef DEBUGMODE
int console_cd_debug(game_state *gs, void *userdata, int argc, char **argv) {
    for(int i = 0; i < 2; i++) {
        har *har = object_get_userdata(game_state_get_player(gs, i)->har);
        har->debug_enabled = !har->debug_enabled;
    }
    return 0;
}
#endif

int console_cmd_history(game_state *gs, void *userdata, int argc, char **argv) {
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

int console_cmd_clear(game_state *gs, void *userdata, int argc, char **argv) {
    con->output[0] = '\0';
    con->output_head = 0;
    con->output_tail = 0;
    con->output_pos = 0;
    con->output_overflowing = 0;
    return 0;
}

int console_cmd_quit(game_state *gs, void *userdata, int argc, char **argv) {
    game_state_set_next(gs, SCENE_CREDITS);
    return 0;
}

int console_cmd_help(game_state *gs, void *userdata, int argc, char **argv) {
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

int console_cmd_scene(game_state *gs, void *userdata, int argc, char **argv) {
    // change scene
    if(argc == 2) {
        int i;
        if(strtoint(argv[1], &i)) {
            if(is_scene(i)) {
                game_state_set_next(gs, i);
                return 0;
            }
        }
    }
    return 1;
}

int console_cmd_har(game_state *gs, void *userdata, int argc, char **argv) {
    // change har
    if(argc == 2) {
        int i;
        if(strtoint(argv[1], &i)) {
            if(i < 0 || i > 10) {
                return 1;
            }

            game_player *player = game_state_get_player(gs, 0);

            object *har_obj = game_player_get_har(player);
            object *obj = malloc(sizeof(object));
            vec2i pos = object_get_pos(har_obj);
            int hd = object_get_direction(har_obj);
            object_create(obj, gs, pos, vec2f_create(0,0));
            player->har_id = HAR_JAGUAR + i;
            if (scene_load_har(game_state_get_scene(gs), 0, player->har_id)) {
                return 1;
            }

            if(har_create(obj, game_state_get_scene(gs)->af_data[0], hd, player->har_id, player->pilot_id, 0)) {
                return 1;
            }

#ifdef DEBUGMODE
            // Copy debugging setting over
            har *new_har = object_get_userdata(obj);
            har *old_har = object_get_userdata(har_obj);
            new_har->debug_enabled = old_har->debug_enabled;
#endif

            // Set HAR to controller and game_player
            game_state_add_object(gs, obj, RENDER_LAYER_MIDDLE);

            game_state_del_object(gs, har_obj);

            // Set HAR for player
            game_player_set_har(player, obj);
            game_player_get_ctrl(player)->har = obj;

            return 0;
        }
    }
    return 1;
}

int console_cmd_win(game_state *gs, void *userdata, int argc, char **argv) {
    if(argc == 1) {
        game_player *player = game_state_get_player(gs, 1);
        object *har_obj = game_player_get_har(player);
        har *har = object_get_userdata(har_obj);
        har->health = 0;
        return 0;
    }
    return 1;
}

int console_cmd_lose(game_state *gs, void *userdata, int argc, char **argv) {
    if(argc == 1) {
        game_player *player = game_state_get_player(gs, 0);
        object *har_obj = game_player_get_har(player);
        har *har = object_get_userdata(har_obj);
        har->health = 0;
        return 0;
    }
    return 1;
}

int console_cmd_stun(game_state *gs, void *userdata, int argc, char **argv) {
    if(argc == 1) {
        game_player *player = game_state_get_player(gs, 1);
        object *har_obj = game_player_get_har(player);
        har *har = object_get_userdata(har_obj);
        har->endurance = 0;
        har->state = STATE_RECOIL;
        return 0;
    }
    return 1;
}

int console_cmd_rein(game_state *gs, void *userdata, int argc, char **argv) {
    scene *sc = game_state_get_scene(gs);
    if(is_arena(sc->id)) {
        arena_toggle_rein(sc);
        return 0;
    }
    return 1;
}

void console_init_cmd() {
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
    console_add_cmd("rein",   &console_cmd_rein,   "R-E-I-N!");
#ifdef DEBUGMODE
    console_add_cmd("cd-debug", &console_cd_debug, "toggle collision detection debugging");
#endif
}
