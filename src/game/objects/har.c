#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "game/objects/har.h"
#include "game/objects/scrap.h"
#include "game/objects/projectile.h"
#include "game/protos/intersect.h"
#include "game/protos/object_specializer.h"
#include "game/scenes/arena.h"
#include "game/game_state.h"
#include "game/serial.h"
#include "resources/af_loader.h"
#include "resources/ids.h"
#include "resources/animation.h"
#include "controller/controller.h"
#include "utils/log.h"
#include "utils/random.h"

#define FUDGEFACTOR 0.003f
#define IS_ZERO(n) (n < 0.8 && n > -0.8)

void har_finished(object *obj);
int har_act(object *obj, int act_type);
void har_spawn_scrap(object *obj, vec2i pos, int amount);

void har_free(object *obj) {
    har *h = object_get_userdata(obj);
    list_free(&h->har_hooks);
    free(h);
}

/* hooks */

void fire_hooks(har *h, har_event event) {
    iterator it;
    har_hook *hook;

    list_iter_begin(&h->har_hooks, &it);
    while((hook = iter_next(&it)) != NULL) {
        hook->cb(event, hook->data);
    }
    controller *ctrl = game_player_get_ctrl(h->gp);
    if(object_get_userdata(ctrl->har) == h) {
        controller_har_hook(ctrl, event);
    }
}


void har_event_jump(har *h, int direction) {
    // direction is -1, 0 or 1, for backwards, up and forwards
    har_event event;
    event.type = HAR_EVENT_JUMP;
    event.player_id = h->player_id;
    event.direction = direction;

    fire_hooks(h, event);
}
void har_event_attack(har *h, af_move *move) {
    har_event event;
    event.type = HAR_EVENT_ATTACK;
    event.player_id = h->player_id;
    event.move = move;

    fire_hooks(h, event);
}

void har_event_enemy_block(har *h, af_move *move) {
    har_event event;
    event.type = HAR_EVENT_ENEMY_BLOCK;
    event.player_id = h->player_id;
    event.move = move;

    fire_hooks(h, event);
}


void har_event_take_hit(har *h, af_move *move) {
    har_event event;
    event.type = HAR_EVENT_TAKE_HIT;
    event.player_id = h->player_id;
    event.move = move;

    fire_hooks(h, event);
}

void har_event_land_hit(har *h, af_move *move) {
    har_event event;
    event.type = HAR_EVENT_LAND_HIT;
    event.player_id = h->player_id;
    event.move = move;

    fire_hooks(h, event);
}

void har_event_hazard_hit(har *h, bk_info *info) {
    har_event event;
    event.type = HAR_EVENT_HAZARD_HIT;
    event.player_id = h->player_id;
    event.info = info;

    fire_hooks(h, event);
}

void har_event_stun(har *h) {
    har_event event;
    event.type = HAR_EVENT_STUN;
    event.player_id = h->player_id;

    fire_hooks(h, event);
}

void har_event_recover(har *h) {
    har_event event;
    event.type = HAR_EVENT_RECOVER;
    event.player_id = h->player_id;

    fire_hooks(h, event);
}

void har_event_hit_wall(har *h, int wall) {
    har_event event;
    event.type = HAR_EVENT_HIT_WALL;
    event.player_id = h->player_id;
    event.wall = wall;

    fire_hooks(h, event);
}

void har_event_land(har *h) {
    har_event event;
    event.type = HAR_EVENT_LAND;
    event.player_id = h->player_id;

    fire_hooks(h, event);
}

void har_event_defeat(har *h) {
    har_event event;
    event.type = HAR_EVENT_DEFEAT;
    event.player_id = h->player_id;

    fire_hooks(h, event);
}

void har_event_scrap(har *h) {
    har_event event;
    event.type = HAR_EVENT_SCRAP;
    event.player_id = h->player_id;

    fire_hooks(h, event);
}

void har_event_destruction(har *h) {
    har_event event;
    event.type = HAR_EVENT_DESTRUCTION;
    event.player_id = h->player_id;

    fire_hooks(h, event);
}

void har_event_done(har *h) {
    har_event event;
    event.type = HAR_EVENT_DONE;
    event.player_id = h->player_id;

    fire_hooks(h, event);
}

void har_stunned_done(object *har_obj) {
    har *h = object_get_userdata(har_obj);

    if (h->state == STATE_STUNNED) {
        // refill endurance
        h->endurance = h->endurance_max;
        h->state = STATE_STANDING;
        har_set_ani(har_obj, ANIM_IDLE, 1);
    }
}

void har_action_hook(object *obj, int action) {
    har *h = object_get_userdata(obj);
    if (h->action_hook_cb) {
        h->action_hook_cb(action, h->action_hook_cb_data);
    }
    int pos = obj->age % OBJECT_EVENT_BUFFER_SIZE;
    h->act_buf[pos].actions[h->act_buf[pos].count] = (unsigned char)action;
    h->act_buf[pos].count++;
    h->act_buf[pos].age = obj->age;;
}

// Simple helper function
void har_set_ani(object *obj, int animation_id, int repeat) {
    har *h = object_get_userdata(obj);
    af_move *move = af_get_move(h->af_data, animation_id);
    char *s = (char*)str_c(&move->move_string);
    object_set_animation(obj, &move->ani);
    if (s != NULL && strcasecmp(s, "!") && strcasecmp(s, "0") && h->delay > 0) {
        DEBUG("delaying move %d %s by %d ticks", move->id, s, h->delay);
        object_set_delay(obj, h->delay);
    }

    if (move->category == CAT_JUMPING) {
        h->state = STATE_JUMPING;
        object_set_gravity(obj, h->af_data->fall_speed * FUDGEFACTOR);
    }
    object_set_repeat(obj, repeat);
    object_set_stride(obj, 1);
    object_tick(obj);
    // update this so mx/my have correct origins
    obj->start = obj->pos;
    h->damage_done = 0;
    h->damage_received = 0;
    h->executing_move = 0;
    h->flinching = 0;
}

int har_is_active(object *obj) {
    har *h = object_get_userdata(obj);
    // during scrap/destruction, the defeated har should be rendered frontmost
    if (h->state == STATE_DEFEAT) {
        return 1;
    }
    if (h->state == STATE_SCRAP || h->state == STATE_DESTRUCTION || h->state == STATE_VICTORY) {
        return 0;
    }
    return h->executing_move;
}

int har_is_walking(har *h) {
    if (h->state == STATE_WALKTO || h->state == STATE_WALKFROM) {
        return 1;
    }
    return 0;
}

int har_is_crouching(har *h) {
    if (h->state == STATE_CROUCHING || h->state == STATE_CROUCHBLOCK) {
        return 1;
    }
    return 0;
}

int har_is_blocking(har *h, af_move *move) {
    if (move->category == CAT_CLOSE) {
        // throws cannot be blocked
        return 0;
    }
    if (h->state == STATE_CROUCHBLOCK && move->category != CAT_JUMPING && h->executing_move == 0) {
        return 1;
    }
    if (h->state == STATE_WALKFROM && move->category != CAT_LOW && h->executing_move == 0) {
        return 1;
    }
    return 0;
}

