#include "game/har.h"
#include "utils/log.h"
#include "video/texture.h"
#include "video/video.h"
#include "audio/sound.h"
#include "utils/array.h"
#include "utils/list.h"
#include <stdlib.h>
#include <string.h>

// Internal functions
void har_add_ani_player(void *userdata, int id, int mx, int my);
void har_set_ani_finished(void *userdata, int id);

void har_add_ani_player(void *userdata, int id, int mx, int my) {
    har *har = userdata;
    animation *ani = array_get(&har->animations, id);
    if(ani != NULL) {
        animationplayer np;
        DEBUG("spawning %id at %d + %d +%d", id, ani->sdani->start_x, mx, har->phy.pos.x);
        np.x = ani->sdani->start_x + mx + har->phy.pos.x;
        np.y = ani->sdani->start_y + my + har->phy.pos.y;
        animationplayer_create(&np, id, ani);
        animationplayer_set_direction(&np, har->direction);
        np.userdata = userdata;
        np.add_player = har_add_ani_player;
        np.del_player = har_set_ani_finished;
        list_append(&har->child_players, &np, sizeof(animationplayer));
        animationplayer_run(&np);
        DEBUG("Create animation %d @ x,y = %d,%d", id, np.x, np.y);
        return;
    } 
}

void har_set_ani_finished(void *userdata, int id) {
    har *har = userdata;
    iterator it;
    animationplayer *tmp = 0;
    
    list_iter_begin(&har->child_players, &it);
    while((tmp = iter_next(&it)) != NULL) {
        if(tmp->id == id) {
            tmp->finished = 1;
            return;
        }
    }
}

void har_switch_animation(har *har, int id) {
    animationplayer_free(&har->player);
    animationplayer_create(&har->player, id, array_get(&har->animations, id));
    animationplayer_set_direction(&har->player, har->direction);
    animationplayer_run(&har->player);
    har->player.userdata = har;
    har->player.add_player = har_add_ani_player;
    har->player.del_player = har_set_ani_finished;
}

void phycb_fall(physics_state *state, void *userdata) {
    har *h = userdata;
    animationplayer_next_frame(&h->player);
    DEBUG("switching to falling");
}

void phycb_floor_hit(physics_state *state, void *userdata, int flight_mode) {
    har *h = userdata;
    h->player.finished = 1;
    h->state = STATE_STANDING;
    DEBUG("stopped jumping");
}

void phycb_stop(physics_state *state, void *userdata) {
    har *h = userdata;
    h->state = STATE_STANDING;
    har_switch_animation(h, 11);
    animationplayer_set_repeat(&h->player, 1);
    DEBUG("switching to idle");
}

void phycb_jump(physics_state *state, void *userdata) {
    har *h = userdata;
    h->state = STATE_JUMPING;
    har_switch_animation(h, 1);
    DEBUG("switching to jumping");
}

void phycb_move(physics_state *state, void *userdata) {
    har *h = userdata;
    h->state = STATE_WALKING;
    har_switch_animation(h, 10);
    animationplayer_set_repeat(&h->player, 1);
    DEBUG("switching to walking");
    // TODO: Handle reverse animation if necessary
}

void phycb_crouch(physics_state *state, void *userdata) {
    har *h = userdata;
    h->state = STATE_CROUCHING;
    har_switch_animation(h, 4);
    //animationplayer_set_repeat(&h->player, 1);
    DEBUG("crouching");
}

int har_load(har *h, sd_palette *pal, char *soundtable, int id, int x, int y, int direction) {
    // Physics & callbacks
    physics_init(&h->phy, x, y, 0.0f, 0.0f, 190, 10, 24, 295, 0.4f, h);
    h->phy.fall = phycb_fall;
    h->phy.floor_hit = phycb_floor_hit;
    h->phy.stop = phycb_stop;
    h->phy.jump = phycb_jump;
    h->phy.move = phycb_move;
    h->phy.crouch = phycb_crouch;
    
    // Initial bot stuff
    h->state = STATE_STANDING;
    h->direction = direction;
    h->tick = 0; // TEMPORARY
    h->af = sd_af_create();
    
    // fill the input buffer with 'pauses'
    memset(h->inputs, '5', 10);
    h->inputs[10] = '\0';
    
    // Load AF
    int ret;
    switch (id) {
        case HAR_JAGUAR:
            ret = sd_af_load(h->af, "resources/FIGHTR0.AF");
            break;
        case HAR_SHADOW:
            ret = sd_af_load(h->af, "resources/FIGHTR1.AF");
            break;
        case HAR_THORN:
            ret = sd_af_load(h->af, "resources/FIGHTR2.AF");
            break;
        case HAR_PYROS:
            ret = sd_af_load(h->af, "resources/FIGHTR3.AF");
            break;
        case HAR_ELECTRA:
            ret = sd_af_load(h->af, "resources/FIGHTR4.AF");
            break;
        case HAR_KATANA:
            ret = sd_af_load(h->af, "resources/FIGHTR5.AF");
            break;
        case HAR_SHREDDER:
            ret = sd_af_load(h->af, "resources/FIGHTR6.AF");
            break;
        case HAR_FLAIL:
            ret = sd_af_load(h->af, "resources/FIGHTR7.AF");
            break;
        case HAR_GARGOYLE:
            ret = sd_af_load(h->af, "resources/FIGHTR8.AF");
            break;
        case HAR_CHRONOS:
            ret = sd_af_load(h->af, "resources/FIGHTR9.af");
            break;
        case HAR_NOVA:
            ret = sd_af_load(h->af, "resources/FIGHTR10.AF");
            break;
        default:
            return 1;
            break;
    }

    if (ret) {
        return 1;
    }
    
    // Handle animations
    animation *ani;
    sd_move *move;
    array_create(&h->animations);
    for(unsigned int i = 0; i < 70; i++) {
        move = h->af->moves[i];
        if(move != NULL) {
            // Create animation + textures, etc.
            ani = malloc(sizeof(animation));
            animation_create(ani, move->animation, pal, -1, soundtable);
            array_set(&h->animations, i, ani);
            DEBUG("Loading animation %d", i);
        }
    }
    
    list_create(&h->child_players);

    // Start player with animation 11
    h->player.x = h->phy.pos.x;
    h->player.y = h->phy.pos.y;
    animationplayer_create(&h->player, 11, array_get(&h->animations, 11));
    animationplayer_set_direction(&h->player, h->direction);
    animationplayer_set_repeat(&h->player, 1);
    animationplayer_run(&h->player);
    h->player.userdata = h;
    h->player.add_player = har_add_ani_player;
    h->player.del_player = har_set_ani_finished;
    DEBUG("Har %d loaded!", id);
    return 0;
}

