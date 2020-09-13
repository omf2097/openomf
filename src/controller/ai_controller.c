#include <math.h>
#include "controller/ai_controller.h"
#include "game/objects/har.h"
#include "game/objects/scrap.h"
#include "game/objects/projectile.h"
#include "game/protos/intersect.h"
#include "game/protos/object_specializer.h"
#include "game/scenes/arena.h"
#include "game/game_state.h"
#include "game/utils/serial.h"
#include "resources/af_loader.h"
#include "resources/ids.h"
#include "resources/animation.h"
#include "controller/controller.h"
#include "utils/log.h"
#include "utils/vec.h"
#include "utils/random.h"

typedef struct move_stat_t {
    int max_hit_dist;
    int min_hit_dist;
    int value;
    int attempts;
    int consecutive;
    int last_dist;
} move_stat;

typedef struct ai_t {
    int har_event_hooked;
    int difficulty;
    int act_timer;
    int cur_act;
    int input_lag; // number of ticks to wait per input
    int input_lag_timer;

    // move stats
    af_move *selected_move;
    int move_str_pos;
    move_stat move_stats[70];
    int blocked;

    // all projectiles currently on screen (vector of projectile object*)
    vector active_projectiles;
} ai;


int char_to_act(int ch, int direction) {
    switch(ch) {
        case '8': return ACT_UP;
        case '2': return ACT_DOWN;
        case '6':
            if(direction == OBJECT_FACE_LEFT) {
                return ACT_LEFT;
            } else {
                return ACT_RIGHT;
            }
        case '4':
            if(direction == OBJECT_FACE_LEFT) {
                return ACT_RIGHT;
            } else {
                return ACT_LEFT;
            }
        case '7':
            if(direction == OBJECT_FACE_LEFT) {
                return ACT_UP|ACT_RIGHT;
            } else {
                return ACT_UP|ACT_LEFT;
            }
        case '9':
            if(direction == OBJECT_FACE_LEFT) {
                return ACT_UP|ACT_LEFT;
            } else {
                return ACT_UP|ACT_RIGHT;
            }
        case '1':
            if(direction == OBJECT_FACE_LEFT) {
                return ACT_DOWN|ACT_RIGHT;
            } else {
                return ACT_DOWN|ACT_LEFT;
            }
        case '3':
            if(direction == OBJECT_FACE_LEFT) {
                return ACT_DOWN|ACT_LEFT;
            } else {
                return ACT_DOWN|ACT_RIGHT;
            }
        case 'K': return ACT_KICK;
        case 'P': return ACT_PUNCH;
        case '5': return ACT_STOP;
        default:
            break;
    }
    return ACT_STOP;
}

int ai_har_event(controller *ctrl, har_event event) {
    ai *a = ctrl->data;
    move_stat *ms;

    switch(event.type) {
        case HAR_EVENT_LAND_HIT:
            ms = &a->move_stats[event.move->id];

            if (ms->max_hit_dist == -1 || ms->last_dist > ms->max_hit_dist){
                ms->max_hit_dist = ms->last_dist;
            }

            if (ms->min_hit_dist == -1 || ms->last_dist < ms->min_hit_dist){
                ms->min_hit_dist = ms->last_dist;
            }

            ms->value++;
            if(ms->value > 10) {
                ms->value = 10;
            }
            break;

        case HAR_EVENT_ENEMY_BLOCK:
            ms = &a->move_stats[event.move->id];
            if(!a->blocked) {
                a->blocked = 1;
                ms->value--;
            }
            break;

        case HAR_EVENT_ATTACK:
            a->selected_move = NULL;
            break;

        default:
            break;
    }

    return 0;
}


void ai_controller_free(controller *ctrl) {
    ai *a = ctrl->data;
    vector_free(&a->active_projectiles);
    free(a);
}

int is_special_move(af_move *move) {
    const char *move_string = str_c(&move->move_string);
    if (
            !strcmp("K", move_string) ||
            !strcmp("K1", move_string) ||
            !strcmp("K2", move_string) ||
            !strcmp("K3", move_string) ||
            !strcmp("K4", move_string) ||
            !strcmp("K6", move_string) ||
            !strcmp("P", move_string) ||
            !strcmp("P1", move_string) ||
            !strcmp("P2", move_string) ||
            !strcmp("P3", move_string) ||
            !strcmp("P4", move_string) ||
            !strcmp("P6", move_string)) {
        return 0;
    }
    return 1;
}

