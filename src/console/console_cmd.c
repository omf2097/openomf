#include <stdio.h>
#include "game/scenes/arena.h"
#include "console/console.h"
#include "console/console_type.h"
#include "resources/ids.h"
#include "video/video.h"
#include "audio/music.h"
#include "utils/allocator.h"

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

int console_cmd_history(game_state *gs, int argc, char **argv) {
    iterator it;
    char *input;
    int input_len = sizeof(con->input);
    char buf[input_len];
    int i = 1;

    list_iter_begin(&con->history, &it);
    while((input = iter_next(&it)) != NULL) {
        snprintf(buf, input_len, "%d. %s", i, input);
        console_output_addline(buf);
        i++;
    }
    return 0;
}

int console_cmd_clear(game_state *gs, int argc, char **argv) {
    con->output[0] = '\0';
    con->output_head = 0;
    con->output_tail = 0;
    con->output_pos = 0;
    con->output_overflowing = 0;
    return 0;
}

int console_cmd_quit(game_state *gs, int argc, char **argv) {
    game_state_set_next(gs, SCENE_CREDITS);
    return 0;
}

int console_cmd_help(game_state *gs, int argc, char **argv) {
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

int console_cmd_scene(game_state *gs, int argc, char **argv) {
    // change scene
    if(argc == 2) {
        int i;
        if(strtoint(argv[1], &i) && is_scene(i)) {
            game_state_set_next(gs, i);
            return 0;
        }
    }
    return 1;
}

int console_cmd_har(game_state *gs, int argc, char **argv) {
    // change har
    if(argc == 2) {
        int i;
        if(strtoint(argv[1], &i)) {
            if(i < 0 || i > 10) {
                return 1;
            }

            game_player *player = game_state_get_player(gs, 0);
            if (scene_load_har(game_state_get_scene(gs), 0, player->har_id)) {
                return 1;
            }

            object *har_obj = game_player_get_har(player);
            vec2i pos = object_get_pos(har_obj);
            int hd = object_get_direction(har_obj);

            object *obj = omf_calloc(1, sizeof(object));
            object_create(obj, gs, pos, vec2f_create(0,0));
            player->har_id = i;

            if(har_create(obj, game_state_get_scene(gs)->af_data[0], hd, player->har_id, player->pilot_id, 0)) {
                object_free(obj);
                free(obj);
                return 1;
            }

            // Set HAR to controller and game_player
            game_state_add_object(gs, obj, RENDER_LAYER_MIDDLE, 0, 0);

            game_state_del_object(gs, har_obj);

            // Set HAR for player
            game_player_set_har(player, obj);
            game_player_get_ctrl(player)->har = obj;
            game_player_get_har(player)->animation_state.enemy = game_player_get_har(game_state_get_player(gs, 1));
            game_player_get_har(game_state_get_player(gs, 1))->animation_state.enemy = game_player_get_har(player);


            maybe_install_har_hooks(game_state_get_scene(gs));

            return 0;
        }
    }
    return 1;
}

int console_cmd_win(game_state *gs, int argc, char **argv) {
    if(argc == 1) {
        game_player *player = game_state_get_player(gs, 1);
        object *har_obj = game_player_get_har(player);
        har *har = object_get_userdata(har_obj);
        har->health = 0;
        return 0;
    }
    return 1;
}

int console_cmd_lose(game_state *gs, int argc, char **argv) {
    if(argc == 1) {
        game_player *player = game_state_get_player(gs, 0);
        object *har_obj = game_player_get_har(player);
        har *har = object_get_userdata(har_obj);
        har->health = 0;
        return 0;
    }
    return 1;
}

int console_cmd_stun(game_state *gs, int argc, char **argv) {
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

int console_cmd_rein(game_state *gs, int argc, char **argv) {
    scene *sc = game_state_get_scene(gs);
    if(is_arena(sc->id)) {
        arena_toggle_rein(sc);
        return 0;
    }
    return 1;
}

int console_cmd_renderer(game_state *gs, int argc, char **argv) {
    if(argc == 2) {
        int i;
        if(strtoint(argv[1], &i) && (i == 0 || i == 1)) {
            video_select_renderer(i);
            return 0;
        }
    }
    return 1;
}


int console_cmd_god(game_state *gs, int argc, char **argv) {
    for(int i = 0;i < game_state_num_players(gs);i++) {
        game_player *gp = game_state_get_player(gs, i);
        gp->god = !gp->god;
    }
    if(game_state_get_player(gs, 0)->god) {
        console_output_addline("God mode ON");
    } else {
        console_output_addline("God mode OFF");
    }

    return 0;
}

int console_kreissack(game_state *gs, int argc, char **argv) {
    game_player *p1 = game_state_get_player(gs, 0);
    p1->sp_wins = (2046 ^ (2 << p1->pilot_id));
    return 0;
}

int console_cmd_ez_destruct(game_state *gs, int argc, char **argv) {
    for(int i = 0;i < game_state_num_players(gs);i++) {
        game_player *gp = game_state_get_player(gs, i);
        gp->ez_destruct = !gp->ez_destruct;
    }
    if(game_state_get_player(gs, 0)->ez_destruct) {
        console_output_addline("EASY DESTRUCT ON");
    } else {
        console_output_addline("EASY DESTRUCT OFF");
    }

    return 0;
}

int console_cmd_music(game_state *gs, int argc, char **argv) {
    if(argc == 2) {
        int i;
        if(strtoint(argv[1], &i)) {
            music_play(PSM_END + i);
        }
    }
    return 0;
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
    console_add_cmd("music", &console_cmd_music, "Play specified song (0-6)");
    console_add_cmd("har",   &console_cmd_har,   "change har. usage: har 1, har 2, etc");
    console_add_cmd("win",   &console_cmd_win,   "Set the other player's health to 0");
    console_add_cmd("lose",  &console_cmd_lose,   "Set your health to 0");
    console_add_cmd("stun",  &console_cmd_stun,   "Stun the other player");
    console_add_cmd("rein",  &console_cmd_rein,   "R-E-I-N!");
    console_add_cmd("rdr",   &console_cmd_renderer, "Renderer (0=sw,1=hw)");
    console_add_cmd("god",   &console_cmd_god,  "Enable god mode");
    console_add_cmd("kreissack",   &console_kreissack,  "Fight Kreissack");
    console_add_cmd("ez-destruct",  &console_cmd_ez_destruct,  "Punch = destruction, kick = scrap");
}
