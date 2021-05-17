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
#include "formats/pilot.h"
#include "resources/af_loader.h"
#include "resources/ids.h"
#include "resources/animation.h"
#include "controller/controller.h"
#include "utils/log.h"
#include "utils/vec.h"
#include "utils/random.h"

/* times thrown before we AI learns its lesson */
#define MAX_TIMES_THROWN 3
/* base likelihood to change movement action (lower is more likely) */
#define BASE_ACT_THRESH 90
/* base timer before we can consider changing movement action */
#define BASE_ACT_TIMER 28
/* base likelihood to keep moving (lower is more likely) */
#define BASE_MOVE_THRESH 16
/* base likelihood to move forwards (lower is more likely) */
#define BASE_FWD_THRESH 50
/* base likelihood to jump while moving forwards (lower is more likely) */
#define BASE_FWD_JUMP_THRESH 78
/* base likelihood to jump while moving backwards (lower is more likely) */
#define BASE_BACK_JUMP_THRESH 84
/* base likelihood to jump while standing still (lower is more likely) */
#define BASE_STILL_JUMP_THRESH 95

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
    int last_move_id;
    int move_str_pos;
    move_stat move_stats[70];
    int blocked;
    int thrown; // times thrown by enemy
    int queued_tactic;
    sd_pilot *pilot;

    // all projectiles currently on screen (vector of projectile object*)
    vector active_projectiles;
} ai;

enum {
    TACTIC_ESCAPE = 1, // escape from enemy
    TACTIC_TURTLE = 2, // block attacks
    TACTIC_GRAB = 3, // charge and grab enemy
    TACTIC_SPAM = 4, // spam the same attack
    TACTIC_SHOOT = 5, // shoot a projectile
    TACTIC_TRIP = 6, // trip enemy
    TACTIC_QUICK = 7, // quick attack
    TACTIC_CLOSE = 8, // close with the enemy
    TACTIC_FLY = 9 // fly towards the enemy
};

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

/** 
 * \brief Roll chance for pilot preference.
 *
 * \param pref_val The value of the pilot preference (-400 to 400)
 *
 * \return A boolean indicating whether the preference is confirmed.
 */
bool roll_pref(int pref_val) {
    int rand_roll = rand_int(800);
    int pref_thresh = pref_val + 400;
    return rand_roll <= pref_thresh;
}

/** 
 * \brief Determine whether the AI is smart enough to usually go ahead with an action.
 *
 * \param a The AI instance.
 *
 * \return A boolean indicating whether the AI is smart enough.
 */
bool smart_usually(ai *a) {
    int rand_roll = rand_int(32);
    int diff_factor = a->difficulty * a->difficulty; // 1 - 49
    return rand_roll <= diff_factor;
}

/** 
 * \brief Determine whether the AI is dumb enough to usually go ahead with an action.
 *
 * \param a The AI instance.
 *
 * \return A boolean indicating whether the AI is dumb enough.
 */
bool dumb_usually(ai *a) {
    return !smart_usually(a);
}

/** 
 * \brief Determine whether the AI is smart enough to sometimes go ahead with an action.
 *
 * \param a The AI instance.
 *
 * \return A boolean indicating whether the AI is smart enough.
 */
bool smart_sometimes(ai *a) {
    int rand_roll = rand_int(72);
    int diff_factor = a->difficulty * a->difficulty; // 1-49
    return rand_roll <= diff_factor;
}

/** 
 * \brief Determine whether the AI is dumb enough to sometimes go ahead with an action.
 *
 * \param a The AI instance.
 *
 * \return A boolean indicating whether the AI is dumb enough.
 */
bool dumb_sometimes(ai *a) {
    return !smart_sometimes(a);
}

/** 
 * \brief Convenience method to check whether the provided move is a special move.
 *
 * \param move The move instance.
 *
 * \return A boolean indicating whether the move is a special move.
 */
bool is_special_move(af_move *move) {
    if (
        !str_equal_c(&move->move_string, "K") ||
        !str_equal_c(&move->move_string, "K1") ||
        !str_equal_c(&move->move_string, "K2") ||
        !str_equal_c(&move->move_string, "K3") ||
        !str_equal_c(&move->move_string, "K4") ||
        !str_equal_c(&move->move_string, "K6") ||
        !str_equal_c(&move->move_string, "P") ||
        !str_equal_c(&move->move_string, "P1") ||
        !str_equal_c(&move->move_string, "P2") ||
        !str_equal_c(&move->move_string, "P3") ||
        !str_equal_c(&move->move_string, "P4") ||
        !str_equal_c(&move->move_string, "P6")
    ) {
        return false;
    }
    return true;
}