int frame_isset(sd_stringparser_frame *frame, const char *tag) {
    const sd_stringparser_tag_value *v;
    sd_stringparser_get_tag(frame->parser, frame->id, tag, &v);
    return v->is_set;
}

int frame_get(sd_stringparser_frame *frame, const char *tag) {
    const sd_stringparser_tag_value *v;
    sd_stringparser_get_tag(frame->parser, frame->id, tag, &v);
    return v->value;
}

int har_is_invincible(object *obj, af_move *move) {
    sd_stringparser_frame f = obj->animation_state.parser->current_frame;
    if (frame_isset(&f, "zz")) {
        // blocks everything
        return 1;
    }
    switch (move->category) {
        // XX 'zg' is not handled here, but the game doesn't use it...
        case CAT_LOW:
            if (frame_isset(&f, "zl")) {
                return 1;
            }
            break;
        case CAT_MEDIUM:
            if (frame_isset(&f, "zm")) {
                return 1;
            }
            break;
        case CAT_HIGH:
            if (frame_isset(&f, "zh")) {
                return 1;
            }
            break;
        case CAT_JUMPING:
            if (frame_isset(&f, "zj")) {
                return 1;
            }
            break;
        case CAT_PROJECTILE:
            if (frame_isset(&f, "zp")) {
                return 1;
            }
            break;
    }
    return 0;
}

// Callback for spawning new objects, eg. projectiles
void cb_har_spawn_object(object *parent, int id, vec2i pos, int g, void *userdata) {
    har *h = userdata;
    vec2i p_pos = object_get_pos(parent);

    // can't do this in object, because it hoses the intro
    if(pos.x == 0) {
        pos.x = p_pos.x;// + p_size.x / 2;
    }
    if(pos.y == 0) {
        pos.y = p_pos.y;//y + p_size.y / 2;
    }


    // If this is a scrap item, handle it as such ...
    if(id == ANIM_SCRAP_METAL || id == ANIM_BOLT || id == ANIM_SCREW || id == ANIM_BURNING_OIL) {
        // Use our existing function for spawning scrap
        har_spawn_scrap(parent, pos, 12);
        return;
    }

    // ... otherwise expect it is a projectile
    af_move *move = af_get_move(h->af_data, id);
    if(move != NULL) {
        object *obj = malloc(sizeof(object));
        object_create(obj, parent->gs, pos, vec2f_create(0,0));
        object_set_userdata(obj, h);
        object_set_stl(obj, object_get_stl(parent));
        object_set_animation(obj, &move->ani);
        object_set_gravity(obj, g/50);
        object_set_pal_offset(obj, object_get_pal_offset(parent));
        // Set all projectiles to their own layer + har layer
        object_set_layers(obj, LAYER_PROJECTILE|(h->player_id == 0 ? LAYER_HAR2 : LAYER_HAR1)); 
        // To avoid projectile-to-projectile collisions, set them to same group
        object_set_group(obj, GROUP_PROJECTILE); 
        object_set_repeat(obj, 0);
        object_set_shadow(obj, 1);
        object_set_direction(obj, object_get_direction(parent));
        obj->animation_state.enemy = parent->animation_state.enemy;
        projectile_create(obj);
        game_state_add_object(parent->gs, obj, RENDER_LAYER_MIDDLE);
    }
}

void har_move(object *obj) {
    vec2f vel = object_get_vel(obj);
    obj->pos.x += vel.x;
    obj->pos.y += vel.y;
    har *h = object_get_userdata(obj);
    if(obj->pos.y > 190) {
        if (h->state != STATE_FALLEN) {
            // We collided with ground, so set vertical velocity to 0 and
            // make sure object is level with ground
            obj->pos.y = 190;
            object_set_vel(obj, vec2f_create(vel.x, 0));
        }

        // Change animation from jump to walk or idle,
        // depending on horizontal velocity
        object_set_gravity(obj, 1);
        if(h->state == STATE_JUMPING) {
            /*if(object_get_hstate(obj) == OBJECT_MOVING) {*/
                /*h->state = STATE_WALKING;*/
                /*har_set_ani(obj, ANIM_WALKING, 1);*/
            /*} else {*/
                h->state = STATE_STANDING;
                har_set_ani(obj, ANIM_IDLE, 1);
                har_action_hook(obj, ACT_STOP);
                har_action_hook(obj, ACT_FLUSH);
                har_event_land(h);
            /*}*/
        } else if (h->state == STATE_FALLEN || h->state == STATE_RECOIL) {
            float dampen = 0.4;
            vec2f vel = object_get_vel(obj);
            vec2i pos = object_get_pos(obj);
            if(pos.y > 190) {
                // TODO spawn clouds of dust
                pos.y = 190;
                vel.y = -vel.y * dampen;
                vel.x = vel.x * dampen;
            }

            if (pos.x <= 15 || pos.x >= 305) {
                vel.x = 0.0;
            }

            object_set_pos(obj, pos);
            object_set_vel(obj, vel);

            // prevent har from sliding after defeat
            if(h->state != STATE_DEFEAT &&
               h->health <= 0 && h->endurance <= 0 &&
               obj->animation_state.parser->current_frame.is_final_frame) {
                h->state = STATE_DEFEAT;
                har_set_ani(obj, ANIM_DEFEAT, 0);
                har_event_defeat(h);
            } else if(pos.y >= 185 &&
                      IS_ZERO(vel.x) &&
                      obj->animation_state.parser->current_frame.is_final_frame) {
                if (h->state == STATE_FALLEN) {
                    h->state = STATE_STANDING_UP;
                    har_set_ani(obj, ANIM_STANDUP, 0);
                    har_event_land(h);
                } else {
                    har_finished(obj);
                }
            }
        }
    } else {
        object_set_vel(obj, vec2f_create(vel.x, vel.y + obj->gravity));
    }
}

