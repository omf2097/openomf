#include "audio/audio.h"
#include "console/console.h"
#include "console/console_type.h"
#include "formats/error.h"
#include "formats/rec_assertion.h"
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
    foreach(it, input) {
        snprintf(buf, sizeof buf, "%d. %s", i, input);
        console_output_addline(buf);
        i++;
    }
    return 0;
}

int insert_assertion(rec_assertion *op, game_state *gs) {
    uint8_t buf[8];
    if(!encode_assertion(op, buf)) {
        console_output_addline("failed to encode assertion\n");
        return 1;
    }

    sd_rec_move mv;
    memset(&mv, 0, sizeof(sd_rec_move));
    mv.lookup_id = 10;
    mv.raw_action = buf[0];
    mv.extra_data = malloc(7);
    mv.tick = gs->int_tick;
    memcpy(mv.extra_data, buf + 1, 7);
    if(sd_rec_insert_action_at_tick(gs->rec, &mv) != SD_SUCCESS) {
        console_output_addline("Inserting assertion failed.");
        return 1;
    } else {
        console_output_addline("Inserted assertion");
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
    foreach(it, pair) {
        vector_append(&sorted, &pair);
    }
    vector_sort(&sorted, &sort_command_by_name);
    vector_iter_begin(&sorted, &it);
    foreach(it, ppair) {
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
                object_create(obj, gs, pos, vec2f_createf(0, 0));
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

        rec_assertion op;
        op.operand1.is_literal = false;
        op.operand1.value.attr.har_id = 1;
        op.operand1.value.attr.attribute = ATTR_HEALTH;

        op.op = OP_SET;

        op.operand2.is_literal = true;
        op.operand2.value.literal = 1;

        // run the assertion so it applies
        if(!game_state_check_assertion_is_met(&op, gs)) {
            console_output_addline("Setting opponent health to 1 failed!");
            return 1;
        } else {
            return insert_assertion(&op, gs);
        }
    }
    return 1;
}

int console_cmd_lose(game_state *gs, int argc, char **argv) {
    if(argc == 1) {
        game_player *player = game_state_get_player(gs, 0);
        object *har_obj = game_state_find_object(gs, game_player_get_har_obj_id(player));
        if(!har_obj)
            return 1;

        rec_assertion op;
        op.operand1.is_literal = false;
        op.operand1.value.attr.har_id = 0;
        op.operand1.value.attr.attribute = ATTR_HEALTH;

        op.op = OP_SET;

        op.operand2.is_literal = true;
        op.operand2.value.literal = 1;

        // run the assertion so it applies
        if(!game_state_check_assertion_is_met(&op, gs)) {
            console_output_addline("Setting player health to 1 failed!");
            return 1;
        } else {
            return insert_assertion(&op, gs);
        }
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
    if(scene_is_arena(sc)) {
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
            int oldrank = player->pilot->rank;
            if(player->chr && oldrank != i && i > 0 && i <= player->pilot->enemies_ex_unranked + 1) {
                player->pilot->rank = i;
                // fix everyone else's rank
                for(int k = 0; k < player->chr->pilot.enemies_ex_unranked; k++) {
                    sd_pilot *p = &player->chr->enemies[k]->pilot;
                    // if newrank < oldrank everyone who was below the old rank and above the new rank goes up a rank
                    if(i < oldrank && p->rank < oldrank && p->rank >= i) {
                        p->rank++;
                    }
                    // if newrank > oldrank everyone who was above the oldrank and below the newrank goes down a rank
                    if(i > oldrank && p->rank > oldrank && p->rank <= i) {
                        p->rank--;
                    }
                }
                if(gs->this_id == SCENE_MECHLAB) {
                    mechlab_update(gs->sc);
                }
                return 0;
            }
        }
    }
    return 1;
}

int console_cmd_assert(game_state *gs, int argc, char **argv) {
    if(argc != 4) {
        console_output_addline("Usage: assert harX.attr OP value");
        return -1;
    }

    // check we have HARs
    game_player *player = game_state_get_player(gs, 0);
    object *har_obj = game_state_find_object(gs, game_player_get_har_obj_id(player));
    if(!har_obj) {
        return 1;
    }

    // Parse LHS: harX.attr
    char *lh_har_part = argv[1];
    char *dot_pos = strchr(lh_har_part, '.');
    if(dot_pos == NULL) {
        console_output_addline("Invalid LHS format. Expected harX.attr");
        return 1;
    }

    // Split into har and attribute parts
    *dot_pos = '\0'; // Terminate har part
    char *lh_attr_part = dot_pos + 1;

    rec_assertion op;

    int res = rec_assertion_get_operand(&op.operand1, lh_har_part, lh_attr_part);
    if(res == 1) {
        console_output_addline("Invalid LHS har identifier. Use har1 or har2.\n");
        return 1;
    } else if(res == 2) {
        console_output_addline("Invalid LHS har attribute");
        return 1;
    } else if(op.operand1.is_literal) {
        console_output_addline("Unexpected LHS literal.<something> use a bare integer literal.");
        return 1;
    }

    // Parse operator
    const char *oper = argv[2];

    if(strcmp(oper, ":=") == 0 || strcmp(oper, "set") == 0) {
        op.op = OP_SET;
    } else if(strcmp(oper, "==") == 0 || strcmp(oper, "eq") == 0) {
        op.op = OP_EQ;
    } else if(strcmp(oper, ">") == 0 || strcmp(oper, "gt") == 0) {
        op.op = OP_GT;
    } else if(strcmp(oper, "<") == 0 || strcmp(oper, "lt") == 0) {
        op.op = OP_LT;
    } else {
        console_output_addline("Invalid operator. Use ==, >, <, or := (or eq, gt, lt, or set).");
        return 1;
    }

    char *rh_har_part = argv[3];
    dot_pos = strchr(argv[3], '.');
    if(dot_pos != NULL) { // RHS is har.attr
        *dot_pos = '\0';  // Terminate har part
        char *rh_attr_part = dot_pos + 1;

        int res = rec_assertion_get_operand(&op.operand2, rh_har_part, rh_attr_part);
        if(res == 1) {
            console_output_addline("Invalid RHS har identifier. Use har1 or har2.\n");
            return 1;
        } else if(res == 2) {
            console_output_addline("Invalid RHS har attribute");
            return 1;
        } else if(op.operand2.is_literal) {
            console_output_addline("Unexpected RHS literal.<something> use a bare integer literal.");
            return 1;
        }

    } else { // RHS is integer
        char *endptr;
        long rhs_long = strtol(argv[3], &endptr, 10);
        if(endptr == argv[3] || *endptr != '\0') {
            console_output_addline("Invalid RHS integer literal.");
            return 1;
        }
        op.operand2.is_literal = true;
        op.operand2.value.literal = rhs_long;
    }

    print_assertion(&op);

    if(!game_state_check_assertion_is_met(&op, gs)) {
        console_output_addline("Assertion failed!");
        return 1;
    } else {
        return insert_assertion(&op, gs);
    }
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
    console_add_cmd("assert", &console_cmd_assert, "Insert an assertion into the current REC file");
}