/** 
 * \brief Check whether a HAR has projectiles.
 *
 * \param har_id An integer identifying the HAR.
 *
 * \return A boolean indicating whether the HAS supports projectiles.
 */
bool har_has_projectiles(int har_id) {
    // disabled for now as AI is unable to use projectile attacks
    return false;

    switch (har_id) {
        case 0: // jaguar (spit)
        case 1: // shadow (shadow kick/punch)
        case 4: // electra (zap ball)
        case 5: // shredder (short-range hands)
        case 8: // chronos (chest triangle)
        case 9: // nova (missile)
            return true;
    }
    return false;
}

/** 
 * \brief Determine whether a pilot might dislike a move.
 *
 * \param a The AI instance.
 * \param selected_move The move instance.
 *
 * \return A boolean indicating whether move was disliked.
 */
bool move_disliked(ai *a, af_move *move) {
    // bail-out 75% of the time
    if (rand_int(4) > 1) {
        return false;
    }

    if (is_special_move(move)) {
        // decide whether to do special move
        return !roll_pref(a->pilot->ap_special);
    }

    switch(move->category) {
        case CAT_BASIC:
            // decide whether to do basic move
            return dumb_sometimes(a);
        case CAT_LOW:
            // decide whether to do low move
            return !roll_pref(a->pilot->ap_low);
        case CAT_MEDIUM:
            // decide whether to do middle move
            return !roll_pref(a->pilot->ap_middle);
        case CAT_HIGH:
            // decide whether to do high move
            return !roll_pref(a->pilot->ap_middle);
        case CAT_THROW:
        case CAT_CLOSE:
            // decide whether to do throw move
            return !roll_pref(a->pilot->ap_throw);
        case CAT_PROJECTILE:
        case CAT_SCRAP:
        case CAT_DESTRUCTION:
            // decide whether to do special move
            return !roll_pref(a->pilot->ap_special);
    }

    return false;
}

/** 
 * \brief Determine whether a move is too powerful for AI difficutly.
 *
 * \param a The AI instance.
 * \param selected_move The move instance.
 *
 * \return A boolean indicating whether move is considered too powerful.
 */
bool move_too_powerful(ai *a, af_move *move) {
    return is_special_move(move) && dumb_usually(a);
}