void har_take_damage(object *obj, str* string, float damage) {
    har *h = object_get_userdata(obj);
    int oldhealth = h->health;
    if(!game_state_get_player(obj->gs, h->player_id)->god) {
        h->health -= damage;
    }
    if(h->health <= 0) { h->health = 0; }

    if (oldhealth <= 0) {
        // har has no health left and is left only with endurance.
        // one hit will end them
        h->endurance = 0;
    } else {
        h->endurance -= damage * 8;
        if(h->endurance <= 0) {
            if (h->state == STATE_STUNNED) {
                // refill endurance
                h->endurance = h->endurance_max;
            } else {
                h->endurance = 0;
            }
        }
    }

    // chronos' stasis does not have a hit animation
    if (string->data) {
        sd_stringparser_frame f;
        const sd_stringparser_tag_value *v;
        h->state = STATE_RECOIL;
        // Set hit animation
        object_set_animation(obj, &af_get_move(h->af_data, ANIM_DAMAGE)->ani);
        object_set_repeat(obj, 0);
        if (h->health <= 0 && h->endurance <= 0) {
            // taken from MASTER.DAT
            // XXX changed the last frame to 200 ticks to ensure the HAR falls down
            char *final = "-x-20ox-20L1-ox-20L2-x-20zzs4l25sp13M1-zzM200";
            char *str = malloc(str_size(string) + strlen(final) + 1);
            // append the 'final knockback' string to the hit string, replacing the final frame
            sprintf(str, "%s", string->data);
            char *last = strrchr(str, '-');
            sprintf(last, "%s", final);
            object_set_custom_string(obj, str);
            free(str);
        } else {
            object_set_custom_string(obj, str_c(string));
        }
        object_tick(obj);
        h->flinching = 1;
        // XXX hack - if the first frame has the 'k' tag, treat it as some vertical knockback
        // we can't do this in player.c because it breaks the jaguar leap, which also uses the 'k' tag.
        sd_stringparser_peek(obj->animation_state.parser, 0, &f);
        sd_stringparser_get_tag(f.parser, f.id, "k", &v);
        if (v->is_set) {
                obj->vel.y -= 7;
        }
    }
}
void har_spawn_oil(object *obj, vec2i pos, int amount, float gravity, int layer) {
    float rv = 0.0f;
    float velx, vely;
    har *h = object_get_userdata(obj);

    // burning oil
    for(int i = 0; i < amount; i++) {
        // Calculate velocity etc.
        rv = rand_int(100) / 100.0f - 0.5;
        velx = (5 * cos(90 + i-(amount) / 2 + rv)) * object_get_direction(obj);
        vely = -12 * sin(i / amount + rv);

        // Make sure scrap has somekind of velocity
        // (to prevent floating scrap objects)
        if(vely < 0.1 && vely > -0.1) vely += 0.21;

        // Create the object
        object *scrap = malloc(sizeof(object));
        int anim_no = ANIM_BURNING_OIL;
        object_create(scrap, obj->gs, pos, vec2f_create(velx, vely));
        object_set_animation(scrap, &af_get_move(h->af_data, anim_no)->ani);
        object_set_stl(scrap, object_get_stl(obj));
        object_set_gravity(scrap, gravity);
        object_set_layers(scrap, LAYER_SCRAP);
        object_tick(scrap);
        scrap_create(scrap);
        game_state_add_object(obj->gs, scrap, layer);
    }

}

// TODO: This is kind of a hack. It's used to check if either 
// HAR is doing destruction. If there is any way to do this better,
// this should be changed.
int is_destruction(game_state *gs) {
    har *har_a = object_get_userdata(game_state_get_player(gs, 0)->har);
    har *har_b = object_get_userdata(game_state_get_player(gs, 1)->har);
    return (har_a->state == STATE_DESTRUCTION || har_b->state == STATE_DESTRUCTION);
}

void har_spawn_scrap(object *obj, vec2i pos, int amount) {
    float rv = 0.0f;
    float velx, vely;
    // wild ass guess
    int oil_amount = amount / 3;
    har *h = object_get_userdata(obj);
    har_spawn_oil(obj, pos, oil_amount, 1, RENDER_LAYER_TOP);

    // scrap metal
    // TODO this assumes the default scrap level and does not consider BIG[1-9]
    int scrap_amount = 0;
    int destr = is_destruction(obj->gs);
    if(destr) {
        scrap_amount = 30;
    } else if (amount > 11 && amount < 14) {
        scrap_amount = 1;
    } else if (amount > 13 && amount < 16) {
        scrap_amount = 2;
    } else if (amount > 15) {
        scrap_amount = 3;
    }
    for(int i = 0; i < scrap_amount; i++) {
        // Calculate velocity etc.
        rv = rand_int(100) / 100.0f - 0.5;
        velx = (5 * cos(90 + i-(scrap_amount) / 2 + rv)) * object_get_direction(obj);
        vely = -12 * sin(i / scrap_amount + rv);

        // Make destruction moves look more impressive :P
        if(destr) {
            velx *= 5;
            vely *= 5;
        }

        // Make sure scrap has somekind of velocity
        // (to prevent floating scrap objects)
        if(vely < 0.1 && vely > -0.1) vely += 0.21;

        // Create the object
        object *scrap = malloc(sizeof(object));
        int anim_no = rand_int(3) + ANIM_SCRAP_METAL;
        object_create(scrap, obj->gs, pos, vec2f_create(velx, vely));
        object_set_animation(scrap, &af_get_move(h->af_data, anim_no)->ani);
        object_set_stl(scrap, object_get_stl(obj));
        object_set_gravity(scrap, 1);
        object_set_pal_offset(scrap, object_get_pal_offset(obj));
        object_set_layers(scrap, LAYER_SCRAP);
        object_tick(scrap);
        object_set_shadow(scrap, 1);
        scrap_create(scrap);
        game_state_add_object(obj->gs, scrap, RENDER_LAYER_TOP);
    }

}

void har_block(object *obj, vec2i hit_coord) {
    har *h = obj->userdata;
    if (h->state == STATE_WALKFROM) {
        object_set_animation(obj, &af_get_move(h->af_data, ANIM_STANDING_BLOCK)->ani);
    } else {
        object_set_animation(obj, &af_get_move(h->af_data, ANIM_CROUCHING_BLOCK)->ani);
    }
    // the HARs have a lame blank frame in their animation string, so use a custom one
    object_set_custom_string(obj, "A5");
    object_set_repeat(obj, 0);
    object_tick(obj);
    // blocking spark
    if (h->damage_received) {
        // don't make another scrape
        return;
    }
    object *scrape = malloc(sizeof(object));
    object_create(scrape, obj->gs, hit_coord, vec2f_create(0, 0));
    object_set_animation(scrape, &af_get_move(h->af_data, ANIM_BLOCKING_SCRAPE)->ani);
    object_set_stl(scrape, object_get_stl(obj));
    object_set_direction(scrape, object_get_direction(obj));
    object_set_repeat(scrape, 0);
    object_set_gravity(scrape, 0);
    object_set_layers(scrape, LAYER_SCRAP);
    object_tick(scrape);
    object_tick(scrape);
    game_state_add_object(obj->gs, scrape, RENDER_LAYER_MIDDLE);
    h->damage_received = 1;
    h->flinching = 1;
}

void har_check_closeness(object *obj_a, object *obj_b) {
    vec2i pos_a = object_get_pos(obj_a);
    vec2i pos_b = object_get_pos(obj_b);
    har *a = object_get_userdata(obj_a);
    har *b = object_get_userdata(obj_b);
    int hard_limit = 35; // Push opponent if HARs too close. Harrison-Stetson method value.
    int soft_limit = 45; // Sets HAR A as being close to HAR B if closer than this.

    if (b->state == STATE_RECOIL || a->state == STATE_RECOIL || b->state == STATE_JUMPING || a->state == STATE_JUMPING || a->state == STATE_FALLEN || b->state == STATE_FALLEN) {
        return;
    }

    // Reset closeness state
    a->close = 0;
    a->hard_close = 0;

    // If HARs get too close together, handle it
    if(har_is_walking(a) && object_get_direction(obj_a) == OBJECT_FACE_LEFT) {
        if(pos_a.x < pos_b.x + hard_limit && pos_a.x > pos_b.x) {
            // don't allow hars to overlap in the corners
            if (pos_b.x > 15) {
                pos_b.x = pos_a.x - hard_limit;
                object_set_pos(obj_b, pos_b);
            } else {
                pos_a.x = pos_b.x + hard_limit;
                object_set_pos(obj_a, pos_a);
            }
            a->hard_close = 1;
        }
        if(pos_a.x < pos_b.x + soft_limit && pos_a.x > pos_b.x) {
            if (b->state == STATE_STANDING || b->state == STATE_STUNNED || har_is_walking(b) || har_is_crouching(b)) {
                a->close = 1;
            }
            a->hard_close = 1;
        }
    }
    if(har_is_walking(a) && object_get_direction(obj_a) == OBJECT_FACE_RIGHT) {
        if(pos_a.x + hard_limit > pos_b.x && pos_a.x < pos_b.x) {
            // don't allow hars to overlap in the corners
            if (pos_b.x < 305) {
                pos_b.x = pos_a.x + hard_limit;
                object_set_pos(obj_b, pos_b);
            } else {
                pos_a.x = pos_b.x - hard_limit;
                object_set_pos(obj_a, pos_a);
            }
            a->hard_close = 1;
        }
        if(pos_a.x + soft_limit > pos_b.x && pos_a.x < pos_b.x) {
            if (b->state == STATE_STANDING || b->state == STATE_STUNNED || har_is_walking(b) || har_is_crouching(b)) {
                a->close = 1;
            }
            a->hard_close = 1;
        }
    }
}