int is_valid_move(af_move *move, har *h) {
    // If category is any of these, and bot is not close, then
    // do not try to execute any of them. This attempts
    // to make the HARs close up instead of standing in place
    // wawing their hands towards each other. Not a perfect solution.
    switch(move->category) {
        case CAT_CLOSE:
        case CAT_LOW:
        case CAT_MEDIUM:
        case CAT_HIGH:
            // Only allow handwaving if close or jumping
            if(!h->close && h->state != STATE_JUMPING) {
                return 0;
            }
    }
    if(move->category == CAT_JUMPING && h->state != STATE_JUMPING) {
        // not jumping but trying to execute a jumping move
        return 0;
    }
    if(move->category != CAT_JUMPING && h->state == STATE_JUMPING) {
        // jumping but this move is not a jumping move
        return 0;
    }
    if(move->category == CAT_SCRAP && h->state != STATE_VICTORY) {
        return 0;
    }

    if(move->category == CAT_DESTRUCTION && h->state != STATE_SCRAP) {
        return 0;
    }
    // XXX check for chaining?

    int move_str_len = str_size(&move->move_string);
    char tmp;
    for(int i = 0; i < move_str_len; i++) {
        tmp = str_at(&move->move_string, i);
        if(!((tmp >= '1' && tmp <= '9') || tmp == 'K' || tmp == 'P')) {
            return 0;
        }
    }

    if((move->damage > 0
        || move->category == CAT_PROJECTILE
        || move->category == CAT_SCRAP
        || move->category == CAT_DESTRUCTION) && move_str_len > 0) 
    {
        return 1;
    }

    return 0;
}

int maybe(int difficulty) {
    // make chance of blocking exponentially better as the difficulty inreases
    int a = rand_int(49);
    int b = difficulty*difficulty;
    /*DEBUG("maybe %d, %d < %d : %s", difficulty, a, b, a < b ? "true" : "false");*/
    if(a < b) {
        return 1;
    }
    return 0;
}

// return 1 on block
int ai_block_har(controller *ctrl, ctrl_event **ev) {
    ai *a = ctrl->data;
    object *o = ctrl->har;
    har *h = object_get_userdata(o);
    object *o_enemy = game_state_get_player(o->gs, h->player_id == 1 ? 0 : 1)->har;
    har *h_enemy = object_get_userdata(o_enemy);

    // XXX TODO get maximum move distance from the animation object
    if(fabsf(o_enemy->pos.x - o->pos.x) < 100 && h_enemy->executing_move &&
       maybe(a->difficulty)) {
        if(har_is_crouching(h_enemy)) {
            a->cur_act = (o->direction == OBJECT_FACE_RIGHT ? ACT_DOWN|ACT_LEFT : ACT_DOWN|ACT_RIGHT);
            controller_cmd(ctrl, a->cur_act, ev);
        } else {
            a->cur_act = (o->direction == OBJECT_FACE_RIGHT ? ACT_LEFT : ACT_RIGHT);
            controller_cmd(ctrl, a->cur_act, ev);
        }
        return 1;
    }
    return 0;
}

int ai_block_projectile(controller *ctrl, ctrl_event **ev) {
    ai *a = ctrl->data;
    object *o = ctrl->har;

    iterator it;
    object **o_tmp;
    vector_iter_begin(&a->active_projectiles, &it);
    while((o_tmp = iter_next(&it)) != NULL) {
        object *o_prj = *o_tmp;
        if(projectile_get_owner(o_prj) == o)  {
            continue;
        }
        if(o_prj->cur_sprite && maybe(a->difficulty)) {
            vec2i pos_prj = vec2i_add(object_get_pos(o_prj), o_prj->cur_sprite->pos);
            vec2i size_prj = object_get_size(o_prj);
            if (object_get_direction(o_prj) == OBJECT_FACE_LEFT) {
                pos_prj.x = object_get_pos(o_prj).x + ((o_prj->cur_sprite->pos.x * -1) - size_prj.x);
            }
            if(fabsf(pos_prj.x - o->pos.x) < 120) {
                a->cur_act = (o->direction == OBJECT_FACE_RIGHT ? ACT_DOWN|ACT_LEFT : ACT_DOWN|ACT_RIGHT);
                controller_cmd(ctrl, a->cur_act, ev);
                return 1;
            }
        }

    }

    return 0;
}