int ai_har_event(controller *ctrl, har_event event) {
    ai *a = ctrl->data;
    object *o = ctrl->har;
    har *h = object_get_userdata(o);
    sd_pilot *pilot = a->pilot;

    move_stat *ms;
    a->selected_move = NULL;

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

            if (smart_usually(a)) {
                if (!h->close) {
                    if (har_has_projectiles(h->id) && (pilot->att_def || pilot->att_sniper)) {
                        // DEBUG("== HIT EVENT == att_def/att_sniper - queue TACTIC_SHOOT");
                        a->queued_tactic = TACTIC_SHOOT;
                    } else if (pilot->att_hyper) {
                        // DEBUG("=== HIT EVENT === att_hyper - queue TACTIC_CLOSE");
                        a->queued_tactic = TACTIC_CLOSE;
                    }
                } else {
                    if (pilot->att_def) {
                        // DEBUG("=== HIT EVENT === att_def - queue TACTIC_TURTLE");
                        a->queued_tactic = TACTIC_TURTLE;
                    } else if (pilot->att_hyper) {
                        // DEBUG("=== HIT EVENT === att_hyper - queue TACTIC_GRAB");
                        a->queued_tactic = TACTIC_GRAB;
                    } else if (pilot->att_sniper) {
                        // DEBUG("=== HIT EVENT === att_sniper - queue TACTIC_QUICK");
                        a->queued_tactic = TACTIC_QUICK;
                    } else if (pilot->att_jump) {
                        // DEBUG("=== HIT EVENT === att_jump - queue TACTIC_ESCAPE");
                        a->queued_tactic = TACTIC_ESCAPE;
                    }
                }
            } else if (h->close && dumb_usually(a)) {
                // DEBUG("=== HIT EVENT === dumb - queue TACTIC_SPAM");
                a->last_move_id = event.move->id;
                a->queued_tactic = TACTIC_SPAM;
            }
            break;

        case HAR_EVENT_ENEMY_BLOCK:
            ms = &a->move_stats[event.move->id];
            if(!a->blocked) {
                a->blocked = 1;
                ms->value--;

                if (smart_usually(a)) {
                    if (h->close) {
                        if (pilot->att_def) {
                            // DEBUG("== ENEMY BLOCK EVENT == att_def - queue TACTIC_TURTLE");
                            a->queued_tactic = TACTIC_TURTLE;
                        } else if (pilot->att_hyper) {
                            if (event.move->category != CAT_LOW) {
                                // DEBUG("=== ENEMY BLOCK EVENT === att_hyper - queue TACTIC_TRIP");
                                a->queued_tactic = TACTIC_TRIP;
                            } else {
                                // DEBUG("=== ENEMY BLOCK EVENT === att_hyper - queue TACTIC_GRAB");
                                a->queued_tactic = TACTIC_GRAB;
                            }
                        } else if (pilot->att_sniper) {
                            // DEBUG("=== ENEMY BLOCK EVENT === att_sniper - queue TACTIC_QUICK");
                            a->queued_tactic = TACTIC_QUICK;
                        }
                    }
                } else if (h->close && dumb_usually(a)) {
                    // DEBUG("=== ENEMY BLOCK EVENT === dumb - queue TACTIC_SPAM");
                    a->last_move_id = event.move->id;
                    a->queued_tactic = TACTIC_SPAM;
                }
            }
            break;

        case HAR_EVENT_BLOCK:
            if (smart_usually(a)) {
                if (event.move->category == CAT_HIGH || event.move->category == CAT_JUMPING) {
                    // DEBUG("=== BLOCK EVENT === CAT_HIGH - queue TACTIC_TRIP");
                    a->queued_tactic = TACTIC_TRIP;
                } else if (pilot->att_def && event.move->category == CAT_LOW) {
                    // DEBUG("=== BLOCK EVENT === att_def - queue TACTIC_TURTLE");
                    a->queued_tactic = TACTIC_TURTLE;
                }
            }
            break;

        case HAR_EVENT_LAND:
            if (smart_sometimes(a)) {
                if (h->close) {
                    if (pilot->att_def) {
                        // DEBUG("== LAND EVENT == att_def - queue TACTIC_TURTLE");
                        a->queued_tactic = TACTIC_TURTLE;
                    } else if (pilot->att_hyper) {
                        // DEBUG("== LAND EVENT == att_hyper - queue TACTIC_GRAB");
                        a->queued_tactic = TACTIC_GRAB;
                    } else if (pilot->att_sniper) {
                        // DEBUG("== LAND EVENT == att_sniper - queue TACTIC_QUICK");
                        a->queued_tactic = TACTIC_QUICK;
                    } else {
                        // DEBUG("=== LAND EVENT === queue TACTIC_TRIP");
                        a->queued_tactic = TACTIC_TRIP;
                    }
                } else {
                    if (pilot->att_hyper) {
                        // DEBUG("== LAND EVENT == att_hyper - queue TACTIC_CLOSE");
                        a->queued_tactic = TACTIC_CLOSE;
                    } else if (pilot->att_sniper) {
                        if (har_has_projectiles(h->id)) {
                            // DEBUG("== LAND EVENT == att_sniper - queue TACTIC_SHOOT");
                            a->queued_tactic = TACTIC_SHOOT;
                        } else {
                            // DEBUG("== LAND EVENT == att_sniper - queue TACTIC_QUICK");
                            a->queued_tactic = TACTIC_QUICK;
                        }
                    }
                }
            }
            break;

        case HAR_EVENT_ATTACK:
            break;

        case HAR_EVENT_HIT_WALL:
            if (!h->close && smart_usually(a)) {
                if (har_has_projectiles(h->id)) {
                    // DEBUG("=== WALL EVENT === queue TACTIC_SHOOT");
                    a->queued_tactic = TACTIC_SHOOT;
                } else {
                    // DEBUG("=== WALL EVENT === queue TACTIC_FLY");
                    a->queued_tactic = TACTIC_FLY;
                }
            } else if (h->close && smart_sometimes(a)) {
                if (pilot->att_hyper) {
                    // DEBUG("=== WALL EVENT === att_hyper queue TACTIC_GRAB");
                    a->queued_tactic = TACTIC_GRAB;
                } else if (pilot->att_def) {
                    // DEBUG("=== WALL EVENT === att_def queue TACTIC_TURTLE");
                    a->queued_tactic = TACTIC_TURTLE;
                } else if (pilot->att_jump) {
                    // DEBUG("=== WALL EVENT === att_jump queue TACTIC_ESCAPE");
                    a->queued_tactic = TACTIC_ESCAPE;
                } else {
                    // DEBUG("=== WALL EVENT === queue TACTIC_TRIP");
                    a->queued_tactic = TACTIC_TRIP;
                }
            }
            break;

        case HAR_EVENT_TAKE_HIT:
            // if smart we usually respond to being hurt
            if (smart_usually(a)) {
                if (pilot->att_def) {
                    // DEBUG("=== HURT EVENT === att_def queue TACTIC_TURTLE");
                    a->queued_tactic = TACTIC_TURTLE;
                } else if (!h->close) {
                    if (smart_usually(a)) {
                        // DEBUG("=== HURT EVENT === !close queue TACTIC_FLY");
                        a->queued_tactic = TACTIC_FLY;
                    } else {
                        // DEBUG("=== HURT EVENT === !close queue TACTIC_CLOSE");
                        a->queued_tactic = TACTIC_CLOSE;
                    }
                } else if (h->is_wallhugging) {
                    if (pilot->att_hyper) {
                        // DEBUG("=== HURT EVENT === wallhug att_hyper queue TACTIC_GRAB");
                        a->queued_tactic = TACTIC_GRAB;
                    } else {
                        // DEBUG("=== HURT EVENT === wallhug queue TACTIC_FLY");
                        a->queued_tactic = TACTIC_FLY; 
                    }
                } else {
                    // DEBUG("=== HURT EVENT === queue TACTIC_TRIP");
                    a->queued_tactic = TACTIC_TRIP;
                }
            }

            if (event.move->category == CAT_THROW || event.move->category == CAT_CLOSE) {
                // keep track of how many times we have been thrown
                a->thrown++;
                // if thrown too often AI will learn to switch it up
                if (smart_usually(a) && a->thrown > MAX_TIMES_THROWN) {
                    if (pilot->att_def) {
                        // turn off defensive personality
                        pilot->att_def = 0;
                        // turn on aggressive personality
                        pilot->att_hyper = 1;
                    } else if (pilot->att_sniper) {
                        // turn off sniper personality
                        pilot->att_sniper = 0;
                        // turn on aggressive personality
                        pilot->att_hyper = 1;
                    }
                }
            }

            break;
        case HAR_EVENT_RECOVER:
            if (smart_usually(a)) {
                if (h->close && pilot->att_hyper) {
                    // DEBUG("=== RECOVER EVENT === att_hyper queue TACTIC_TRIP");
                    a->queued_tactic = TACTIC_TRIP;
                } else if (!h->close && pilot->att_def) {
                    // DEBUG("=== RECOVER EVENT === att_def queue TACTIC_TURTLE");
                    a->queued_tactic = TACTIC_TURTLE;
                }
            }
            break;
        case HAR_EVENT_ENEMY_STUN:
            // take advantage of stunned enemy
            if (smart_usually(a)) {
                if (h->close) {
                    // DEBUG("=== ADVANTAGE EVENT === queue TACTIC_GRAB");
                    a->queued_tactic = TACTIC_GRAB;
                } else {
                    // DEBUG("=== ADVANTAGE EVENT === queue TACTIC_CLOSE");
                    a->queued_tactic = TACTIC_CLOSE;
                }
            }
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

/** 
 * \brief Check whether a move is valid and can be initiated.
 *
 * \param move The move instance.
 * \param h The HAR instance.
 *
 * \return A boolean indicating whether the move is valid
 */
bool is_valid_move(af_move *move, har *h) {
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
        return false;
    }
    if(move->category != CAT_JUMPING && h->state == STATE_JUMPING) {
        // jumping but this move is not a jumping move
        return false;
    }
    if(move->category == CAT_SCRAP && h->state != STATE_VICTORY) {
        return false;
    }

    if(move->category == CAT_DESTRUCTION && h->state != STATE_SCRAP) {
        return false;
    }

    // XXX check for chaining?

    int move_str_len = str_size(&move->move_string);
    char tmp;
    for(int i = 0; i < move_str_len; i++) {
        tmp = str_at(&move->move_string, i);
        if(!((tmp >= '1' && tmp <= '9') || tmp == 'K' || tmp == 'P')) {
            return false;
        }
    }

    if((move->damage > 0
        || move->category == CAT_PROJECTILE
        || move->category == CAT_SCRAP
        || move->category == CAT_DESTRUCTION) && move_str_len > 0) 
    {
        return true;
    }

    return false;
}