void har_collide_with_har(object *obj_a, object *obj_b, int loop) {
    har *a = object_get_userdata(obj_a);
    har *b = object_get_userdata(obj_b);

    sd_stringparser_frame f = obj_a->animation_state.parser->current_frame;

    if (frame_isset(&f, "ua")) {
        obj_b->sprite_state.disable_gravity=1;
    }

    if (b->state == STATE_FALLEN || b->state == STATE_STANDING_UP) {
        // can't hit em while they're down
        return;
    }


    // Check for collisions by sprite collision points
    int level = 1;
    af_move *move = af_get_move(a->af_data, obj_a->cur_animation->id);
    vec2i hit_coord = vec2i_create(0, 0);
    if(obj_a->can_hit) {
        a->damage_done = 0;
        obj_a->can_hit = 0;
        obj_a->hit_frames = 0;
    }
    if(a->damage_done == 0 &&
            (intersect_sprite_hitpoint(obj_a, obj_b, level, &hit_coord)
            || move->category == CAT_CLOSE ||
            (frame_isset(&f, "ue") && b->state != STATE_JUMPING))) {

        if (har_is_blocking(b, move)) {
            har_event_enemy_block(a, move);
            har_block(obj_b, hit_coord);
            return;
        }

        // is the HAR invulnerable to this kind of attack?
        if (har_is_invincible(obj_b, move)) {
            return;
        }

        vec2i hit_coord2 = vec2i_create(0, 0);

        if(b->damage_done == 0 && loop == 0 && intersect_sprite_hitpoint(obj_b, obj_a, level, &hit_coord2)) {
            DEBUG("both hars hit at the same time!");
            har_collide_with_har(obj_b, obj_a, 1);
        }


        if (move->category == CAT_CLOSE) {
          a->close = 0;
        }

        if ((b->state == STATE_STUNNED || b->state == STATE_RECOIL) && object_get_direction(obj_a) == object_get_direction(obj_b)) {
            // opponent is stunned and facing the same way we are, backwards
            // so flip them around
            object_set_direction(obj_b, object_get_direction(obj_a) * -1);
        }

        if (move->next_move) {
            object_set_animation(obj_a, &af_get_move(a->af_data, move->next_move)->ani);
            object_set_repeat(obj_a, 0);
            return;
        }

        har_event_take_hit(b, move);
        har_event_land_hit(a, move);

        if (b->state == STATE_RECOIL) {
            // back the attacker off a little
            a->flinching = 1;
        }
        har_take_damage(obj_b, &move->footer_string, move->damage);
        if (hit_coord.x != 0 || hit_coord.y != 0) {
            har_spawn_scrap(obj_b, hit_coord, move->scrap_amount);
        }
        a->damage_done = 1;
        b->damage_received = 1;

        if (move->category == CAT_CLOSE) {
            // never flinch from a throw
            b->flinching = 0;
        }

        DEBUG("HAR %s to HAR %s collision at %d,%d!", 
            get_id_name(a->id), 
            get_id_name(b->id),
            hit_coord.x,
            hit_coord.y);
        DEBUG("HAR %s animation set to %s", get_id_name(b->id), str_c(&move->footer_string));
    }
}

void har_collide_with_projectile(object *o_har, object *o_pjt) {
    har *h = object_get_userdata(o_har);
    af *prog_owner_af_data = projectile_get_af_data(o_pjt);
    // lol
    har *other = object_get_userdata(game_state_get_player(o_har->gs, abs(h->player_id - 1))->har);

    if (h->state == STATE_FALLEN || h->state == STATE_STANDING_UP) {
        // can't hit em while they're down
        return;
    }

    // Check for collisions by sprite collision points
    int level = 2;
    vec2i hit_coord;
    if(intersect_sprite_hitpoint(o_pjt, o_har, level, &hit_coord)) {
        af_move *move = af_get_move(prog_owner_af_data, o_pjt->cur_animation->id);
        if (har_is_blocking(h, move)) {
            har_event_enemy_block(other, move);
            har_block(o_har, hit_coord);
            return;
        }

        // is the HAR invulnerable to this kind of attack?
        if (har_is_invincible(o_har, move)) {
            return;
        }

        int damage = 0;

        if (move->successor_id) {
            af_move *next_move = af_get_move(prog_owner_af_data, move->successor_id);
            if (!move->footer_string.data) {
                DEBUG("using sucessor footer string %s", str_c(&next_move->footer_string));
                har_take_damage(o_har, &next_move->footer_string, move->damage);
                damage = 1;
            }
            object_set_animation(o_pjt, &next_move->ani);
            object_set_repeat(o_pjt, 0);
            /*object_set_vel(o_pjt, vec2f_create(0,0));*/
            o_pjt->animation_state.finished = 0;
        }

        if (!damage) {
            har_take_damage(o_har, &move->footer_string, move->damage);
        }

        har_event_take_hit(h, move);
        har_event_land_hit(other, move);

        /*if (h->hit_hook_cb) {*/
            /*h->hit_hook_cb(h->player_id, abs(h->player_id - 1), move, h->hit_hook_cb_data);*/
        /*}*/
        har_spawn_scrap(o_har, hit_coord, move->scrap_amount);
        h->damage_received = 1;

        vec2f vel = object_get_vel(o_har);
        vel.x = 0.0f;
        object_set_vel(o_har, vel);

        if (move->successor_id) {
            af_move *next_move = af_get_move(prog_owner_af_data, move->successor_id);
            if (!move->footer_string.data && next_move->footer_string.data) {
                DEBUG("using sucessor footer string %s", str_c(&next_move->footer_string));
                object_set_custom_string(o_har, str_c(&next_move->footer_string));
            }
            object_set_animation(o_pjt, &next_move->ani);
            object_set_repeat(o_pjt, 0);
            /*object_set_vel(o_pjt, vec2f_create(0,0));*/
            o_pjt->animation_state.finished = 0;
        }

        DEBUG("PROJECTILE %d to HAR %s collision at %d,%d!", 
            object_get_animation(o_pjt)->id, 
            get_id_name(h->id),
            hit_coord.x,
            hit_coord.y);
        DEBUG("HAR %s animation set to %s", get_id_name(h->id), str_c(&move->footer_string));
    }
}