int ai_controller_poll(controller *ctrl, ctrl_event **ev) {
    ai *a = ctrl->data;
    object *o = ctrl->har;
    if (!o) {
        return 1;
    }
    har *h = object_get_userdata(o);
    object *o_enemy = game_state_get_player(o->gs, h->player_id == 1 ? 0 : 1)->har;

    // Do not run AI while the game is paused
    if(game_state_is_paused(o->gs)) { return 0; }

    // Do not run AI while match is starting or ending
    // XXX this prevents the AI from doing scrap/destruction moves
    // XXX this could be fixed by providing a "scene changed" event
    if(is_arena(game_state_get_scene(o->gs)->id) &&
       arena_get_state(game_state_get_scene(o->gs)) != ARENA_STATE_FIGHTING) {

        // null out selected move to fix the "AI not moving problem"
        a->selected_move = NULL;
        return 0;
    }

    // Grab all projectiles on screen
    vector_clear(&a->active_projectiles);
    game_state_get_projectiles(o->gs, &a->active_projectiles);

    // Try to block har
    if(ai_block_har(ctrl, ev)) {
        return 0;
    }

    // Try to block projectiles
    if(ai_block_projectile(ctrl, ev)) {
        return 0;
    }

    if(a->selected_move) {
        // finish doing the selected move first
        if(a->input_lag_timer > 0) {
            a->input_lag_timer--;
        } else {
            a->move_str_pos--;
            if(a->move_str_pos <= 0) {
                a->move_str_pos = 0;
            }
            a->input_lag_timer = a->input_lag;
        }
        int ch = str_at(&a->selected_move->move_string, a->move_str_pos);
        controller_cmd(ctrl, char_to_act(ch, o->direction), ev);

    } else if(rand_int(100) < a->difficulty) {
        af_move *selected_move = NULL;
        int top_value = 0;

        // Attack
        for(int i = 0; i < 70; i++) {
            af_move *move = NULL;
            if((move = af_get_move(h->af_data, i))) {
                move_stat *ms = &a->move_stats[i];
                if(is_valid_move(move, h)) {
                    int value = ms->value + rand_int(10);
                    if (ms->min_hit_dist != -1){
                        if (ms->last_dist < ms->max_hit_dist+5 && ms->last_dist > ms->min_hit_dist+5){
                            value += 2;
                        } else if (ms->last_dist > ms->max_hit_dist+10){
                            value -= 3;
                        }
                    }

                    value -= ms->attempts/2;
                    value -= ms->consecutive*2;

                    if (is_special_move(move) && !maybe(a->difficulty)) {
                        DEBUG("skipping special move %s because of difficulty", str_c(&move->move_string));
                        continue;
                    }

                    if (selected_move == NULL){
                        selected_move = move;
                        top_value = value;
                    } else if (value > top_value) {
                        selected_move = move;
                        top_value = value;
                    }
                }
            }
        }
        for(int i = 0; i < 70; i++) {
            a->move_stats[i].consecutive /= 2;
        }
        if(selected_move) {
            a->move_stats[selected_move->id].attempts++;
            a->move_stats[selected_move->id].consecutive++;

            // do the move
            a->selected_move = selected_move;
            a->move_str_pos = str_size(&selected_move->move_string)-1;
            a->move_stats[a->selected_move->id].last_dist = fabsf(o->pos.x - o_enemy->pos.x);
            a->blocked = 0;
            DEBUG("AI selected move %s", str_c(&selected_move->move_string));
        }
    } else {
        // Change action after 30 ticks
        if(a->act_timer <= 0 && rand_int(100) > 88){
            int p = rand_int(100);
            if(p > 40){
                // walk forward
                a->cur_act = (o->direction == OBJECT_FACE_RIGHT ? ACT_RIGHT : ACT_LEFT);
            } else if(p > 20){
                // walk backward
                a->cur_act = (o->direction == OBJECT_FACE_RIGHT ? ACT_LEFT : ACT_RIGHT);
            } else if(p > 10){
                // do nothing
                a->cur_act = ACT_STOP;
            } else {
                // crouch and block
                a->cur_act = (o->direction == OBJECT_FACE_RIGHT ? ACT_DOWN|ACT_LEFT : ACT_DOWN|ACT_RIGHT);
            }

            a->act_timer = 30;
            controller_cmd(ctrl, a->cur_act, ev);
        } else {
            a->act_timer--;
        }

        // Jump once in a while
        if(rand_int(100) == 88){
            if(o->vel.x < 0) {
                controller_cmd(ctrl, ACT_UP|ACT_LEFT, ev);
            } else if(o->vel.x > 0) {
                controller_cmd(ctrl, ACT_UP|ACT_RIGHT, ev);
            } else {
                controller_cmd(ctrl, ACT_UP, ev);
            }
        }
    }
    return 0;
}

void ai_controller_create(controller *ctrl, int difficulty) {
    ai *a = malloc(sizeof(ai));
    a->har_event_hooked = 0;
    a->difficulty = difficulty+1;
    a->act_timer = 0;
    a->cur_act = 0;
    a->input_lag = 3;
    a->input_lag_timer = a->input_lag;
    a->selected_move = NULL;
    a->move_str_pos = 0;
    memset(a->move_stats, 0, sizeof(a->move_stats));
    for(int i = 0;i < 70;i++) {
        a->move_stats[i].max_hit_dist = -1;
        a->move_stats[i].min_hit_dist = -1;
        a->move_stats[i].last_dist = -1;
    }
    a->blocked = 0;
    vector_create(&a->active_projectiles, sizeof(object*));

    ctrl->data = a;
    ctrl->type = CTRL_TYPE_AI;
    ctrl->poll_fun = &ai_controller_poll;
    ctrl->har_hook = &ai_har_event;
}


