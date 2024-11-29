#include "audio/audio.h"
#include "console/console.h"
#include "console/console_type.h"
#include "game/scenes/arena.h"
#include "game/scenes/mechlab.h"
#include "resources/ids.h"
#include "utils/allocator.h"
#include <stdio.h>

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
    char buf[sizeof con->input];
    int i = 1;

    list_iter_begin(&con->history, &it);
    while((input = iter_next(&it)) != NULL) {
        snprintf(buf, sizeof buf, "%d. %s", i, input);
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

    vector_create(&sorted, sizeof(hashmap_pair *));
    hashmap_iter_begin(&con->cmds, &it);
    while((pair = iter_next(&it)) != NULL) {
        vector_append(&sorted, &pair);
    }
    vector_sort(&sorted, &sort_command_by_name);
    vector_iter_begin(&sorted, &it);
    while((ppair = iter_next(&it)) != NULL) {
        char *name = (*ppair)->key;
        command *cmd = (*ppair)->value;
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
        } else if((i = scene_get_id(argv[1])) > 0) {
            game_state_set_next(gs, i);
            return 0;
        }
    }
    return 1;
}

int console_toggle_warp(game_state *gs, int argc, char **argv) {
    gs->warp_speed = !gs->warp_speed;
    if(gs->warp_speed) {
        console_output_addline("Warp speed ON");
    } else {
        console_output_addline("Warp speed OFF");
    }
    return 0;
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
            player->pilot->har_id = i;
            if(gs->this_id >= SCENE_ARENA0 && gs->this_id <= SCENE_ARENA4) {
                if(scene_load_har(game_state_get_scene(gs), 0)) {
                    return 1;
                }

                object *har_obj = game_state_find_object(gs, game_player_get_har_obj_id(player));
                vec2i pos = object_get_pos(har_obj);
                int hd = object_get_direction(har_obj);

                object *obj = omf_calloc(1, sizeof(object));
                object_create(obj, gs, pos, vec2f_create(0, 0));
                // set the object to the same as the old one, so all the references remain intact
                obj->id = har_obj->id;

                if(har_create(obj, game_state_get_scene(gs)->af_data[0], hd, player->pilot->har_id,
                              player->pilot->pilot_id, 0)) {
                    object_free(obj);
                    omf_free(obj);
                    return 1;
                }

                // Set HAR to controller and game_player
                game_state_add_object(gs, obj, RENDER_LAYER_MIDDLE, 0, 0);

                game_state_del_object(gs, har_obj);

                // Set HAR for player
                game_player_set_har(player, obj);

                maybe_install_har_hooks(game_state_get_scene(gs));
            } else if(gs->this_id == SCENE_MECHLAB) {
                mechlab_update(gs->sc);
            }

            return 0;
        }
    }
    return 1;
}

int console_cmd_win(game_state *gs, int argc, char **argv) {
    if(argc == 1) {
        game_player *player = game_state_get_player(gs, 1);
        object *har_obj = game_state_find_object(gs, game_player_get_har_obj_id(player));
        if(!har_obj)
            return 1;
        har *har = object_get_userdata(har_obj);
        har->health = 0;
        return 0;
    }
    return 1;
}

int console_cmd_lose(game_state *gs, int argc, char **argv) {
    if(argc == 1) {
        game_player *player = game_state_get_player(gs, 0);
        object *har_obj = game_state_find_object(gs, game_player_get_har_obj_id(player));
        if(!har_obj)
            return 1;
        har *har = object_get_userdata(har_obj);
        har->health = 0;
        return 0;
    }
    return 1;
}

int console_cmd_stun(game_state *gs, int argc, char **argv) {
    if(argc == 1) {
        game_player *player = game_state_get_player(gs, 1);
        object *har_obj = game_state_find_object(gs, game_player_get_har_obj_id(player));
        if(!har_obj)
            return 1;
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

int console_cmd_god(game_state *gs, int argc, char **argv) {
    for(int i = 0; i < game_state_num_players(gs); i++) {
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
    p1->sp_wins = (2046 ^ (2 << p1->pilot->pilot_id));
    return 0;
}

int console_cmd_ez_destruct(game_state *gs, int argc, char **argv) {
    for(int i = 0; i < game_state_num_players(gs); i++) {
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
            audio_play_music(PSM_END + i);
        }
    }
    return 0;
}

int console_cmd_money(game_state *gs, int argc, char **argv) {
    // change pilot's money
    if(argc == 2) {
        int i;
        if(strtoint(argv[1], &i)) {

            game_player *player = game_state_get_player(gs, 0);
            player->pilot->money = i;
            if(gs->this_id == SCENE_MECHLAB) {
                mechlab_update(gs->sc);
            }
            return 0;
        }
    }
    return 1;
}

int console_cmd_rank(game_state *gs, int argc, char **argv) {
    // change tournament rank
    if(argc == 2) {
        int i;
        if(strtoint(argv[1], &i)) {
            game_player *player = game_state_get_player(gs, 0);
            if(i > 0 && i <= player->pilot->enemies_ex_unranked + 1) {
                player->pilot->rank = i;
                if(gs->this_id == SCENE_MECHLAB) {
                    mechlab_update(gs->sc);
                }
                return 0;
            }
        }
    }
    return 1;
}

void console_init_cmd(void) {
    // Add console commands
    console_add_cmd("h", &console_cmd_history, "show command history");
    console_add_cmd("clear", &console_cmd_clear, "clear the console");
    console_add_cmd("cls", &console_cmd_clear, "clear the console");
    console_add_cmd("quit", &console_cmd_quit, "quit the game");
    console_add_cmd("exit", &console_cmd_quit, "quit the game");
    console_add_cmd("help", &console_cmd_help, "show all commands");
    console_add_cmd("scene", &console_cmd_scene, "change scene. usage: scene 1, scene 2, etc");
    console_add_cmd("music", &console_cmd_music, "Play specified song (0-6)");
    console_add_cmd("har", &console_cmd_har, "change har. usage: har 1, har 2, etc");
    console_add_cmd("win", &console_cmd_win, "Set the other player's health to 0");
    console_add_cmd("lose", &console_cmd_lose, "Set your health to 0");
    console_add_cmd("stun", &console_cmd_stun, "Stun the other player");
    console_add_cmd("rein", &console_cmd_rein, "R-E-I-N!");
    console_add_cmd("god", &console_cmd_god, "Enable god mode");
    console_add_cmd("kreissack", &console_kreissack, "Fight Kreissack");
    console_add_cmd("ez-destruct", &console_cmd_ez_destruct, "Punch = destruction, kick = scrap");
    console_add_cmd("warp", &console_toggle_warp, "Toggle warp speed");
    console_add_cmd("money", &console_cmd_money, "Set tournament mode money");
    console_add_cmd("rank", &console_cmd_rank, "Set tournament mode rank");
}