void har_collide_with_hazard(object *o_har, object *o_pjt) {
    har *h = object_get_userdata(o_har);
    bk *bk_data = object_get_userdata(o_pjt);
    bk_info *anim = bk_get_info(bk_data, o_pjt->cur_animation->id);

    if (h->state == STATE_FALLEN || h->state == STATE_STANDING_UP) {
        // can't hit em while they're down
        return;
    }

    if (h->state == STATE_VICTORY || h->state == STATE_DEFEAT || h->state == STATE_SCRAP || h->state == STATE_DESTRUCTION || h->state == STATE_DONE) {
        // Hazards should not affect HARs at the end of a match
        return;
    }

    // Check for collisions by sprite collision points
    int level = 2;
    vec2i hit_coord;
    if(!h->damage_received && intersect_sprite_hitpoint(o_pjt, o_har, level, &hit_coord)) {

        har_take_damage(o_har, &anim->footer_string, anim->hazard_damage);
        har_event_hazard_hit(h, anim);
        if (anim->chain_no_hit) {
            object_set_animation(o_pjt, &bk_get_info(bk_data, anim->chain_no_hit)->ani);
            object_set_repeat(o_pjt, 0);
        }
        har_spawn_scrap(o_har, hit_coord, 9);
        h->damage_received = 1;
    } else if (anim->chain_hit && intersect_sprite_hitpoint(o_har, o_pjt, level, &hit_coord)) {
        // we can punch this! Only set on fire pit orb
        anim = bk_get_info(bk_data, anim->chain_hit);
        o_pjt->animation_state.enemy = o_har->animation_state.enemy;
        object_set_animation(o_pjt, &anim->ani);
        object_set_repeat(o_pjt, 0);
        o_pjt->animation_state.finished = 0;
    }
}

void har_collide(object *obj_a, object *obj_b) {
    // Check if this is projectile to har collision
    if(object_get_layers(obj_a) & LAYER_PROJECTILE) {
        har_collide_with_projectile(obj_b, obj_a);
        return;
    }
    if(object_get_layers(obj_b) & LAYER_PROJECTILE) {
        har_collide_with_projectile(obj_a, obj_b);
        return;
    }

    if(object_get_layers(obj_a) & LAYER_HAZARD) {
        /*DEBUG("har collided with hazard");*/
        har_collide_with_hazard(obj_b, obj_a);
        return;
    }
    if(object_get_layers(obj_b) & LAYER_HAZARD) {
        /*DEBUG("har collided with hazard");*/
        har_collide_with_hazard(obj_a, obj_b);
        return;
    }


    // Check for closeness between HARs and handle it
    har_check_closeness(obj_a, obj_b);
    har_check_closeness(obj_b, obj_a);

    // Handle har collisions
    har_collide_with_har(obj_a, obj_b, 0);
    har_collide_with_har(obj_b, obj_a, 0);
}

void har_tick(object *obj) {
    har *h = object_get_userdata(obj);
    // Make sure HAR doesn't walk through walls
    // TODO: Roof!
    vec2i pos = object_get_pos(obj);
    if (h->state != STATE_DEFEAT) {
        sd_stringparser_frame f = obj->animation_state.parser->current_frame;
        int wall_flag = frame_isset(&f, "aw");
        int wall = 0;
        int hit = 0;
        if(pos.x <  15) {
            pos.x = 15;
            hit = 1;
        } else if(pos.x > 305) {
            pos.x = 305;
            wall = 1;
            hit = 1;
        }

        object_set_pos(obj, pos);

        if (hit && wall_flag) {
            af_move *move = af_get_move(h->af_data, obj->cur_animation->id);
            if (move->next_move) {
                DEBUG("wall hit chaining to next animation %d", move->next_move);
                har_set_ani(obj, move->next_move, 0);
            }
        }

        if (hit) {
            har_event_hit_wall(h, wall);
        }
    }

    if ((h->state == STATE_VICTORY || h->state == STATE_DONE) && 
               obj->animation_state.parser->current_frame.is_final_frame && obj->animation_state.entered_frame == 1) {
        // match is over
        har_event_done(h);
    }

    if (pos.y < 190 && h->state == STATE_RECOIL) {
        DEBUG("switching to fallen");
        h->state = STATE_FALLEN;
        har_event_recover(h);
    }

    if (h->state == STATE_STUNNED) {
        h->stun_timer++;
        if(h->stun_timer % 10 == 0) {
            vec2i pos = object_get_pos(obj);
            pos.y -= 60;
            har_spawn_oil(obj, pos, 5, 0.5f, RENDER_LAYER_BOTTOM);
        }
        if (h->stun_timer > 100) {
            har_stunned_done(obj);
        }
    }

    // Stop HAR from sliding if touching the ground
    if(h->state != STATE_JUMPING && h->state != STATE_FALLEN && h->state != STATE_RECOIL) {
        if(!har_is_walking(h) && h->executing_move == 0) {
            vec2f vel = object_get_vel(obj);
            vel.x = 0;
            object_set_vel(obj, vel);
        }
    }
    if(h->flinching) {
        vec2f push = object_get_vel(obj);
        // The infamous Harrison-Stetson method
        // XXX TODO is there a non-hardcoded value that we could use?
        if(h->executing_move == 0 && (h->state == STATE_CROUCHBLOCK || h->state == STATE_WALKFROM)) {
            push.x = 1.0f * -object_get_direction(obj);
        } else {
            push.x = 4.0f * -object_get_direction(obj);
        }
        object_set_vel(obj, push);
        h->flinching = 0;
    }

    if (h->endurance < h->endurance_max && !(h->executing_move || h->state == STATE_RECOIL || h->state == STATE_STUNNED || h->state == STATE_FALLEN || h->state == STATE_STANDING_UP || h->state == STATE_DEFEAT)) {
        h->endurance += 1;
    }

    int act_pos = obj->age % OBJECT_EVENT_BUFFER_SIZE;
    if (h->act_buf[act_pos].age == obj->age) {
        DEBUG("REPLAYING %d inputs", h->act_buf[act_pos].count);
        for (int i = 0; i < h->act_buf[act_pos].count; i++) {
            har_act(obj, h->act_buf[act_pos].actions[i]);
        }
    } else {
        // clear the action buffer because we're onto a new tick
        h->act_buf[act_pos].count = 0;
    }
}

void add_input_to_buffer(char *buf, char c) {
    // only add it if it is not the current head of the array
    if(buf[0] == c) {
        return;
    }

    // use memmove to move everything over one spot in the array, leaving the first slot free
    memmove((buf)+1, buf, 9);
    // write the new first element
    buf[0] = c;
}