void har_free(har *h) {
    // Free AF
    sd_af_delete(h->af);
    
    // Free animations
    animation *ani = 0;
    iterator it;
    array_iter_begin(&h->animations, &it);
    while((ani = iter_next(&it)) != 0) {
        animation_free(ani);
        free(ani);
    }
    array_free(&h->animations);
    
    // Unload player
    animationplayer_free(&h->player);
}

void har_take_damage(har *har, int amount) {
    DEBUG("HAR took %f damage", amount / 2.0f);
    // we actually only want to play 3 frames from this animation, but lack the ability
    har_switch_animation(har, 9);
}

void har_collision_scene(har *har, scene *scene) {
}

void har_collision_har(har *har_a, har *har_b) {
    // Make stuff easier to get to :)
    int ani_id = har_a->player.id;
    int frame_id = animationplayer_get_frame(&har_a->player);
    sd_animation *ani = har_a->af->moves[ani_id]->animation;

    int other_ani_id = har_b->player.id;
    if (other_ani_id == 9) {
        // can't kick them while they're down
        return;
    }
    char other_frame_letter = animationplayer_get_frame_letter(&har_b->player);
    /*DEBUG("other frame letter is %d -> %c for frame %d", other_frame_letter, other_frame_letter, frame_id);*/
    /*sd_animation *other_ani = har_a->af->moves[ani_id]->animation;*/
    sd_sprite *sprite = har_b->af->moves[other_ani_id]->animation->sprites[(int)other_frame_letter - 65];
    int x = har_b->phy.pos.x + sprite->pos_x;
    int y = har_b->phy.pos.y + sprite->pos_y;
    int w = sprite->img->w;
    int h = sprite->img->h;

    // Find collision points, if any
    for(int i = 0; i < ani->col_coord_count; i++) {
        if(ani->col_coord_table[i].y_ext == frame_id) {
            DEBUG("%d vs %d-%d", (ani->col_coord_table[i].x * har_a->direction) + har_a->phy.pos.x, x, x+w);
            // coarse check vs sprite dimensions
            if ((ani->col_coord_table[i].x * har_a->direction) + har_a->phy.pos.x > x && (ani->col_coord_table[i].x * har_a->direction) +har_a->phy.pos.x < x + w) {
                DEBUG("x coordinate hit!");
                if (ani->col_coord_table[i].y + har_a->phy.pos.y > y && ani->col_coord_table[i].y + har_a->phy.pos.y < y + h) {
                    DEBUG("y coordinate hit!");
                    // TODO Do a fine grained per-pixel check for a hit
                    har_take_damage(har_b, har_a->af->moves[ani_id]->unknown[17]);
                }
            }
        }
    }

    if (har_a->state == STATE_WALKING && har_a->direction == -1) {
        // walking towards the enemy
        // 35 is a made up number that 'looks right'
        DEBUG("%d between %d, %d", har_a->phy.pos.x, har_b->phy.pos.x, har_b->phy.pos.x+35);
        if (har_a->phy.pos.x < har_b->phy.pos.x+35 && har_a->phy.pos.x > har_b->phy.pos.x) {
            har_a->phy.pos.x = har_b->phy.pos.x+35;
        }
    }
    if (har_a->state == STATE_WALKING && har_a->direction == 1) {
        // walking towards the enemy
        DEBUG("%d between %d, %d", har_a->phy.pos.x, har_b->phy.pos.x-35, har_b->phy.pos.x);
        if (har_a->phy.pos.x+35 > har_b->phy.pos.x && har_a->phy.pos.x < har_b->phy.pos.x) {
            har_a->phy.pos.x = har_b->phy.pos.x - 35;
        }
    }
}