/** 
 * \brief Sets the selected move.
 *
 * \param ctrl Controller instance.
 * \param selected_move The move instance.
 *
 * \return Void.
 */
void set_selected_move(controller *ctrl, af_move *selected_move) {
    ai *a = ctrl->data;
    object *o = ctrl->har;
    har *h = object_get_userdata(o);

    a->move_stats[selected_move->id].attempts++;
    a->move_stats[selected_move->id].consecutive++;

    // do the move
    a->selected_move = selected_move;
    a->move_str_pos = str_size(&selected_move->move_string)-1;
    object *o_enemy = game_state_get_player(o->gs, h->player_id == 1 ? 0 : 1)->har;
    a->move_stats[a->selected_move->id].last_dist = fabsf(o->pos.x - o_enemy->pos.x);
    a->blocked = 0;
    // DEBUG("AI selected move %s", str_c(&selected_move->move_string));
}

/** 
 * \brief Assigns a move by category identifier.
 *
 * \param ctrl Controller instance.
 * \param category An integer identifying the desired category of move.
 *
 * \return A boolean indicating whether move was assigned.
 */
bool assign_move_by_cat(controller *ctrl, int category) {
    ai *a = ctrl->data;
    object *o = ctrl->har;
    har *h = object_get_userdata(o);

    af_move *selected_move = NULL;
    int top_value = 0;
    for(int i = 0; i < 70; i++) {
        af_move *move = NULL;
        if((move = af_get_move(h->af_data, i))) {
            move_stat *ms = &a->move_stats[i];
            if(is_valid_move(move, h)) {
                // category filter
                if (category != move->category) {
                    continue;
                }

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
        set_selected_move(ctrl, selected_move);
        return true;
    }

    return false;
}

/** 
 * \brief Assigns a move by move_id.
 *
 * \param ctrl Controller instance.
 * \param move_id An integer identifying the desired move.
 *
 * \return A boolean indicating whether move was assigned.
 */
bool assign_move_by_id(controller *ctrl, int move_id) {
    object *o = ctrl->har;
    har *h = object_get_userdata(o);

    for(int i = 0; i < 70; i++) {
        af_move *move = NULL;
        if((move = af_get_move(h->af_data, i))) {
            if(is_valid_move(move, h)) {
                // move_id filter
                if (move_id != move->id) {
                    continue;
                }
                
                // DEBUG("=== assign_move_by_id === id %d", move_id);
                set_selected_move(ctrl, move);
                return true;
            }
        }
    }

    return false;
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
       smart_usually(a)) {
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
        if(o_prj->cur_sprite && smart_usually(a)) {
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

/** 
 * \brief Process the current selected move.
 *
 * \param ctrl Controller instance.
 * \param ev The current controller event.
 *
 * \return Void.
 */
void process_selected_move(controller *ctrl, ctrl_event **ev) {
    ai *a = ctrl->data;
    object *o = ctrl->har;

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
}

/** 
 * \brief Handle the AI's movement.
 *
 * \param ctrl Controller instance.
 * \param ev The current controller event.
 *
 * \return Void.
 */
void handle_movement(controller *ctrl, ctrl_event **ev) {
    ai *a = ctrl->data;
    object *o = ctrl->har;

    // Change action after act_timer runs out
    int jump_thresh = 0;
    if(a->act_timer <= 0 && rand_int(100) > (BASE_ACT_THRESH - (a->difficulty * 3))) {
        int p_move_roll = rand_int(100);

        int p_move_thresh = BASE_MOVE_THRESH - (a->difficulty * 2);

        if (p_move_roll > p_move_thresh) {

            int p_fwd_roll = rand_int(100);

            int p_fwd_thresh = BASE_FWD_THRESH - ((a->difficulty - 1) * 2);
            if (a->pilot->pref_fwd >= a->pilot->pref_back) p_fwd_thresh -= 4; // pilot pref
            if (a->pilot->att_hyper) p_fwd_thresh -= 4; // pilot hyper

            // DEBUG("p_fwd_thresh %d", p_fwd_thresh);
            
            if (p_fwd_roll >= p_fwd_thresh) {
                // walk forward
                a->cur_act = (o->direction == OBJECT_FACE_RIGHT ? ACT_RIGHT : ACT_LEFT);
                jump_thresh = BASE_FWD_JUMP_THRESH - (a->difficulty * 2);
            } else {
                // walk backward
                a->cur_act = (o->direction == OBJECT_FACE_RIGHT ? ACT_LEFT : ACT_RIGHT);
                jump_thresh = BASE_BACK_JUMP_THRESH - (a->difficulty * 2);
            }
        } else {
            if (smart_sometimes(a)) {
                // crouch and block
                a->cur_act = (o->direction == OBJECT_FACE_RIGHT ? ACT_DOWN|ACT_LEFT : ACT_DOWN|ACT_RIGHT);
            } else {
                // do nothing
                a->cur_act = ACT_STOP;
                jump_thresh = BASE_STILL_JUMP_THRESH - a->difficulty;
            }             
        }

        a->act_timer = BASE_ACT_TIMER - (a->difficulty * 2);

        controller_cmd(ctrl, a->cur_act, ev);
    } else {
        a->act_timer--;
    }

    // 5% more chance of jumping if pilot personality likes it
    if (jump_thresh > 0 && a->pilot->att_jump) {
        jump_thresh -= 5;
    }

    // Jump once in a while if they like to jump
    if(jump_thresh > 0 && rand_int(100) >= jump_thresh && roll_pref(a->pilot->pref_jump)){
        if(o->vel.x < 0) {
            controller_cmd(ctrl, ACT_UP|ACT_LEFT, ev);
        } else if(o->vel.x > 0) {
            controller_cmd(ctrl, ACT_UP|ACT_RIGHT, ev);
        } else {
            controller_cmd(ctrl, ACT_UP, ev);
        }
    }
}

/** 
 * \brief Attempt to select a random attack.
 *
 * \param ctrl Controller instance.
 *
 * \return Boolean indicating whether an attack was selected.
 */
bool attempt_attack(controller *ctrl) {
    ai *a = ctrl->data;
    object *o = ctrl->har;
    har *h = object_get_userdata(o);

    if(smart_usually(a)) {
        af_move *selected_move = NULL;
        int top_value = 0;

        // Attack
        for(int i = 0; i < 70; i++) {
            af_move *move = NULL;
            if((move = af_get_move(h->af_data, i))) {
                move_stat *ms = &a->move_stats[i];
                if(is_valid_move(move, h)) {

                    // sometimes skip move if pilot dislikes it
                    if (move_disliked(a, move)) {
                        // DEBUG("skipping move %s because of pilot preference", str_c(&move->move_string));
                        continue;
                    }

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

                    // sometimes skip move if it is too powerful for difficulty
                    if (move_too_powerful(a, move)) {
                        DEBUG("skipping move %s because of difficulty", str_c(&move->move_string));
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
            set_selected_move(ctrl, selected_move);
            return true;
        }
    }

    return false;
}

/** 
 * \brief Attempt to initiate the currently queued tactic.
 *
 * \param ctrl Controller instance.
 * \param ev The current controller event.
 *
 * \return Boolean indicating whether an tactic was initiated.
 */
bool attempt_tactic(controller *ctrl, ctrl_event **ev) {
    ai *a = ctrl->data;
    object *o = ctrl->har;
    har *h = object_get_userdata(o);

    switch (a->queued_tactic) {
        case TACTIC_TURTLE:
            if (rand_int(2) == 1 || h->is_wallhugging) {
                // DEBUG("=== attempt_tactic === TACTIC_TURTLE - crouch & block");
                // crouch and block
                a->cur_act = (o->direction == OBJECT_FACE_RIGHT ? ACT_DOWN|ACT_LEFT : ACT_DOWN|ACT_RIGHT);
            } else {
                // DEBUG("=== attempt_tactic === TACTIC_TURTLE - walk backward");
                // walking retreat
                a->cur_act = (o->direction == OBJECT_FACE_RIGHT ? ACT_LEFT : ACT_RIGHT)|ACT_UP;
            }
        break;
        case TACTIC_FLY:
            // DEBUG("=== attempt_tactic === TACTIC_FLY - jump forward");
            // jump forward
            a->cur_act = (o->direction == OBJECT_FACE_RIGHT ? ACT_RIGHT : ACT_LEFT)|ACT_UP;
        break;
        case TACTIC_ESCAPE:
            if (h->is_wallhugging) {
                // DEBUG("=== attempt_tactic === TACTIC_ESCAPE - jump forward");
                // jump forward
                a->cur_act = (o->direction == OBJECT_FACE_RIGHT ? ACT_RIGHT : ACT_LEFT)|ACT_UP;
            } else {
                // DEBUG("=== attempt_tactic === TACTIC_ESCAPE - jump backward");
                // jump backward
                a->cur_act = (o->direction == OBJECT_FACE_RIGHT ? ACT_LEFT : ACT_RIGHT)|ACT_UP;
            }
        break;
        case TACTIC_CLOSE:
            a->queued_tactic = 0;

            if (!h->close) {
                if (rand_int(4) == 1) {
                    // DEBUG("=== attempt_tactic === TACTIC_CLOSE - jump forward");
                    // jump forward
                    a->cur_act = (o->direction == OBJECT_FACE_RIGHT ? ACT_RIGHT : ACT_LEFT)|ACT_UP;
                } else {
                    // DEBUG("=== attempt_tactic === TACTIC_CLOSE - walk forward");
                    // walking forward
                    a->cur_act = o->direction == OBJECT_FACE_RIGHT ? ACT_RIGHT : ACT_LEFT;
                }

                // persist with trying to close
                a->queued_tactic = TACTIC_CLOSE;
            } else {
                // if we manage to close
                if (a->pilot->att_hyper && assign_move_by_cat(ctrl, CAT_THROW)) {
                    // DEBUG("=== attempt_tactic === TACTIC_CLOSE - throw");
                    return true;
                } else if (assign_move_by_cat(ctrl, CAT_BASIC)) {
                    // DEBUG("=== attempt_tactic === TACTIC_CLOSE - basic");
                    return true;
                }
            }
        break;
        case TACTIC_GRAB:
            a->queued_tactic = 0;
            if (assign_move_by_cat(ctrl, CAT_THROW)) {
                // DEBUG("=== attempt_tactic === TACTIC_GRAB - throw SUCCESS");
                return true;
            } else {
                if (!h->close) {
                    // DEBUG("=== attempt_tactic === TACTIC_GRAB - closing");
                    // walking forward
                    a->cur_act = o->direction == OBJECT_FACE_RIGHT ? ACT_RIGHT : ACT_LEFT;
                    // persist with trying to throw
                    a->queued_tactic = TACTIC_GRAB;
                } else {
                    // DEBUG("=== attempt_tactic === TACTIC_GRAB - failed");
                    return false;
                }
            }
        break;
        case TACTIC_QUICK:
            a->queued_tactic = 0;
            if (assign_move_by_cat(ctrl, CAT_BASIC)) {
                // DEBUG("=== attempt_tactic === TACTIC_QUICK - basic SUCCESS");
                return true;
            } else {
                if (!h->close) {
                    // DEBUG("=== attempt_tactic === TACTIC_QUICK - closing");
                    // walking forward
                    a->cur_act = o->direction == OBJECT_FACE_RIGHT ? ACT_RIGHT : ACT_LEFT;
                    // persist with trying to attack
                    a->queued_tactic = TACTIC_QUICK;
                } else {
                    // DEBUG("=== attempt_tactic === TACTIC_QUICK - failed");
                    return false;
                }
            }
        case TACTIC_SPAM:
            // TODO: make spam throw CAT_BASIC attacks
            a->queued_tactic = 0;
            if (a->last_move_id > 0 && assign_move_by_id(ctrl, a->last_move_id)) {
                // DEBUG("=== attempt_tactic === TACTIC_SPAM - repeat SUCCESS");
                a->last_move_id = 0;
                return true;
            } else {
                // DEBUG("=== attempt_tactic === TACTIC_SPAM - failed");
                return false;
            }
        break;
        case TACTIC_SHOOT:
            a->queued_tactic = 0;
            if (assign_move_by_cat(ctrl, CAT_PROJECTILE)) {
                // DEBUG("=== attempt_tactic === TACTIC_SHOOT - projectile SUCCESS");
                return true;
            } else {
                // DEBUG("=== attempt_tactic === TACTIC_SHOOT - failed");
                return false;
            }
        break;
        case TACTIC_TRIP:
            a->queued_tactic = 0;

            if (assign_move_by_cat(ctrl, CAT_LOW)) {
                // DEBUG("=== attempt_tactic === TACTIC_TRIP - trip SUCCESS");
                return true;
            } else {
                if (!h->close) {
                    // DEBUG("=== attempt_tactic === TACTIC_TRIP - closing");
                    // walking forward
                    a->cur_act = o->direction == OBJECT_FACE_RIGHT ? ACT_RIGHT : ACT_LEFT;
                    // persist with trying to throw
                    a->queued_tactic = TACTIC_TRIP;
                } else {
                    // DEBUG("=== attempt_tactic === TACTIC_TRIP - failed");
                    return false;
                }
            }
        break;
        default:
            a->queued_tactic = 0;
            return false;
        break;
    }

    // reset movement act timer
    a->act_timer = BASE_ACT_TIMER - (a->difficulty * 2);

    controller_cmd(ctrl, a->cur_act, ev);
    a->queued_tactic = 0;
    return true;
}

int ai_controller_poll(controller *ctrl, ctrl_event **ev) {
    ai *a = ctrl->data;
    object *o = ctrl->har;
    if (!o) {
        return 1;
    }

    har *h = object_get_userdata(o);

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

    // handle selected move
    if(a->selected_move) {
        // finish doing the selected move first
        process_selected_move(ctrl, ev);
        // DEBUG("=== POLL === process_selected_move");
        return 0;
    }

    // attempt queued tactic
    if (a->queued_tactic > 0 &&
        (
            h->state == STATE_STANDING ||
            h->state == STATE_WALKTO ||
            h->state == STATE_WALKFROM
        ) && 
        attempt_tactic(ctrl, ev)
    ) {
        // DEBUG("=== POLL === attempt_tactic passed");
        return 0;
    }

    // attempt an attack
    if (attempt_attack(ctrl)) {
        // DEBUG("=== POLL === attempt_attack passed");
        return 0;
    }

    // handle movement
    handle_movement(ctrl, ev);
    // DEBUG("=== POLL === handle_movement");

    return 0;
}

/** 
 * \brief Populate pilot preferences according to perceived personality.
 *
 * \param pilot The pilot details.
 * \param pilot_id An integer identifying the pilot.
 *
 * \return Void.
 */
void populate_pilot_prefs(sd_pilot *pilot, int pilot_id) {
    // not sure if this only exists when in tournament mode
    // but i wanted a way for arcade pilots to be behave uniquely
    switch (pilot_id) {
        case 0:
            // crystal
            // determined and independent
            pilot->pref_fwd = 50;
            pilot->att_sniper = 1;
            pilot->ap_throw = 250;
            pilot->ap_special = 50;
            break;
        case 1:
            // stefan
            // young and skillful
            pilot->att_normal = 1;
            pilot->ap_special = 200;
            pilot->ap_jump = 100;
            break;
        case 2:
            // milano
            // fast kickboxer
            pilot->att_jump = 1;
            pilot->pref_back = 50;
            pilot->ap_special = -150;
            pilot->ap_jump = 300;
            pilot->pref_jump = 100;
            break;
        case 3:
            // christian
            // aggressive
            pilot->att_hyper = 1;
            pilot->pref_fwd = 100;
            pilot->ap_special = 150;
            break;
        case 4:
            // shirro
            // slow but powerful
            pilot->att_normal = 1;
            pilot->ap_jump = -100;
            pilot->pref_jump = -100;
            pilot->ap_throw = 300;
            pilot->ap_special = -50;
            break;
        case 5:
            // jean-paul
            // well rounded & calculating
            pilot->att_normal = 1;
            pilot->pref_back = 50;
            pilot->ap_low = 100;
            pilot->ap_jump = 100;
            pilot->ap_special = 100;
            break;
        case 6:
            // ibrahim
            // patience
            pilot->att_def = 1;
            pilot->pref_back = 100;
            pilot->ap_special = 100;
            pilot->ap_throw = 100;
            break;
        case 7:
            // angel
            // mysterious
            pilot->att_sniper = 1;
            pilot->ap_special = 300;
            break;
        case 8:
            // cosette
            // defensive / cautious
            pilot->att_def = 1;
            pilot->ap_low = 100;
            pilot->ap_special = -50;
            pilot->pref_jump = -100;
            pilot->ap_jump = -50;
            break;
        case 9:
            // raven
            // that's so raven
            pilot->att_hyper = 1;
            pilot->pref_jump = 50;
            pilot->ap_jump = 100;
            pilot->ap_special = 300;
            break;
        case 10:
            // kreissack
            // special
            pilot->att_normal = 1;
            pilot->ap_throw = 100;
            pilot->ap_special = 350;
            break;
    }
}

void ai_controller_create(controller *ctrl, int difficulty, sd_pilot *pilot, int pilot_id) {
    ai *a = malloc(sizeof(ai));
    a->har_event_hooked = 0;
    a->difficulty = difficulty+1;
    a->act_timer = 0;
    a->cur_act = 0;
    a->input_lag = 3;
    a->input_lag_timer = a->input_lag;
    a->selected_move = NULL;
    a->last_move_id = 0;
    a->move_str_pos = 0;
    memset(a->move_stats, 0, sizeof(a->move_stats));
    for(int i = 0;i < 70;i++) {
        a->move_stats[i].max_hit_dist = -1;
        a->move_stats[i].min_hit_dist = -1;
        a->move_stats[i].last_dist = -1;
    }
    a->blocked = 0;
    a->queued_tactic = 0;
    vector_create(&a->active_projectiles, sizeof(object*));
    a->pilot = pilot;

    // pilot prefs are always zero outside of tournament mode
    // populating them here to make arcade mode more interesting
    populate_pilot_prefs(pilot, pilot_id);
    DEBUG("pilot %d", pilot_id);
    DEBUG("att_normal %d", pilot->att_normal);
    DEBUG("att_def %d", pilot->att_def);
    DEBUG("att_hyper %d", pilot->att_hyper);
    DEBUG("att_jump %d", pilot->att_jump);
    DEBUG("att_sniper %d", pilot->att_sniper);
    DEBUG("ap_throw %d", pilot->ap_throw);
    DEBUG("ap_special %d", pilot->ap_special);

    ctrl->data = a;
    ctrl->type = CTRL_TYPE_AI;
    ctrl->poll_fun = &ai_controller_poll;
    ctrl->har_hook = &ai_har_event;
}