void add_input(char *buf, int act_type, int direction) {
   // for the reason behind the numbers, look at a numpad sometime
    switch(act_type) {
        case ACT_UP:
            add_input_to_buffer(buf, '8');
            break;
        case ACT_DOWN:
            add_input_to_buffer(buf, '2');
            break;
        case ACT_LEFT:
            if(direction == OBJECT_FACE_LEFT) {
                add_input_to_buffer(buf, '6');
            } else {
                add_input_to_buffer(buf, '4');
            }
            break;
        case ACT_RIGHT:
            if(direction == OBJECT_FACE_LEFT) {
                add_input_to_buffer(buf, '4');
            } else {
                add_input_to_buffer(buf, '6');
            }
            break;
        case ACT_UPRIGHT:
            if(direction == OBJECT_FACE_LEFT) {
                add_input_to_buffer(buf, '7');
            } else {
                add_input_to_buffer(buf, '9');
            }
            break;
        case ACT_UPLEFT:
            if(direction == OBJECT_FACE_LEFT) {
                add_input_to_buffer(buf, '9');
            } else {
                add_input_to_buffer(buf, '7');
            }
            break;
        case ACT_DOWNRIGHT:
            if(direction == OBJECT_FACE_LEFT) {
                add_input_to_buffer(buf, '1');
            } else {
                add_input_to_buffer(buf, '3');
            }
            break;
        case ACT_DOWNLEFT:
            if(direction == OBJECT_FACE_LEFT) {
                add_input_to_buffer(buf, '3');
            } else {
                add_input_to_buffer(buf, '1');
            }
            break;
        case ACT_KICK:
            add_input_to_buffer(buf, 'K');
            break;
        case ACT_PUNCH:
            add_input_to_buffer(buf, 'P');
            break;
        case ACT_STOP:
            add_input_to_buffer(buf, '5');
            break;
    }
}

af_move* match_move(object *obj, char *inputs) {
    har *h = object_get_userdata(obj);
    af_move *move = NULL;
    size_t len;
    for(int i = 0; i < 70; i++) {
        if((move = af_get_move(h->af_data, i))) {
            len = move->move_string.len;
            if(!strncmp(str_c(&move->move_string), inputs, len)) {
                sd_stringparser_frame f = obj->animation_state.parser->current_frame;
                if (move->category == CAT_CLOSE && h->close != 1) {
                    // not standing close enough
                    continue;
                }
                if (move->category == CAT_JUMPING && h->state != STATE_JUMPING) {
                    // not jumping
                    continue;
                }
                if (move->category != CAT_JUMPING && h->state == STATE_JUMPING) {
                    // jumping but this move is not a jumping move
                    continue;
                }
                if (move->category == CAT_SCRAP && h->state != STATE_VICTORY) {
                    continue;
                }

                if (move->category == CAT_DESTRUCTION && h->state != STATE_SCRAP) {
                    continue;
                }

                if (h->executing_move) {
                    // check if the current frame allows chaining
                   int allowed = 0;
                   if (frame_isset(&f, "jn") && i == frame_get(&f, "jn")) {
                       allowed = 1;
                   } else {
                       switch (move->category) {
                           case CAT_LOW:
                               if (frame_isset(&f, "jl")) {
                                   allowed = 1;
                               }
                               break;
                           case CAT_MEDIUM:
                               if (frame_isset(&f, "jm")) {
                                   allowed = 1;
                               }
                               break;
                           case CAT_HIGH:
                               if (frame_isset(&f, "jh")) {
                                   allowed = 1;
                               }
                               break;
                           case CAT_SCRAP:
                               if (frame_isset(&f, "jf")) {
                                   allowed = 1;
                               }
                               break;
                           case CAT_DESTRUCTION:
                               if (frame_isset(&f, "jf2")) {
                                   allowed = 1;
                               }
                               break;
                       }
                   }
                   if (!allowed) {
                       // not allowed
                       continue;
                   }
                   DEBUG("CHAINING");
                }

                DEBUG("matched move %d with string %s", i, str_c(&move->move_string));
                /*DEBUG("input was %s", h->inputs);*/
                return move;
            }
        }
    }
    return NULL;
}
af_move* scrap_destruction_cheat(object *obj, char *inputs) {
    har *h = object_get_userdata(obj);
    for(int i = 0; i < 70; i++) {
        af_move *move;
        if((move = af_get_move(h->af_data, i))) {
            if (move->category == CAT_SCRAP && h->state == STATE_VICTORY && inputs[0] == 'K') {
                return move;
            }

            if (move->category == CAT_DESTRUCTION && h->state == STATE_SCRAP && inputs[0] == 'P') {
                return move;
            }
        }
    }
    return NULL;
}


int maybe_har_change_state(int oldstate, int direction, int act_type) {
    int state = 0;
    switch(act_type) {
        case ACT_DOWNRIGHT:
            if (direction == OBJECT_FACE_LEFT) {
                state = STATE_CROUCHBLOCK;
            } else {
                state = STATE_CROUCHING;
            }
            break;
        case ACT_DOWNLEFT:
            if (direction == OBJECT_FACE_RIGHT) {
                state = STATE_CROUCHBLOCK;
            } else {
                state = STATE_CROUCHING;
            }
            break;
        case ACT_DOWN:
            state = STATE_CROUCHING;
            break;
        case ACT_STOP:
            state = STATE_STANDING;
            break;
        case ACT_LEFT:
            if (direction == OBJECT_FACE_LEFT) {
                state = STATE_WALKTO;
            } else {
                state = STATE_WALKFROM;
            }
            break;
        case ACT_RIGHT:
            if (direction == OBJECT_FACE_RIGHT) {
                state = STATE_WALKTO;
            } else {
                state = STATE_WALKFROM;
            }
            break;
        case ACT_UP:
            state = STATE_JUMPING;
            break;
        case ACT_UPLEFT:
            state = STATE_JUMPING;
            break;
        case ACT_UPRIGHT:
            state = STATE_JUMPING;
            break;
    }
    if (oldstate != state) {
        // we changed state
        return state;
    }
    return 0;
}