void har_tick(har *har) {
    physics_tick(&har->phy);

    har->player.x = har->phy.pos.x;
    if(physics_is_in_air(&har->phy)) {
        har->player.y = har->phy.pos.y - 40;
    } else {
        har->player.y = har->phy.pos.y;
    }
    har->tick++;

    iterator it;
    animationplayer *tmp = 0;
    list_iter_begin(&har->child_players, &it);
    while((tmp = iter_next(&it)) != NULL) {
        animationplayer_run(tmp);
    }

    if(har->tick > 3) {
        animationplayer_run(&har->player);
        har->tick = 0;
    }

    if (har->player.id == 9 && animationplayer_get_frame(&har->player) > 4) {
        // bail out of the 'hit' animation early, because the hit animation
        // contains ALL the hit aniamtions one after the other
        har->player.finished = 1;
    }
    if(har->player.finished) {
        // 11 will never be finished, if it is set to repeat
        har->tick = 0;
        int animation = 11;
        switch(har->state) {
            case STATE_STANDING:
                animation = 11;
                break;
            case STATE_CROUCHING:
                animation = 4;
                break;
        }
        DEBUG("next animation is %d", animation);
        har_switch_animation(har, animation);
        animationplayer_set_repeat(&har->player, 1);
    }
}

void har_render(har *har) {
    iterator it;
    animationplayer *tmp = 0;
    list_iter_begin(&har->child_players, &it);
    while((tmp = iter_next(&it)) != NULL) {
        animationplayer_render(tmp);
    }

    animationplayer_render(&har->player);
}

void har_set_direction(har *har, int direction) {
    har->direction = direction;
    if (har->player.id != 11 && har->player.id != 4 && har->player.id != 10 && har->player.id != 1) {
        // don't change the direction yet, non-idle move in progress
        return;
    }
    animationplayer_set_direction(&har->player, har->direction);
}

void add_input(har *har, char c) {
    // only add it if it is not the current head of the array
    if (har->inputs[0] == c) {
        return;
    }

    // use memmove to move everything over one spot in the array, leaving the first slot free
    memmove((har->inputs)+1, har->inputs, 9);
    // write the new first element
    har->inputs[0] = c;
}

void har_act(har *har, int act_type) {
    if (har->player.id != 11 && har->player.id != 4 && har->player.id != 10 && har->player.id != 1) {
        // if we're not in the idle loop, bail for now
        return;
    }

    // for the reason behind the numbers, look at a numpad sometime
    switch(act_type) {
        case ACT_UP:
            add_input(har, '8');
            break;
        case ACT_DOWN:
            add_input(har, '2');
            break;
        case ACT_LEFT:
            if (har->direction == -1) {
                add_input(har, '6');
            } else {
                add_input(har, '4');
            }
            break;
        case ACT_RIGHT:
            if (har->direction == -1) {
                add_input(har, '4');
            } else {
                add_input(har, '6');
            }
            break;
        case ACT_UPRIGHT:
            if (har->direction == -1) {
                add_input(har, '7');
            } else {
                add_input(har, '9');
            }
            break;
        case ACT_UPLEFT:
            if (har->direction == -1) {
                add_input(har, '9');
            } else {
                add_input(har, '7');
            }
            break;
        case ACT_DOWNRIGHT:
            if (har->direction == -1) {
                add_input(har, '1');
            } else {
                add_input(har, '3');
            }
            break;
        case ACT_DOWNLEFT:
            if (har->direction == -1) {
                add_input(har, '3');
            } else {
                add_input(har, '1');
            }
            break;
        case ACT_KICK:
            add_input(har, 'K');
            break;
        case ACT_PUNCH:
            add_input(har, 'P');
            break;
        case ACT_STOP:
            physics_move(&har->phy, 0);
            add_input(har, '5');
            break;
        case ACT_WALKLEFT:
            physics_move(&har->phy, -1.0f);
            break;
        case ACT_WALKRIGHT:
            physics_move(&har->phy, 1.0f);
            break;
        case ACT_CROUCH:
            physics_crouch(&har->phy);
            break;
        case ACT_JUMP:
            physics_jump(&har->phy, -8.0f);
            break;
    }
    /*DEBUG("input buffer is now %s", har->inputs);*/

    // try to find a matching move
    sd_move *move;
    size_t len;
    for(unsigned int i = 0; i < 70; i++) {
        move = har->af->moves[i];
        if(move != NULL) {
            len = strlen(move->move_string);
            if (!strncmp(move->move_string, har->inputs, len)) {
                if ((har->state == STATE_JUMPING && move->unknown[13] == CAT_JUMPING) || har->state != STATE_JUMPING) {
                    DEBUG("matched move %d with string %s", i, move->move_string);
                    DEBUG("input was %s", har->inputs);

                    physics_move(&har->phy, 0);
                    har_switch_animation(har, i);
                    har->inputs[0]='\0';
                    break;
                }
            }
        }
    }
}