int har_act(object *obj, int act_type) {
    har *h = object_get_userdata(obj);
    int direction = object_get_direction(obj);
    if(!(h->state == STATE_STANDING ||
         har_is_walking(h) ||
         har_is_crouching(h) ||
         h->state == STATE_JUMPING ||
         h->state == STATE_VICTORY ||
         h->state == STATE_SCRAP)) {
        // doing something else, ignore input
        return 0;
    }

    // Don't allow movement if arena is starting or ending
    int arena_state = arena_get_state(game_state_get_scene(obj->gs));
    if(arena_state == ARENA_STATE_STARTING) {
        return 0;
    }

    int oldstate = h->state;

    add_input(h->inputs, act_type, direction);

    af_move *move = match_move(obj, h->inputs);

    if(game_state_get_player(obj->gs, h->player_id)->ez_destruct && move == NULL && (h->state == STATE_VICTORY || h->state == STATE_SCRAP)) {
        move = scrap_destruction_cheat(obj, h->inputs);
    }

    if (move) {
        char *s = (char*)str_c(&move->move_string); // start
        for (int j = str_size(&move->move_string)-1; j >=0; j--) { 
            switch(s[j]) {
                case '1':
                    if(direction == OBJECT_FACE_LEFT) {
                        har_action_hook(obj, ACT_DOWNRIGHT);
                    } else {
                        har_action_hook(obj, ACT_DOWNLEFT);
                    }
                    break;
                case '2':
                    har_action_hook(obj, ACT_DOWN);
                    break;
                case '3':
                    if(direction == OBJECT_FACE_LEFT) {
                        har_action_hook(obj, ACT_DOWNLEFT);
                    } else {
                        har_action_hook(obj, ACT_DOWNRIGHT);
                    }
                    break;
                case '4':
                    if(direction == OBJECT_FACE_LEFT) {
                        har_action_hook(obj, ACT_RIGHT);
                    } else {
                        har_action_hook(obj, ACT_LEFT);
                    }
                    break;
                case '5':
                    har_action_hook(obj, ACT_STOP);
                    break;
                case '6':
                    if(direction == OBJECT_FACE_LEFT) {
                        har_action_hook(obj, ACT_LEFT);
                    } else {
                        har_action_hook(obj, ACT_RIGHT);
                    }
                    break;
                case '7':
                    if(direction == OBJECT_FACE_LEFT) {
                        har_action_hook(obj, ACT_UPRIGHT);
                    } else {
                        har_action_hook(obj, ACT_UPLEFT);
                    }
                    break;
                case '8':
                    har_action_hook(obj, ACT_UP);
                    break;
                case '9':
                    if(direction == OBJECT_FACE_LEFT) {
                        har_action_hook(obj, ACT_UPLEFT);
                    } else {
                        har_action_hook(obj, ACT_UPRIGHT);
                    }
                    break;
                case 'K':
                    har_action_hook(obj, ACT_KICK);
                    break;
                case 'P':
                    har_action_hook(obj, ACT_PUNCH);
                    break;
            }
        }
        har_action_hook(obj, ACT_FLUSH);

        // Set correct animation etc.
        // executing_move = 1 prevents new moves while old one is running.
        har_set_ani(obj, move->id, 0);
        h->inputs[0] = '\0';
        h->executing_move = 1;

        // Stop horizontal movement, when move is done
        // TODO: Make this work better
        vec2f spd = object_get_vel(obj);
        if (h->state != STATE_JUMPING) {
            spd.x = 0.0f;
        }
        object_set_vel(obj, spd);

        if (move->category == CAT_SCRAP) {
            DEBUG("going to scrap state");
            h->state = STATE_SCRAP;
            har_event_scrap(h);
        } else if (move->category == CAT_DESTRUCTION) {
            DEBUG("going to destruction state");
            h->state = STATE_DESTRUCTION;
            har_event_destruction(h);
        } else {
            har_event_attack(h, move);
        }

        // make the other har participate in the scrap/destruction
        if (move->category == CAT_SCRAP || move->category == CAT_DESTRUCTION) {
            int opp_id = h->player_id ? 0 : 1;
            af_move *move = af_get_move(h->af_data, obj->cur_animation->id);
            object *opp = game_player_get_har(game_state_get_player(obj->gs, opp_id));
            object_set_animation(opp, &af_get_move(((har*)opp->userdata)->af_data, ANIM_DAMAGE)->ani);
            object_set_repeat(opp, 0);
            object_set_custom_string(opp, str_c(&move->footer_string));
            object_tick(opp);
        }

        // we actually did something interesting
        // return 1 so we can use this as sync point for netplay
        return 1;
    }

    // Don't allow new movement while we're still executing a move
    if(h->executing_move) {
        return 0;
    }

    if(obj->pos.y < 190) {
        // airborne
        return 0;
    }

    if(arena_state == ARENA_STATE_ENDING) {
        return 0;
    }

    float vx, vy;
    // no moves matched, do player movement
    int newstate;
    if ((newstate = maybe_har_change_state(h->state, direction, act_type))) {
        h->state = newstate;
        switch(newstate) {
            case STATE_CROUCHBLOCK:
                har_set_ani(obj, ANIM_CROUCHING, 1);
                object_set_vel(obj, vec2f_create(0,0));
                break;
            case STATE_CROUCHING:
                har_set_ani(obj, ANIM_CROUCHING, 1);
                object_set_vel(obj, vec2f_create(0,0));
                break;
            case STATE_STANDING:
                har_set_ani(obj, ANIM_IDLE, 1);
                object_set_vel(obj, vec2f_create(0,0));
                obj->slide_state.vel.x = 0;
                break;
            case STATE_WALKTO:
                har_set_ani(obj, ANIM_WALKING, 1);
                vx = (h->af_data->forward_speed*direction)/(float)320;
                object_set_vel(obj, vec2f_create(vx*(h->hard_close ? 0.5 : 1.0),0));
                break;
            case STATE_WALKFROM:
                har_set_ani(obj, ANIM_WALKING, 1);
                vx = (h->af_data->reverse_speed*direction*-1)/(float)320;
                object_set_vel(obj, vec2f_create(vx*(h->hard_close ? 0.5 : 1.0),0));
                break;
            case STATE_JUMPING:
                har_set_ani(obj, ANIM_JUMPING, 0);
                vx = 0.0f;
                vy = (float)h->af_data->jump_speed * FUDGEFACTOR;
                int jump_dir = 0;
                if ((act_type == ACT_UPLEFT && direction == OBJECT_FACE_LEFT) ||
                        (act_type == ACT_UPRIGHT && direction == OBJECT_FACE_RIGHT)) {
                    vx = (h->af_data->forward_speed*direction)/(float)320;
                    object_set_tick_pos(obj, 110);
                    object_set_stride(obj, 7); // Pass 10 frames per tick
                    jump_dir = 1;
                } else if (act_type == ACT_UPLEFT || act_type == ACT_UPRIGHT) {
                    // If we are jumping backwards, start animation from end
                    // at -100 frames (seems to be about right)
                    object_set_playback_direction(obj, PLAY_BACKWARDS);
                    object_set_tick_pos(obj, -110);
                    vx = (h->af_data->reverse_speed*direction*-1)/(float)320;
                    object_set_stride(obj, 7); // Pass 10 frames per tick
                    jump_dir = -1;
                }
                if (oldstate == STATE_CROUCHING || oldstate == STATE_CROUCHBLOCK) {
                    // jumping frop crouch makes you jump 25% higher
                    vy = vy * 1.25;
                    vx = vx * 1.25;
                }
                object_set_gravity(obj, h->af_data->fall_speed * FUDGEFACTOR);
                object_set_vel(obj, vec2f_create(vx,vy));
                har_event_jump(h, jump_dir);
                break;
        }
        har_action_hook(obj, act_type);
        har_action_hook(obj, ACT_FLUSH);
        return 1;
    }

    return 0;
}

void har_finished(object *obj) {
    har *h = object_get_userdata(obj);
    if (h->state == STATE_SCRAP || h->state == STATE_DESTRUCTION) {
        // play vistory animation again, but do not allow any more moves to be executed
        h->state = STATE_DONE;
        har_set_ani(obj, ANIM_VICTORY, 0);
    } else if (h->state == STATE_VICTORY || h->state == STATE_DONE) {
        // end the arena
        DEBUG("ending arena!");
        game_state_set_next(obj->gs, SCENE_MENU);
    } else if (h->state == STATE_RECOIL && h->endurance <= 0 && h->health <= 0) {
        h->state = STATE_DEFEAT;
        har_set_ani(obj, ANIM_DEFEAT, 0);
        har_event_defeat(h);
    } else if ((h->state == STATE_RECOIL || h->state == STATE_STANDING_UP) && h->endurance <= 0) {
        if (h->state == STATE_RECOIL) {
            har_event_recover(h);
        }
        h->state = STATE_STUNNED;
        h->stun_timer = 0;
        har_set_ani(obj, ANIM_STUNNED, 1);
        har_event_stun(h);
    } else if (h->state == STATE_RECOIL) {
        har_event_recover(h);
        h->state = STATE_STANDING;
        har_set_ani(obj, ANIM_IDLE, 1);
    } else if(h->state != STATE_CROUCHING && h->state != STATE_CROUCHBLOCK) {
        // Don't transition to standing state while in midair
        if(h->state != STATE_JUMPING) { h->state = STATE_STANDING; }
        har_set_ani(obj, ANIM_IDLE, 1);
    } else {
        har_set_ani(obj, ANIM_CROUCHING, 1);
    }
    h->executing_move = 0;
    h->flinching = 0;
}

int har_serialize(object *obj, serial *ser) {
    har *h = object_get_userdata(obj);

    // Specialization
    serial_write_int8(ser, SPECID_HAR);

    // Set serialization data
    serial_write_int16(ser, h->id);
    serial_write_int8(ser, h->player_id);
    serial_write_int8(ser, h->pilot_id);
    serial_write_int8(ser, h->state);
    serial_write_int8(ser, h->executing_move);
    serial_write_int8(ser, h->flinching);
    serial_write_int8(ser, h->close);
    serial_write_int8(ser, h->hard_close);
    serial_write_int8(ser, h->damage_done);
    serial_write_int8(ser, h->damage_received);
    serial_write_int16(ser, h->health);
    serial_write_int16(ser, h->endurance);
    serial_write(ser, h->inputs, 10);

    // ...
    // TODO: Set the other ser attrs here

    // Return success
    return 0;
}

int har_unserialize(object *obj, serial *ser, int animation_id, game_state *gs) {

    int har_id = serial_read_int16(ser);
    int player_id = serial_read_int8(ser);
    int pilot_id = serial_read_int8(ser);
    af *af_data;

    /*DEBUG("unserializing HAR %d for player %d", har_id - HAR_JAGUAR, player_id);*/

    // find the AF data in the scene

    if (gs->sc->af_data[player_id]->id == har_id - HAR_JAGUAR) {
        af_data = gs->sc->af_data[player_id];
    } else {
        DEBUG("expected har %d, got %d", har_id - HAR_JAGUAR, gs->sc->af_data[player_id]->id);
        // HAR IDs do not match!
        // TODO maybe the other player changed their HAR, who knows
        return 1;
    }

    har_create(obj, af_data, obj->direction, har_id, pilot_id, player_id);

    har *h = object_get_userdata(obj);
    // we are unserializing a state update for a HAR, we expect it to have the AF data already loaded into RAM, we're just updating the volatile attributes

    // TODO sanity check pilot/player/HAR IDs
    h->state = serial_read_int8(ser);
    h->executing_move = serial_read_int8(ser);
    h->flinching = serial_read_int8(ser);
    h->close = serial_read_int8(ser);
    h->hard_close = serial_read_int8(ser);
    h->damage_done = serial_read_int8(ser);
    h->damage_received = serial_read_int8(ser);
    h->health = serial_read_int16(ser);
    h->endurance = serial_read_int16(ser);
    serial_read(ser, h->inputs, 10);

    /*DEBUG("har animation id is %d with state %d with %d", animation_id, h->state, h->executing_move);*/

    object_set_animation(obj, &af_get_move(af_data, animation_id)->ani);

    if (h->executing_move && (animation_id == ANIM_IDLE || animation_id == ANIM_CROUCHING)) {
        // XXX this is a hack to fix a bug we can't find
        DEBUG("============== HACK ATTACK =========================");
        h->executing_move = 0;
    }

    // Return success
    return 0;
}

void har_install_action_hook(har *h, har_action_hook_cb hook, void *data) {
    h->action_hook_cb = hook;
    h->action_hook_cb_data = data;
}

void har_install_hook(har *h, har_hook_cb hook, void *data) {
    har_hook hk;
    hk.cb = hook;
    hk.data = data;
    list_append(&h->har_hooks, &hk, sizeof(har_hook));

    /*h->hook_cb = hook;*/
    /*h->hook_cb_data = data;*/
}

void har_bootstrap(object *obj) {
    object_set_serialize_cb(obj, har_serialize);
    object_set_unserialize_cb(obj, har_unserialize);
}

void har_copy_actions(object *new, object *old) {
    har *h_new = object_get_userdata(new);
    har *h_old = object_get_userdata(old);
    memcpy(h_new->act_buf, h_old->act_buf, sizeof(action_buffer) * OBJECT_EVENT_BUFFER_SIZE);
}

int har_create(object *obj, af *af_data, int dir, int har_id, int pilot_id, int player_id) {
    // Create local data
    har *local = malloc(sizeof(har));
    object_set_userdata(obj, local);
    har_bootstrap(obj);

    local->gp = game_state_get_player(obj->gs, player_id);
    local->af_data = af_data;

    // Save har id
    local->id = har_id;
    local->player_id = player_id;
    local->pilot_id = pilot_id;

    // Health, endurance
    local->health_max = local->health = 100;
    local->endurance_max = local->endurance = 400;
    local->close = 0;
    local->hard_close =  0;
    local->state = STATE_STANDING;
    local->executing_move = 0;

    local->delay = 0;

    local->action_hook_cb = NULL;
    local->action_hook_cb_data = NULL;

    /*local->hook_cb = NULL;*/
    /*local->hook_cb_data = NULL;*/

    list_create(&local->har_hooks);

    local->stun_timer = 0;

    // Set palette offset 0 for player1, 48 for player2
    object_set_pal_offset(obj, player_id * 48);

    // Object related stuff
    /*object_set_gravity(obj, local->af_data->fall_speed);*/
    object_set_gravity(obj, 1);
    object_set_layers(obj, LAYER_HAR | (player_id == 0 ? LAYER_HAR1 : LAYER_HAR2));
    object_set_direction(obj, dir);
    object_set_repeat(obj, 1);
    object_set_stl(obj, local->af_data->sound_translation_table);
    object_set_shadow(obj, 1);

    // New object spawner callback
    object_set_spawn_cb(obj, cb_har_spawn_object, local);

    // Set running animation 
    har_set_ani(obj, ANIM_IDLE, 1);

    // fill the input buffer with 'pauses'
    memset(local->inputs, '5', 10);
    local->inputs[10] = '\0';

    // Callbacks and userdata
    object_set_free_cb(obj, har_free);
    object_set_act_cb(obj, har_act);
    object_set_tick_cb(obj, har_tick);
    object_set_move_cb(obj, har_move);
    object_set_collide_cb(obj, har_collide);
    object_set_finish_cb(obj, har_finished);
    //object_set_debug_cb(obj, har_debug);

    for (int i = 0; i < OBJECT_EVENT_BUFFER_SIZE; i++) {
        local->act_buf[i].count = 0;
        local->act_buf[i].age = 0;
    }

    // All done
    return 0;
}
