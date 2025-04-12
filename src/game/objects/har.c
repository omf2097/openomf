#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio/audio.h"
#include "controller/controller.h"
#include "formats/af.h"
#include "game/common_defines.h"
#include "game/game_state.h"
#include "game/objects/arena_constraints.h"
#include "game/objects/har.h"
#include "game/objects/projectile.h"
#include "game/objects/scrap.h"
#include "game/protos/intersect.h"
#include "game/scenes/arena.h"
#include "game/utils/serial.h"
#include "resources/af_loader.h"
#include "resources/animation.h"
#include "resources/pilots.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/miscmath.h"
#include "utils/random.h"
#include "video/damage_tracker.h"
#include "video/vga_state.h"
#include "video/video.h"

#define IS_ZERO(n) (n < 0.8 && n > -0.8)

void har_finished(object *obj);
int har_act(object *obj, int act_type);
void har_spawn_scrap(object *obj, vec2i pos, int amount);

void har_free(object *obj) {
    har *h = object_get_userdata(obj);
    list_free(&h->har_hooks);
    hashmap_free(&h->disabled_animations);
#ifdef DEBUGMODE
    surface_free(&h->hit_pixel);
    surface_free(&h->har_origin);
#endif
    vector_free(&h->child_objects);
    omf_free(h);
    object_set_userdata(obj, NULL);
}

/* hooks */

void fire_hooks(har *h, har_event event, controller *ctrl) {
    iterator it;
    har_hook *hook;

    list_iter_begin(&h->har_hooks, &it);
    foreach(it, hook) {
        hook->cb(event, ctrl->gs->sc);
    }
    if(object_get_userdata(game_state_find_object(ctrl->gs, ctrl->har_obj_id)) == h) {
        controller_har_hook(ctrl, event);
    }
}

void har_event_jump(har *h, int direction, controller *ctrl) {
    // direction is -1, 0 or 1, for backwards, up and forwards
    har_event event;
    event.type = HAR_EVENT_JUMP;
    event.player_id = h->player_id;
    event.direction = direction;

    fire_hooks(h, event, ctrl);
}

void har_event_air_turn(har *h, controller *ctrl) {
    har_event event;
    event.type = HAR_EVENT_AIR_TURN;
    event.player_id = h->player_id;

    fire_hooks(h, event, ctrl);
}

void har_event_walk(har *h, int direction, controller *ctrl) {
    // direction is -1, 1, for backwards and forwards
    har_event event;
    memset(&event, 0, sizeof(event));
    event.type = HAR_EVENT_WALK;
    event.player_id = h->player_id;
    event.direction = direction;

    fire_hooks(h, event, ctrl);
}

void har_event_air_attack_done(har *h, controller *ctrl) {
    har_event event;
    memset(&event, 0, sizeof(event));
    event.type = HAR_EVENT_AIR_ATTACK_DONE;
    event.player_id = h->player_id;

    fire_hooks(h, event, ctrl);
}

void har_event_attack(har *h, af_move *move, controller *ctrl) {
    har_event event;
    memset(&event, 0, sizeof(event));
    event.type = HAR_EVENT_ATTACK;
    event.player_id = h->player_id;
    event.move = move;

    fire_hooks(h, event, ctrl);
}

void har_event_enemy_block(har *h, af_move *move, bool projectile, controller *ctrl) {
    har_event event;
    memset(&event, 0, sizeof(event));
    event.type = projectile ? HAR_EVENT_ENEMY_BLOCK_PROJECTILE : HAR_EVENT_ENEMY_BLOCK;
    event.type = HAR_EVENT_ENEMY_BLOCK;
    event.player_id = h->player_id;
    event.move = move;

    fire_hooks(h, event, ctrl);
}

void har_event_block(har *h, af_move *move, bool projectile, controller *ctrl) {
    har_event event;
    memset(&event, 0, sizeof(event));
    event.type = projectile ? HAR_EVENT_BLOCK_PROJECTILE : HAR_EVENT_BLOCK;
    event.player_id = h->player_id;
    event.move = move;

    fire_hooks(h, event, ctrl);
}

void har_event_take_hit(har *h, af_move *move, bool projectile, controller *ctrl) {
    har_event event;
    memset(&event, 0, sizeof(event));
    event.type = projectile ? HAR_EVENT_TAKE_HIT_PROJECTILE : HAR_EVENT_TAKE_HIT;
    event.player_id = h->player_id;
    event.move = move;

    fire_hooks(h, event, ctrl);
}

void har_event_land_hit(har *h, af_move *move, bool projectile, controller *ctrl) {
    har_event event;
    memset(&event, 0, sizeof(event));
    event.type = projectile ? HAR_EVENT_LAND_HIT_PROJECTILE : HAR_EVENT_LAND_HIT;
    event.player_id = h->player_id;
    event.move = move;

    fire_hooks(h, event, ctrl);
}

void har_event_hazard_hit(har *h, bk_info *info, controller *ctrl) {
    har_event event;
    memset(&event, 0, sizeof(event));
    event.type = HAR_EVENT_HAZARD_HIT;
    event.player_id = h->player_id;
    event.info = info;

    fire_hooks(h, event, ctrl);
}

void har_event_enemy_hazard_hit(har *h, controller *ctrl) {
    har_event event;
    memset(&event, 0, sizeof(event));
    event.type = HAR_EVENT_ENEMY_HAZARD_HIT;
    event.player_id = h->player_id;

    fire_hooks(h, event, ctrl);
}

void har_event_stun(har *h, controller *ctrl) {
    har_event event;
    memset(&event, 0, sizeof(event));
    event.type = HAR_EVENT_STUN;
    event.player_id = h->player_id;

    fire_hooks(h, event, ctrl);
}

void har_event_enemy_stun(har *h, controller *ctrl) {
    har_event event;
    memset(&event, 0, sizeof(event));
    event.type = HAR_EVENT_ENEMY_STUN;
    event.player_id = h->player_id;

    fire_hooks(h, event, ctrl);
}

void har_event_recover(har *h, controller *ctrl) {
    har_event event;
    memset(&event, 0, sizeof(event));
    event.type = HAR_EVENT_RECOVER;
    event.player_id = h->player_id;

    fire_hooks(h, event, ctrl);
}

void har_event_hit_wall(har *h, int wall, controller *ctrl) {
    har_event event;
    memset(&event, 0, sizeof(event));
    event.type = HAR_EVENT_HIT_WALL;
    event.player_id = h->player_id;
    event.wall = wall;

    fire_hooks(h, event, ctrl);
}

void har_event_land(har *h, controller *ctrl) {
    har_event event;
    memset(&event, 0, sizeof(event));
    event.type = HAR_EVENT_LAND;
    event.player_id = h->player_id;

    fire_hooks(h, event, ctrl);
}

void har_event_defeat(har *h, controller *ctrl) {
    har_event event;
    memset(&event, 0, sizeof(event));
    event.type = HAR_EVENT_DEFEAT;
    event.player_id = h->player_id;

    fire_hooks(h, event, ctrl);
}

void har_event_scrap(har *h, controller *ctrl) {
    har_event event;
    memset(&event, 0, sizeof(event));
    event.type = HAR_EVENT_SCRAP;
    event.player_id = h->player_id;

    fire_hooks(h, event, ctrl);
}

void har_event_destruction(har *h, controller *ctrl) {
    har_event event;
    memset(&event, 0, sizeof(event));
    event.type = HAR_EVENT_DESTRUCTION;
    event.player_id = h->player_id;

    fire_hooks(h, event, ctrl);
}

void har_event_done(har *h, controller *ctrl) {
    har_event event;
    memset(&event, 0, sizeof(event));
    event.type = HAR_EVENT_DONE;
    event.player_id = h->player_id;

    fire_hooks(h, event, ctrl);
}

void har_stunned_done(object *har_obj) {
    har *h = object_get_userdata(har_obj);

    if(h->state == STATE_STUNNED) {
        // refill endurance
        h->endurance = 0;
        h->state = STATE_STANDING;
        har_set_ani(har_obj, ANIM_IDLE, 1);
    }
}

// Simple helper function
void har_set_ani(object *obj, int animation_id, int repeat) {
    har *h = object_get_userdata(obj);
    af_move *move = af_get_move(h->af_data, animation_id);
    char *s = (char *)str_c(&move->move_string);
    // uint8_t has_corner_hack = obj->animation_state.shadow_corner_hack;
    object_set_animation(obj, &move->ani);
    // obj->animation_state.shadow_corner_hack = has_corner_hack;
    if(s != NULL && strcmp(s, "!") != 0 && strcmp(s, "0") != 0 && h->delay > 0) {
        log_debug("delaying move %d %s by %d ticks", move->id, s, h->delay);
        object_set_delay(obj, h->delay);
    }

    // we shouldn't be idling while defeated
    assert(!((animation_id == ANIM_IDLE || animation_id == ANIM_CROUCHING) && h->health <= 0));

    if(move->category == CAT_JUMPING) {
        h->state = STATE_JUMPING;
    }
    object_set_repeat(obj, repeat);
    object_set_stride(obj, 1);
    object_dynamic_tick(obj);
    // update this so mx/my have correct origins
    obj->start = obj->pos;
    h->damage_done = 0;
    h->damage_received = 0;
    h->executing_move = 0;
}

int har_is_active(object *obj) {
    har *h = object_get_userdata(obj);
    // during scrap/destruction, the defeated har should be rendered frontmost
    if(h->state == STATE_DEFEAT) {
        return 1;
    }
    if(h->state == STATE_SCRAP || h->state == STATE_DESTRUCTION || h->state == STATE_VICTORY) {
        return 0;
    }
    return h->executing_move;
}

void har_walk_to(object *obj, int destination) {
    har *h = object_get_userdata(obj);
    af_move *move = af_get_move(h->af_data, 10);

    h->walk_done_anim = obj->cur_animation->id;

    float vx = h->fwd_speed * object_get_direction(obj);
    log_debug("set velocity to %f", vx);
    object_set_vel(obj, vec2f_create(vx, 0));

    object_set_animation(obj, &move->ani);
    object_set_repeat(obj, 1);
    object_set_stride(obj, 1);
    h->walk_destination = destination;
    object_dynamic_tick(obj);
    // update this so mx/my have correct origins
    obj->start = obj->pos;
}

int har_is_walking(har *h) {
    if(h->state == STATE_WALKTO || h->state == STATE_WALKFROM) {
        return 1;
    }
    return 0;
}

int har_is_crouching(har *h) {
    if(h->state == STATE_CROUCHING || h->state == STATE_CROUCHBLOCK) {
        return 1;
    }
    return 0;
}

int har_is_blocking(har *h, af_move *move) {
    if(move->category == CAT_CLOSE) {
        // throws cannot be blocked
        return 0;
    }
    if(h->state == STATE_BLOCKSTUN) {
        return 1;
    }
    if(h->state == STATE_CROUCHBLOCK && move->category != CAT_JUMPING && h->executing_move == 0) {
        return 1;
    }
    if(h->state == STATE_WALKFROM && move->category != CAT_LOW && h->executing_move == 0) {
        return 1;
    }
    return 0;
}

bool is_in_range(object *obj, af_move *move) {
    if(move->successor_id) { // This is a throw with limited range
        // CLOSE moves use the successor id field as a distance requirement
        float throw_range = (float)obj->gs->match_settings.throw_range / 100.0f;
        har *h = object_get_userdata(obj);
        object *enemy_obj =
            game_state_find_object(obj->gs, game_player_get_har_obj_id(game_state_get_player(obj->gs, !h->player_id)));
        if(object_distance(obj, enemy_obj) > move->successor_id * throw_range) {
            return false;
        }
    }
    return true;
}

int har_is_invincible(object *obj, af_move *move) {
    if(player_frame_isset(obj, "zz")) {
        // blocks everything
        return 1;
    }

    switch(move->category) {
        case CAT_CLOSE:
            if(player_frame_isset(obj, "zg") || obj->cur_animation->id == ANIM_DAMAGE ||
               obj->cur_animation->id == ANIM_STANDING_BLOCK || obj->cur_animation->id == ANIM_CROUCHING_BLOCK) {
                return 1;
            }
            break;
        case CAT_LOW:
            if(player_frame_isset(obj, "zl")) {
                return 1;
            }
            break;
        case CAT_MEDIUM:
            if(player_frame_isset(obj, "zm")) {
                return 1;
            }
            break;
        case CAT_HIGH:
            if(player_frame_isset(obj, "zh")) {
                return 1;
            }
            break;
        case CAT_JUMPING:
            if(player_frame_isset(obj, "zj")) {
                return 1;
            }
            break;
        case CAT_PROJECTILE:
            if(player_frame_isset(obj, "zp")) {
                return 1;
            }
            break;
    }
    return 0;
}

// Callback for temporarily disabling an animation
void cb_har_disable_animation(object *parent, uint8_t animation_id, uint16_t ticks, void *userdata) {
    object *har_obj = userdata;
    har *h = object_get_userdata(har_obj);
    hashmap_put_int(&h->disabled_animations, (int)animation_id, &ticks, sizeof(ticks));
}

// Callback for spawning new objects, eg. projectiles
void cb_har_spawn_object(object *parent, int id, vec2i pos, vec2f vel, uint8_t mp_flags, int s, int g, void *userdata) {
    object *har_obj = userdata;
    har *h = object_get_userdata(har_obj);
    vec2i p_pos = object_get_pos(parent);

    // can't do this in object, because it hoses the intro
    if(pos.x == 0) {
        pos.x = p_pos.x; // + p_size.x / 2;
    }
    if(pos.y == 0) {
        pos.y = p_pos.y; // y + p_size.y / 2;
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
        object *obj = omf_calloc(1, sizeof(object));
        object_create(obj, parent->gs, pos, vel);
        object_set_stl(obj, object_get_stl(parent));
        object_set_animation(obj, &move->ani);
        object_set_gravity(obj, g / 100.0f);
        object_set_pal_offset(obj, object_get_pal_offset(parent));
        object_set_pal_limit(obj, object_get_pal_limit(parent));
        // Set all projectiles to their own layer + har layer
        object_set_layers(obj, LAYER_PROJECTILE | (h->player_id == 0 ? LAYER_HAR2 : LAYER_HAR1));
        // To avoid projectile-to-projectile collisions, set them to same group
        object_set_group(obj, GROUP_PROJECTILE);
        object_set_repeat(obj, 0);
        object_set_shadow(obj, 1);
        object_set_direction(obj, object_get_direction(parent));
        obj->animation_state.enemy_obj_id = parent->animation_state.enemy_obj_id;
        projectile_create(obj, parent);

        obj->animation_state.enemy_obj_id = parent->animation_state.enemy_obj_id;

        // allow projectiles to spawn projectiles, eg. shadow's scrap animation
        object_set_spawn_cb(obj, cb_har_spawn_object, har_obj);
        object_set_disable_cb(obj, cb_har_disable_animation, har_obj);

        // Handle Nova animation where the bot gets destroyed in single player
        if(h->id == 10 && id >= 25 && id <= 30) {
            projectile_set_wall_bounce(obj, 1);
            projectile_stop_on_ground(obj, 1);
        }

        if(h->state == STATE_SCRAP || h->state == STATE_DESTRUCTION) {
            // some scrap animations, like shadow's spawn projectiles that might collide with the wall
            // and we don't want them to disappear
            projectile_set_invincible(obj);
        }

        game_state_add_object(parent->gs, obj, RENDER_LAYER_MIDDLE, 0, 0);
    }
}

// Callback for destroying objects, eg. projectiles
void cb_har_destroy_object(object *parent, int animation_id, void *userdata) {
    object *har_obj = userdata;
    har *h = object_get_userdata(har_obj);

    // find all projectiles owned by us with the supplied animation id
    vector vec;
    vector_create(&vec, sizeof(object *));
    game_state_get_projectiles(parent->gs, &vec);

    object **p;
    iterator it;

    vector_iter_begin(&vec, &it);
    foreach(it, p) {
        if(projectile_get_owner(*p) == h->player_id && object_get_animation(*p)->id == animation_id) {
            game_state_del_object(parent->gs, *p);
        }
    }
    vector_free(&vec);
}

void har_floor_landing_effects(object *obj, bool play_sound) {
    int amount = rand_int(2) + 1;
    for(int i = 0; i < amount; i++) {
        int variance = rand_int(20) - 10;
        vec2i coord = vec2i_create(obj->pos.x + variance + i * 10, obj->pos.y);
        object *dust = omf_calloc(1, sizeof(object));
        object_create(dust, obj->gs, coord, vec2f_create(0, 0));
        object_set_stl(dust, object_get_stl(obj));
        object_set_animation(dust, &bk_get_info(game_state_get_scene(obj->gs)->bk_data, 26)->ani);
        game_state_add_object(obj->gs, dust, RENDER_LAYER_MIDDLE, 0, 0);
    }

    // Landing sound
    if(play_sound) {
        float pos_pan = ((float)obj->pos.x - 160.0f) / 160.0f;
        game_state_play_sound(obj->gs, 56, 0.3f, pos_pan, 2.2f);
    }
}

char get_last_input(har *har) {
    return har->inputs[0];
}

void har_move(object *obj) {
    har *h = object_get_userdata(obj);

    if(h->is_grabbed > 0 || h->in_stasis_ticks > 0) {
        return;
    }

    if(!player_frame_isset(obj, "h")) {
        obj->pos.x += obj->vel.x;
        obj->pos.y += obj->vel.y;
    }

    object *enemy_obj =
        game_state_find_object(obj->gs, game_player_get_har_obj_id(game_state_get_player(obj->gs, !h->player_id)));
    har *enemy_har = object_get_userdata(enemy_obj);

    if(h->walk_destination > 0 && h->walk_done_anim &&
       ((obj->pos.x >= h->walk_destination && object_get_direction(obj) == OBJECT_FACE_RIGHT) ||
        (obj->pos.x <= h->walk_destination && object_get_direction(obj) == OBJECT_FACE_LEFT))) {
        obj->pos.x = h->walk_destination;
        log_debug("reached destination!");
        if(obj->animation_state.shadow_corner_hack) {
            object_set_direction(obj, object_get_direction(obj) * -1);
        }

        object_set_vel(obj, vec2f_create(0, 0));
        har_set_ani(obj, h->walk_done_anim, 0);

        af_move *move = af_get_move(h->af_data, h->walk_done_anim);
        object_set_animation(enemy_obj, &af_get_move(enemy_har->af_data, ANIM_DAMAGE)->ani);
        object_set_repeat(enemy_obj, 0);
        object_set_custom_string(enemy_obj, str_c(&move->footer_string));
        object_dynamic_tick(enemy_obj);

        h->walk_destination = -1;
        h->walk_done_anim = 0;
        return;
    } else if(h->walk_destination > 0) {
        log_debug("still walking to %d, at %f", h->walk_destination, obj->pos.x);
    }

    // Check for wall hits
    if(obj->pos.x <= ARENA_LEFT_WALL || obj->pos.x >= ARENA_RIGHT_WALL) {
        h->is_wallhugging = 1;
        if(player_frame_isset(obj, "cw") && player_frame_isset(obj, "d")) {
            log_debug("disabling d tag on animation because of wall hit");
            obj->animation_state.disable_d = 1;
        }

    } else {
        h->is_wallhugging = 0;
    }

    // Handle floor collisions
    if(obj->pos.y >= ARENA_FLOOR) {
        controller *ctrl = game_player_get_ctrl(game_state_get_player(obj->gs, h->player_id));

        obj->pos.y = ARENA_FLOOR;

        char last_input = get_last_input(h);
        if(h->state == STATE_VICTORY && obj->vel.y > 0) {
            // TODO: A trigger in arena.c often puts us in STATE_VICTORY too early, which can be a problem
            // if we're still airborne.  This allows us to land somewhat properly.
            object_set_vel(obj, vec2f_create(0, 0));
            har_set_ani(obj, ANIM_IDLE, 1);
            object_set_stride(obj, h->stride);
            har_event_land(h, ctrl);
            har_floor_landing_effects(obj, true);
        } else if(h->state == STATE_JUMPING && enemy_har->is_grabbed == 0) {
            // Change animation from jump to walk or idle,
            // depending on held inputs
            if(last_input == '6') {
                h->state = STATE_WALKTO;
                har_set_ani(obj, ANIM_WALKING, 1);
                object_set_vel(obj, vec2f_create(0, 0));
                object_set_stride(obj, h->stride);
                har_event_walk(h, 1, ctrl);
            } else if(last_input == '4') {
                h->state = STATE_WALKFROM;
                har_set_ani(obj, ANIM_WALKING, 1);
                object_set_vel(obj, vec2f_create(0, 0));
                object_set_stride(obj, h->stride);
                har_event_walk(h, -1, ctrl);
            } else if(last_input == '7' || last_input == '8' || last_input == '9') {
                har_set_ani(obj, ANIM_JUMPING, 0);
                h->state = STATE_JUMPING;
                float vx = 0.0f;
                float vy = h->jump_speed;
                int jump_dir = 0;
                int direction = object_get_direction(obj);
                if(last_input == '9') {
                    vx = (h->fwd_speed * direction);
                    object_set_tick_pos(obj, 110);
                    object_set_stride(obj, 7); // Pass 7 frames per tick
                    jump_dir = 1;
                } else if(last_input == '7') {
                    // If we are jumping backwards, start animation from end
                    // at -100 frames (seems to be about right)
                    object_set_playback_direction(obj, PLAY_BACKWARDS);
                    object_set_tick_pos(obj, -110);
                    vx = (h->back_speed * direction * -1);
                    object_set_stride(obj, 7); // Pass 7 frames per tick
                    jump_dir = -1;
                } else {
                    // we are jumping upwards
                    object_set_tick_pos(obj, 110);
                    if(h->id == HAR_GARGOYLE) {
                        object_set_stride(obj, 7);
                    }
                }
                object_set_vel(obj, vec2f_create(vx, vy));
                har_event_jump(h, jump_dir, ctrl);
            } else {
                object_set_vel(obj, vec2f_create(0, 0));
                h->state = STATE_STANDING;
                har_set_ani(obj, ANIM_IDLE, 1);
                object_set_stride(obj, h->stride);
            }
            har_event_land(h, ctrl);
            har_floor_landing_effects(obj, true);
        } else if(h->state == STATE_RECOIL) {
            if(obj->vel.y > 0) {
                // bounce and screenshake if falling fast enough
                if(obj->vel.y > 6) {
                    har_floor_landing_effects(obj, false);
                    obj->vel.y = -3;
                    obj->vel.x = obj->vel.x / 2;
                    if(h->id != 10) {
                        object_set_custom_string(obj, "l20s4sp13zzN3-zzM100");
                        obj->gs->screen_shake_vertical = 5; // Multiplied by 5 to make it visible
                    } else {
                        // Nova falls harder
                        object_set_custom_string(obj, "l40s4sp13zzN3-zzM100");
                        obj->gs->screen_shake_vertical = 15; // Multiplied by 5 to make it visible
                    }
                } else {
                    obj->vel.y = 0;
                    obj->vel.x = 0;
                    har_event_land(h, ctrl);
                    har_finished(obj);
                }
            }

            if(obj->pos.x < ARENA_LEFT_WALL) {
                obj->pos.x = ARENA_LEFT_WALL;
            }
            if(obj->pos.x > ARENA_RIGHT_WALL) {
                obj->pos.x = ARENA_RIGHT_WALL;
            }
        }

        if(h->state != STATE_SCRAP) {
            // add some friction from the floor if we're not walking during scrap
            // This is important to dampen/eliminate the velocity added from pushing away from the other HAR
            // friction decreases velocity by 1 each tick, and sets it to 0 if its under |2|
            if(obj->vel.x > 0.0f) {
                if(obj->vel.x < 2.0f) {
                    obj->vel.x = 0.0f;
                } else {
                    obj->vel.x -= 1.0f;
                }
            } else if(obj->vel.x < 0.0f) {
                if(obj->vel.x > -2.0f) {
                    obj->vel.x = 0.0f;
                } else {
                    obj->vel.x += 1.0f;
                }
            }
        }

        if(h->state == STATE_WALKTO) {
            har_face_enemy(obj, enemy_obj);
            obj->pos.x += (h->fwd_speed * object_get_direction(obj)) * (h->hard_close ? 0.0 : 1.0);
        } else if(h->state == STATE_WALKFROM) {
            har_face_enemy(obj, enemy_obj);
            obj->pos.x -= (h->back_speed * object_get_direction(obj)) * (h->hard_close ? 0.5 : 1.0);
        }

        object_apply_controllable_velocity(obj, false, last_input);
    } else {
        obj->vel.y += obj->gravity;
        // Terminal Velocity
        if(obj->vel.y > 13) {
            obj->vel.y = 13;
        }
    }
}

void har_take_damage(object *obj, const str *string, float damage, float stun) {
    har *h = object_get_userdata(obj);

    if(h->state == STATE_VICTORY || h->state == STATE_DONE) {
        // can't die if the other guy died first
        return;
    }

    // Got hit, disable stasis activator on this bot
    h->in_stasis_ticks = 1;

    // Save damage taken
    h->last_damage_value = damage;

    h->last_stun_value = stun;

    // interrupted
    h->executing_move = 0;

    if(h->linked_obj) {
        object *linked = game_state_find_object(obj->gs, h->linked_obj);
        if(linked) {
            // end the animation of the linked object, so it can go to the successor
            linked->animation_state.finished = 1;
        }
    }

    if(vector_size(&h->child_objects)) {
        // shadow children need to die when the controlling HAR is hit
        iterator it;
        uint32_t *child_id;
        vector_iter_begin(&h->child_objects, &it);
        foreach(it, child_id) {
            object *child = game_state_find_object(obj->gs, *child_id);
            if(child) {
                int next_anim = af_get_move(h->af_data, child->cur_animation->id)->throw_duration;
                if(next_anim) {
                    object_set_animation(child, &af_get_move(h->af_data, next_anim)->ani);
                } else {
                    child->animation_state.finished = 1;
                }
            }
            vector_delete(&h->child_objects, &it);
        }
    }

    game_player *player = game_state_get_player(obj->gs, h->player_id);
    // If god mode is not on, take damage
    // Throw damage is delayed until after the throw ends
    if(!player->god && !h->throw_duration) {
        if(player->pilot->photo) {
            // in tournament mode, damage is mitigated by armor
            // (Armor + 2.5) * .25
            log_debug("applying %f to %d modulated by armor %f", damage, h->health,
                      0.25f * (2.5f + player->pilot->armor));
            h->health -= damage / (0.25f * (2.5f + player->pilot->armor));
        } else {
            h->health -= damage;
        }
    }

    // Handle health changes
    if(h->health <= 0) {
        h->health = 0;
    }

    if(!h->throw_duration) {
        int stun_amount = stun;
        if(h->state == STATE_RECOIL && object_is_airborne(obj)) { // Less stun on rehit and throws
            stun_amount /= 2;
        }
        stun_amount = (stun_amount * 2 + 12) * 256;
        log_debug("applying %f stun damage to %f", stun_amount, h->endurance);
        h->endurance += stun_amount;
    }

    if(h->endurance < 1.0f) {
        if(h->state == STATE_STUNNED) {
            // refill endurance
            h->endurance = 0;
        }
    } else if(h->endurance >= h->endurance_max) {
        // Calculate how much dizzy time we have based on the stun limit overage.
        // The more negative you go, the more time you're stunned.
        h->endurance = ((((h->endurance - h->endurance_max) / 256) * -2.5) - 60) * 256;
    }

    if(h->health == 0) {
        // Take a screencap of enemy har
        game_player *other_player = game_state_get_player(obj->gs, !h->player_id);
        object *other_har = game_state_find_object(obj->gs, other_player->har_obj_id);
        har_screencaps_capture(&other_player->screencaps, other_har, obj, SCREENCAP_BLOW);

        // Slow down game more for last shot
        log_debug("Slowdown: Slowing from %d to %d.", game_state_get_speed(obj->gs),
                  h->health == 0 ? game_state_get_speed(obj->gs) - 10 : game_state_get_speed(obj->gs) - 6);
        game_state_slowdown(obj->gs, 12,
                            h->health == 0 ? game_state_get_speed(obj->gs) - 10 : game_state_get_speed(obj->gs) - 6);
    } else {
        game_state_hit_pause(obj->gs);
    }

    str custom;
    str_create(&custom);

    // chronos' stasis does not have a hit animation
    if(str_size(string) > 0) {
        h->state = STATE_RECOIL;
        // Set hit animation
        object_set_animation(obj, &af_get_move(h->af_data, ANIM_DAMAGE)->ani);
        object_set_repeat(obj, 0);
        if(h->health <= 0) {
            controller *ctrl = game_player_get_ctrl(game_state_get_player(obj->gs, h->player_id));
            // trigger the defeat hook immediately
            har_event_defeat(h, ctrl);
        }

        if(h->throw_duration) {
            // No special handling
            object_set_stride(obj, 1);
        } else if(object_is_airborne(obj)) {
            log_debug("airborne knockback");
            // append the 'airborne knockback' string to the hit string, replacing the final frame
            size_t last_line = 0;
            if(!str_last_of(string, '-', &last_line)) {
                last_line = 0;
            }

            str_from_slice(&custom, string, 0, last_line);
            if(h->endurance < 0 || h->health <= 0) {
                // this hit stunned them, so make them hit the floor stunned
                str_append_c(&custom, "-L3-M5000");
            } else {
                str_append_c(&custom, "-L2-M5-L2");
            }

            obj->vel.y = obj->vertical_velocity_modifier * ((((30.0f - damage) * 0.133333f) + 6.5f) * -1.0);
            // TODO there's an alternative formula used in some conditions:
            // (((damage * 0.09523809523809523) + 3.5)  * -1) * obj->vertical_velocity_modifier
            // but we don't know what those conditions are
            obj->vel.x =
                (((damage * 0.16666666f) + 2.0f) * object_get_direction(obj) * -1) * obj->horizontal_velocity_modifier;
            object_set_stride(obj, 1);
        } else {
            if(h->health <= 0 || h->endurance < 0) {
                // taken from MASTER.DAT
                size_t last_line = 0;
                if(!str_last_of(string, '-', &last_line)) {
                    last_line = 0;
                }
                str_from_slice(&custom, string, 0, last_line);
                str_append_c(&custom, "-x-20ox-20L1-ox-20L2-x-20zzs4l25sp13M1-zzM2");
            }
        }

        if(str_size(&custom) > 0) {
            object_set_custom_string(obj, str_c(&custom));
            log_debug("HAR %s animation set to %s", har_get_name(h->id), str_c(&custom));
            str_free(&custom);
        } else {
            object_set_custom_string(obj, str_c(string));
            log_debug("HAR %s animation set to %s", har_get_name(h->id), str_c(string));
        }
        object_dynamic_tick(obj);

        // XXX hack - if the first frame has the 'k' tag, treat it as some vertical knockback
        // we can't do this in player.c because it breaks the jaguar leap, which also uses the 'k' tag.
        // Insanius 3/17/2025 - This is actually mostly correct, the OG checks if the string starts with the 'k' char
        const sd_script_frame *frame = sd_script_get_frame(&obj->animation_state.parser, 0);
        if(frame != NULL && sd_script_isset(frame, "k")) {
            obj->vel.x = -5 * object_get_direction(obj);
            obj->vel.y = -8;
        }
    }
}

// for scrap, nuts, bolts, and sparks (aka burning oil)
static vec2f har_debris_random_vel(object *har_obj, bool is_destruction) {
    vec2f vel;
    vel.x = rand_float() * 4.0f - 2.0f;
    vel.y = rand_float() * 4.0f - 2.0f;

    // FIXME: burning oil/sparks in destruction and scrap animations
    // FIXME: how does the original game handle scrap velocity in destruction anims?
    if(is_destruction) {
        // For scrap and destruction animations, make the scrap fly all over the place!
        vel.x *= 10.0f;
        vel.y *= 10.0f;
    } else {
        // push scrap and sparks upwards and away from the direction of impact
        vel.x -= object_get_direction(har_obj) * 5.1f;
        vel.y -= 3.0f;
    }

    return vel;
}

// gravity for sparks. scrap, nuts, and bolts seem to have different gravity
static inline float har_sparks_random_gravity(object *har_obj) {
    return (float)(rand_int(30) + 40) / 100.0;
}

static void har_spawn_oil(object *obj, vec2i pos, int amount, int layer) {
    har *h = object_get_userdata(obj);

    // burning oil
    for(int i = 0; i < amount; i++) {
        // Create the object
        object *scrap = omf_calloc(1, sizeof(object));
        int anim_no = ANIM_BURNING_OIL;
        object_create(scrap, obj->gs, pos, har_debris_random_vel(obj, false));
        object_set_animation(scrap, &af_get_move(h->af_data, anim_no)->ani);
        object_set_stl(scrap, object_get_stl(obj));
        object_set_gravity(scrap, har_sparks_random_gravity(obj));
        object_set_layers(scrap, LAYER_SCRAP);
        object_dynamic_tick(scrap);
        scrap_create(scrap);
        game_state_add_object(obj->gs, scrap, layer, 0, 0);
    }
}

// TODO: This is kind of a hack. It's used to check if either
// HAR is doing destruction. If there is any way to do this better,
// this should be changed.
int is_destruction(game_state *gs) {
    har *har_a = object_get_userdata(game_state_find_object(gs, game_state_get_player(gs, 0)->har_obj_id));
    har *har_b = object_get_userdata(game_state_find_object(gs, game_state_get_player(gs, 1)->har_obj_id));
    return (har_a->state == STATE_DESTRUCTION || har_b->state == STATE_DESTRUCTION);
}

void har_spawn_scrap(object *obj, vec2i pos, int amount) {
    // wild ass guess
    int oil_amount = amount / 3;
    har *h = object_get_userdata(obj);
    har_spawn_oil(obj, pos, oil_amount, RENDER_LAYER_TOP);

    // scrap metal
    // TODO this assumes the default scrap level and does not consider BIG[1-9]
    int scrap_amount = 0;
    if(amount > 11 && amount < 14) {
        scrap_amount = 1;
    } else if(amount > 13 && amount < 16) {
        scrap_amount = 2;
    } else if(amount > 15) {
        scrap_amount = 3;
    }
    for(int i = 0; i < scrap_amount; i++) {
        // Create the object
        object *scrap = omf_calloc(1, sizeof(object));
        int anim_no = rand_int(3) + ANIM_SCRAP_METAL;
        object_create(scrap, obj->gs, pos, har_debris_random_vel(obj, h->state == STATE_DEFEAT));
        object_set_animation(scrap, &af_get_move(h->af_data, anim_no)->ani);
        object_set_stl(scrap, object_get_stl(obj));
        object_set_gravity(scrap, 1.0f);
        object_set_pal_offset(scrap, object_get_pal_offset(obj));
        object_set_pal_limit(obj, object_get_pal_limit(obj));
        object_set_layers(scrap, LAYER_SCRAP);
        object_set_group(scrap, GROUP_SCRAP);
        object_dynamic_tick(scrap);
        object_set_shadow(scrap, 1);
        scrap_create(scrap);
        game_state_add_object(obj->gs, scrap, RENDER_LAYER_TOP, 0, 0);
    }
}

void har_block(object *obj, vec2i hit_coord, uint8_t block_stun) {
    har *h = obj->userdata;
    if(h->state == STATE_WALKFROM) {
        object_set_animation(obj, &af_get_move(h->af_data, ANIM_STANDING_BLOCK)->ani);
    } else if(h->state == STATE_BLOCKSTUN) {
        // restart the block animation
        object_set_animation(obj, obj->cur_animation);
    } else {
        object_set_animation(obj, &af_get_move(h->af_data, ANIM_CROUCHING_BLOCK)->ani);
    }
    // the HARs have a lame blank frame in their animation string, so use a custom one

    char stun_str[5];
    snprintf(stun_str, sizeof(stun_str), "A%d", block_stun);
    object_set_custom_string(obj, stun_str);
    object_set_repeat(obj, 0);
    object_dynamic_tick(obj);
    // blocking spark
    if(h->damage_received) {
        // don't make another scrape
        return;
    }
    h->state = STATE_BLOCKSTUN;
    game_state_hit_pause(obj->gs);
    object *scrape = omf_calloc(1, sizeof(object));
    object_create(scrape, obj->gs, hit_coord, vec2f_create(0, 0));
    object_set_animation(scrape, &af_get_move(h->af_data, ANIM_BLOCKING_SCRAPE)->ani);
    object_set_stl(scrape, object_get_stl(obj));
    object_set_direction(scrape, object_get_direction(obj));
    object_set_repeat(scrape, 0);
    object_set_gravity(scrape, 0);
    object_set_layers(scrape, LAYER_SCRAP);
    object_dynamic_tick(scrape);
    object_dynamic_tick(scrape);
    game_state_play_sound(obj->gs, 3, 0.7f, 0.5f, 1.0f);
    game_state_add_object(obj->gs, scrape, RENDER_LAYER_MIDDLE, 0, 0);
    h->damage_received = 1;
}

void har_land_on_har(object *obj_a, object *obj_b, int hard_limit, int soft_limit) {
    vec2i pos_a = object_get_pos(obj_a);
    vec2i pos_b = object_get_pos(obj_b);
    sprite *sprite_a = animation_get_sprite(obj_a->cur_animation, obj_a->cur_sprite_id);
    sprite *sprite_b = animation_get_sprite(obj_b->cur_animation, obj_b->cur_sprite_id);

    vec2i size_b = sprite_get_size(sprite_b);

    // the 50 here is to reverse the damage done in har_fix_sprite_coords
    int y1 = pos_a.y + sprite_a->pos.y + 50;
    int y2 = pos_b.y - size_b.y;

    // XXX make this code less redundanta
    if(object_get_direction(obj_a) == OBJECT_FACE_LEFT) {
        if(pos_a.x <= pos_b.x + hard_limit && pos_a.x >= pos_b.x && y1 >= y2) {
            if(pos_b.x == ARENA_LEFT_WALL) {
                pos_a.x = pos_b.x + hard_limit;
                object_set_pos(obj_a, pos_a);
            } else {
                // landed in front of the HAR, push opponent back
                int t = hard_limit - (pos_a.x - pos_b.x);
                pos_b.x = pos_b.x - t / 2;
                object_set_pos(obj_b, pos_b);
                pos_a.x = pos_a.x + t / 2;
                object_set_pos(obj_a, pos_a);
            }
        } else if(pos_a.x + hard_limit >= pos_b.x && pos_a.x <= pos_b.x && y1 >= y2) {
            if(pos_b.x == ARENA_LEFT_WALL) {
                pos_a.x = pos_b.x + hard_limit;
                object_set_pos(obj_a, pos_a);
            } else {
                // landed behind of the HAR, push opponent forwards
                int t = hard_limit - (pos_b.x - pos_a.x);
                pos_b.x = pos_b.x + t / 2;
                object_set_pos(obj_b, pos_b);
                pos_a.x = pos_a.x - t / 2;
                object_set_pos(obj_a, pos_a);
            }
        }
    } else if(object_get_direction(obj_a) == OBJECT_FACE_RIGHT) {
        if(pos_a.x + hard_limit >= pos_b.x && pos_a.x <= pos_b.x && y1 >= y2) {
            if(pos_b.x == ARENA_RIGHT_WALL) {
                pos_a.x = pos_b.x - hard_limit;
                object_set_pos(obj_a, pos_a);
            } else {
                // landed in front of the HAR, push opponent back
                int t = hard_limit - (pos_b.x - pos_a.x);
                pos_b.x = pos_b.x + t / 2;
                object_set_pos(obj_b, pos_b);
                pos_a.x = pos_a.x - t / 2;
                object_set_pos(obj_a, pos_a);
            }
        } else if(pos_a.x <= pos_b.x + hard_limit && pos_a.x >= pos_b.x && y1 >= y2) {
            if(pos_b.x == ARENA_RIGHT_WALL) {
                pos_a.x = pos_b.x - hard_limit;
                object_set_pos(obj_a, pos_a);
            } else {
                // landed behind of the HAR, push opponent forwards
                int t = hard_limit - (pos_b.x - pos_a.x);
                pos_b.x = pos_b.x - t / 2;
                object_set_pos(obj_b, pos_b);
                pos_a.x = pos_a.x + t / 2;
                object_set_pos(obj_a, pos_a);
            }
        }
    }
}

void har_check_closeness(object *obj_a, object *obj_b) {
    vec2i pos_a = object_get_pos(obj_a);
    vec2i pos_b = object_get_pos(obj_b);
    har *a = object_get_userdata(obj_a);
    har *b = object_get_userdata(obj_b);
    sprite *sprite_a = animation_get_sprite(obj_a->cur_animation, obj_a->cur_sprite_id);
    sprite *sprite_b = animation_get_sprite(obj_b->cur_animation, obj_b->cur_sprite_id);
    int hard_limit = 32; // Push opponent if HARs too close. Harrison-Stetson method value.
    // TODO verify throw distance soft limit
    int soft_limit = 45; // Sets HAR A as being close to HAR B if closer than this. This should be affected by throw
                         // distance setting.

    // Reset closeness state
    a->close = 0;
    b->close = 0;
    a->hard_close = 0;
    b->hard_close = 0;

    if(!sprite_a || !sprite_b) {
        // no sprite, eg chronos' teleport
        return;
    }

    if(object_is_airborne(obj_a) && object_is_airborne(obj_b)) {
        // HARs clip through each other in the air
        return;
    }

    if(a->throw_duration || b->throw_duration) {
        // don't mess with coreography
        return;
    }

    if(a->health <= 0 || b->health <= 0) {
        return;
    }

    // handle one HAR landing on top of another
    if((a->state == STATE_JUMPING || a->state == STATE_RECOIL) && b->state != STATE_JUMPING &&
       b->state != STATE_RECOIL) {
        har_land_on_har(obj_a, obj_b, hard_limit, soft_limit);
        return;
    } else if((b->state == STATE_JUMPING || b->state == STATE_RECOIL) && a->state != STATE_JUMPING &&
              a->state != STATE_RECOIL) {
        har_land_on_har(obj_b, obj_a, hard_limit, soft_limit);
        return;
    }

    if(abs(pos_a.x - pos_b.x) <= hard_limit) {
        a->hard_close = 1;
        b->hard_close = 1;
    }

    float a_speed = (a->fwd_speed * object_get_direction(obj_a)) * (a->hard_close ? 0.5 : 1.0);
    float b_speed = (b->fwd_speed * object_get_direction(obj_b)) * (b->hard_close ? 0.5 : 1.0);

    bool pushed = true;
    // If HARs get too close together, handle it
    if(a->state == STATE_WALKTO && b->state == STATE_WALKTO && a->hard_close) {
        // both hars are walking into each other, figure out the resulting vector and apply it
        obj_b->pos.x += b_speed + a_speed;
        obj_a->pos.x += a_speed + b_speed;
    } else if(a->state == STATE_WALKTO && a->hard_close) {
        // A pushes B
        obj_b->pos.x += a_speed;
    } else if(b->state == STATE_WALKTO && b->hard_close) {
        // B pushes A
        obj_a->pos.x += b_speed;
    } else {
        pushed = false;
    }

    if(fabsf(obj_a->pos.x - obj_b->pos.x) < hard_limit && a->state != STATE_JUMPING && b->state != STATE_JUMPING) {
        if(obj_a->pos.x <= ARENA_LEFT_WALL) {
            obj_a->pos.x = ARENA_LEFT_WALL;
            obj_b->pos.x = ARENA_LEFT_WALL + hard_limit;
        } else if(obj_b->pos.x <= ARENA_LEFT_WALL) {
            obj_b->pos.x = ARENA_LEFT_WALL;
            obj_a->pos.x = ARENA_LEFT_WALL + hard_limit;
        } else if(obj_a->pos.x >= ARENA_RIGHT_WALL) {
            obj_a->pos.x = ARENA_RIGHT_WALL;
            obj_b->pos.x = ARENA_RIGHT_WALL - hard_limit;
        } else if(obj_b->pos.x >= ARENA_RIGHT_WALL) {
            obj_b->pos.x = ARENA_RIGHT_WALL;
            obj_a->pos.x = ARENA_RIGHT_WALL - hard_limit;
        } else if(!pushed) {
            // they're simply too close
            float distance = hard_limit - fabsf(obj_a->pos.x - obj_b->pos.x);
            if(obj_a->pos.x < obj_b->pos.x) {
                obj_a->pos.x -= distance / 2.0;
                obj_a->pos.x = clampf(obj_a->pos.x, ARENA_LEFT_WALL, ARENA_RIGHT_WALL);
                obj_b->pos.x = obj_a->pos.x + hard_limit;
            } else {
                obj_a->pos.x += distance / 2.0;
                obj_a->pos.x = clampf(obj_a->pos.x, ARENA_LEFT_WALL, ARENA_RIGHT_WALL);
                obj_b->pos.x = obj_a->pos.x - hard_limit;
            }
        }
    }

    // track if the hars are "close"
    // TODO defensive throws setting may impact this
    if(a->state == STATE_WALKTO && abs(pos_a.x - pos_b.x) < soft_limit) {
        if(b->state == STATE_STANDING || b->state == STATE_STUNNED || har_is_walking(b) || har_is_crouching(b)) {
            a->close = 1;
        }
    }
    if(b->state == STATE_WALKTO && abs(pos_a.x - pos_b.x) < soft_limit) {
        if(a->state == STATE_STANDING || a->state == STATE_STUNNED || har_is_walking(a) || har_is_crouching(a)) {
            b->close = 1;
        }
    }
}

#ifdef DEBUGMODE
void har_debug(object *obj) {
    har *h = object_get_userdata(obj);
    if(obj->cur_sprite_id < 0) {
        return;
    }
    // Some useful variables
    vec2i pos_a = object_get_pos(obj); //, obj->cur_sprite->pos);
    // vec2i size_a = object_get_size(obj);

    int flip = 1;

    if(object_get_direction(obj) == OBJECT_FACE_LEFT) {
        // pos_a.x = object_get_pos(obj).x + ((obj->cur_sprite->pos.x * -1) - size_a.x);
        flip = -1;
    }

    int flip_mode = obj->sprite_state.flipmode;
    if(object_get_direction(obj) == OBJECT_FACE_LEFT) {
        flip_mode ^= FLIP_HORIZONTAL;
    }

    video_draw_full(&h->har_origin, pos_a.x - 2, pos_a.y - 2, 4, 4, 0, 0, 0, 255, 255, flip_mode, 0);

    // Make sure there are hitpoints to check.
    if(vector_size(&obj->cur_animation->collision_coords) == 0) {
        return;
    }

    // Iterate through hitpoints
    iterator it;
    collision_coord *cc;
    vector_iter_begin(&obj->cur_animation->collision_coords, &it);

    int found = 0;
    foreach(it, cc) {
        if(cc->frame_index != obj->cur_sprite_id)
            continue;
        found = 1;
    }

    if(!found) {
        return;
    }

    vector_iter_begin(&obj->cur_animation->collision_coords, &it);
    foreach(it, cc) {
        if(cc->frame_index != obj->cur_sprite_id)
            continue;
        video_draw(&h->hit_pixel, pos_a.x + (cc->pos.x * flip), pos_a.y + cc->pos.y);
        /*log_debug("%d drawing hit point at %d %d ->%d %d", obj->cur_sprite_id, pos_a.x, pos_a.y, pos_a.x +
         (cc->pos.x * flip), pos_a.y + cc->pos.y);*/
    }
}
#endif // DEBUGMODE

// function to check if har A is hitting har B. Returns 1 if the har is executing a priority move which
// would interrupt B. Currently only throws are considered priority.
int har_collide_with_har(object *obj_a, object *obj_b, int loop) {
    har *a = object_get_userdata(obj_a);
    har *b = object_get_userdata(obj_b);

    controller *ctrl_a = game_player_get_ctrl(game_state_get_player(obj_a->gs, a->player_id));
    controller *ctrl_b = game_player_get_ctrl(game_state_get_player(obj_b->gs, b->player_id));

    if(b->state == STATE_WALLDAMAGE || b->state >= STATE_VICTORY || b->state == STATE_STANDING_UP) {
        // can't hit em while they're down
        return 0;
    }

    if(a->in_stasis_ticks) {
        // frozen HARs can't hit
        return 0;
    }

    // rehit mode is off
    if(!obj_b->gs->match_settings.rehit && (b->state == STATE_RECOIL && object_is_airborne(obj_b))) {
        return 0;
    }

    // rehit mode is on, but the opponent isn't airborne or stunned
    if(obj_b->gs->match_settings.rehit && b->state == STATE_RECOIL &&
       (!object_is_airborne(obj_b) || b->endurance < 0)) {
        log_debug("REHIT is not possible %d %f %f %f", object_is_airborne(obj_b), obj_b->pos.x, obj_b->pos.y,
                  b->endurance);
        return 0;
    }

    bool rehit =
        obj_b->gs->match_settings.rehit && b->state == STATE_RECOIL && object_is_airborne(obj_b) && b->endurance >= 0;

    // Check for collisions by sprite collision points
    int level = 1;
    af_move *move = af_get_move(a->af_data, obj_a->cur_animation->id);

    // is the HAR invulnerable to this kind of attack?
    if(har_is_invincible(obj_b, move)) {
        return 0;
    }

    if(!is_in_range(obj_b, move)) { // Won't get hit by throws out of range
        return 0;
    }

    // check this mode hasn't already rehit
    if(rehit && strchr(b->rehits, move->id)) {
        log_debug("move %d has already done a rehit");
        return 0;
    }

    vec2i hit_coord = vec2i_create(0, 0);
    if(obj_a->can_hit) {
        a->damage_done = 0;
        obj_a->can_hit = 0;
    }
    if(a->damage_done == 0 &&
       (intersect_sprite_hitpoint(obj_a, obj_b, level, &hit_coord) || move->category == CAT_CLOSE ||
        (player_frame_isset(obj_a, "ue") && b->state != STATE_JUMPING))) {

        obj_a->q_counter = obj_a->q_val;

        if(har_is_blocking(b, move) &&
           // earthquake smash is unblockable
           !player_frame_isset(obj_a, "ue")) {
            a->damage_done = 1;
            har_event_enemy_block(a, move, false, ctrl_a);
            har_event_block(b, move, false, ctrl_b);
            har_block(obj_b, hit_coord, move->block_stun);
            if(player_frame_isset(obj_a, "i") && move->next_move) {
                har_set_ani(obj_a, move->next_move, 0);
            }
            if(b->is_wallhugging) {
                vec2f push = object_get_vel(obj_a);
                // TODO use 90% of the block pushback as cornerpush for now
                push.x = -1 * object_get_direction(obj_a) * (((move->block_stun - 2) * 0.74) + 1) * 0.9;
                log_debug("doing block cornerpush of %f",
                          -1 * object_get_direction(obj_a) * (((move->block_stun - 2) * 0.74) + 1) * 0.9);
                object_set_vel(obj_a, push);
            } else {
                vec2f push = object_get_vel(obj_b);
                push.x = -1 * object_get_direction(obj_b) * (((move->block_stun - 2) * 0.74) + 1);
                log_debug("doing block pushback of %f",
                          -1 * object_get_direction(obj_b) * (((move->block_stun - 2) * 0.74) + 1));
                object_set_vel(obj_b, push);
            }
            return 0;
        }

        vec2i hit_coord2 = vec2i_create(0, 0);

        if(move->category != CAT_CLOSE && b->damage_done == 0 && loop == 0 &&
           intersect_sprite_hitpoint(obj_b, obj_a, level, &hit_coord2)) {
            log_debug("both hars hit at the same time!");
            if(har_collide_with_har(obj_b, obj_a, 1)) {
                // other player threw us
                return 0;
            }
            // check if they are still alive
            if(b->state == STATE_RECOIL || b->state == STATE_STANDING_UP || b->state == STATE_WALLDAMAGE ||
               b->health <= 0 || b->state >= STATE_VICTORY) {
                // can't hit em while they're down
                return 0;
            }
        }

        if(move->category == CAT_CLOSE) {
            a->close = 0;
        }

        if((b->state == STATE_STUNNED || b->state == STATE_RECOIL) &&
           object_get_direction(obj_a) == object_get_direction(obj_b)) {
            // opponent is stunned and facing the same way we are, backwards
            // so flip them around
            object_set_direction(obj_b, object_get_direction(obj_a) * -1);
        }

        har_event_take_hit(b, move, false, ctrl_b);
        har_event_land_hit(a, move, false, ctrl_a);

        if(move->category != CAT_CLOSE) {
            if(b->is_wallhugging) {
                // back the attacker off a little
                vec2f push = object_get_vel(obj_a);
                if(fabsf(push.x) < 5.5f) {
                    // TODO need real formula here
                    log_debug("doing corner push of 6.3");
                    push.x = -6.3f * object_get_direction(obj_a);
                    object_set_vel(obj_a, push);
                }
            } else {
                vec2f push = object_get_vel(obj_b);
                if(fabsf(push.x) < 7.0f) {
                    log_debug("doing knockback of 7");
                    push.x = -7.0f * object_get_direction(obj_b);
                    object_set_vel(obj_b, push);
                }
            }
        }

        // rehits only do 60% damage
        int damage = rehit ? move->damage * 0.6 : move->damage;

        if(object_is_airborne(obj_a) && object_is_airborne(obj_b)) {
            // modify the horizontal velocity of the attacker when doing air knockback
            obj_a->vel.x *= 0.7f;
            // the opponent's velocity is modified in har_take_damage
        }

        b->throw_duration = move->throw_duration;

        log_debug("HAR %s to HAR %s collision at %d,%d!", har_get_name(a->id), har_get_name(b->id), hit_coord.x,
                  hit_coord.y);

        // face B to the direction they're being attacked from
        object_set_direction(obj_b, -object_get_direction(obj_a));

        if(player_frame_isset(obj_a, "ai")) {
            str str;
            str_from_c(&str, "A1-s01l50B2-C2-L5-M400");
            har_take_damage(obj_b, &str, damage, move->stun);
            str_free(&str);
            obj_b->vel.x = -5.0 * object_get_direction(obj_b);
            obj_b->vel.y = -9.0;
        } else {
            har_take_damage(obj_b, &move->footer_string, damage, move->stun);
        }

        if(rehit) {
            obj_b->vel.y -= 3;
            b->rehits[strlen(b->rehits)] = move->id;
        } else {
            memset(b->rehits, 0, sizeof(b->rehits));
        }

        if((hit_coord.x != 0 || hit_coord.y != 0) && damage != 0) {
            har_spawn_scrap(obj_b, hit_coord, move->block_stun);
        }

        if(move->next_move) {
            af_move *next_move = af_get_move(a->af_data, move->next_move);
            if(str_size(&move->footer_string) == 0 && b->health == 0 && next_move->damage > 0) {
                // chained move like thorn's spike charge
                // we want to keep the opponent alive until the next thing hits
                b->health = 1;
            }
            log_debug("HAR %s going to next move %d", har_get_name(b->id), move->next_move);

            har_set_ani(obj_a, move->next_move, 0);

            // prevent next move from being interrupted
            a->executing_move = 1;

            int retval = move->category == CAT_CLOSE ? 1 : 0;

            if(loop == 0) {
                // recurse so jaguar's overhead throw grabs enemy on first tick of anim 22
                retval = har_collide_with_har(obj_a, obj_b, 1) || retval;
            }

            // bail out early, the next move can still brutalize the oppopent so don't set them immune to further damage
            // this fixes flail's charging punch and katana's wall spin, but thorn's spike charge still works
            //
            // but still return if we had priority
            return retval;
        }

        a->damage_done = 1;
        b->damage_received = 1;

        // return if this move had priority
        return move->category == CAT_CLOSE ? 1 : 0;
    }

    return 0;
}

void har_collide_with_projectile(object *o_har, object *o_pjt) {
    har *h = object_get_userdata(o_har);
    const af *prog_owner_af_data = projectile_get_af_data(o_pjt);
    har *other = object_get_userdata(
        game_state_find_object(o_har->gs, game_state_get_player(o_har->gs, abs(h->player_id - 1))->har_obj_id));

    if(h->state == STATE_STANDING_UP || h->state == STATE_WALLDAMAGE || h->state >= STATE_VICTORY) {
        // can't hit em while they're down, or done
        return;
    }

    // rehit mode is off
    if(!o_har->gs->match_settings.rehit && h->state == STATE_RECOIL) {
        log_debug("REHIT is off");
        return;
    }

    // rehit mode is on, but the opponent isn't airborne or stunned
    if(o_har->gs->match_settings.rehit && h->state == STATE_RECOIL &&
       (!object_is_airborne(o_har) || h->endurance < 0)) {
        log_debug("REHIT is not possible %d %f %f %f", object_is_airborne(o_har), o_har->pos.x, o_har->pos.y,
                  h->endurance);
        return;
    }

    bool rehit =
        o_har->gs->match_settings.rehit && h->state == STATE_RECOIL && object_is_airborne(o_har) && h->endurance >= 0;

    // Check for collisions by sprite collision points
    int level = 2;
    vec2i hit_coord;
    if(intersect_sprite_hitpoint(o_pjt, o_har, level, &hit_coord)) {
        af_move *move = af_get_move(prog_owner_af_data, o_pjt->cur_animation->id);

        controller *ctrl = game_player_get_ctrl(game_state_get_player(o_har->gs, h->player_id));
        controller *ctrl_other = game_player_get_ctrl(game_state_get_player(o_pjt->gs, other->player_id));
        if(har_is_blocking(h, move)) {
            projectile_mark_hit(o_pjt); // prevent this projectile from hitting again
            o_pjt->animation_state.finished = 1;
            if(move->successor_id && move->category != CAT_CLOSE) {
                af_move *next_move = af_get_move(prog_owner_af_data, move->successor_id);
                object_set_animation(o_pjt, &next_move->ani);
                object_set_repeat(o_pjt, 0);
                o_pjt->animation_state.finished = 0;
            }
            har_event_enemy_block(other, move, true, ctrl_other);
            har_event_block(h, move, true, ctrl);
            // Clear damage received flag, as projectiles always trigger a block.
            // This is especially true for electra's shards.
            h->damage_received = 0;
            har_block(o_har, hit_coord, move->block_stun);
            // do block pushback
            vec2f push = object_get_vel(o_har);
            push.x = -1 * object_get_direction(o_har) * (((move->block_stun - 2) * 0.74) + 1);
            log_debug("doing block pushback of %f",
                      -1 * object_get_direction(o_har) * (((move->block_stun - 2) * 0.74) + 1));
            object_set_vel(o_har, push);
            return;
        }

        // is the HAR invulnerable to this kind of attack?
        if(har_is_invincible(o_har, move)) {
            return;
        }

        if(projectile_did_hit(o_pjt)) {
            // projectile has already hit
            return;
        }

        // check the animation is still going
        // for some reason this has been observed to happen sometimes, an example is frame 18 of chronos' stasis
        if(!sd_script_get_frame_at(&o_pjt->animation_state.parser, o_pjt->animation_state.current_tick)) {
            log_debug("no such frame at tick %d", o_pjt->animation_state.current_tick);
            return;
        }

        log_debug("PROJECTILE %d to HAR %s collision at %d,%d!", object_get_animation(o_pjt)->id, har_get_name(h->id),
                  hit_coord.x, hit_coord.y);

        if(move->next_move) {
            log_debug("PROJECTILE %d going to next move %d on HIT", object_get_animation(o_pjt)->id, move->next_move);
            object_set_animation(o_pjt, &af_get_move(prog_owner_af_data, move->next_move)->ani);
            object_set_repeat(o_pjt, 0);
            return;
        }

        // Exception case for chronos' time freeze
        if(player_frame_isset(o_pjt, "af")) {
            // statis ticks is the raw damage from the move
            h->in_stasis_ticks = move->raw_damage;
        } else {
            if(move->damage > 0) {
                // assume all projectile that do damage have a footer string
                assert(str_size(&move->footer_string) > 0);
            }
            // Just take damage normally if there is no footer string in successor
            log_debug("projectile dealt damage of %f", move->damage);
            log_debug("projectile %d dealt damage of %f", move->id, move->damage);

            // face B to the direction they're being attacked from
            object_set_direction(o_har, -object_get_direction(o_pjt));

            int damage = rehit ? move->damage * 0.6 : move->damage;
            if(player_frame_isset(o_pjt, "ai")) {
                str str;
                str_from_c(&str, "A1-s01l50B2-C2-L5-M400");
                har_take_damage(o_har, &str, damage, move->stun);
                str_free(&str);
                o_har->vel.x = -5.0 * object_get_direction(o_har);
                o_har->vel.y = -9.0;
            } else {
                har_take_damage(o_har, &move->footer_string, damage, move->stun);
                if(rehit) {
                    o_har->vel.y -= 3;
                }
                if(!h->is_wallhugging && !object_is_airborne(o_har)) {
                    vec2f push = object_get_vel(o_har);
                    if(fabsf(push.x) < 7.0f) {
                        log_debug("doing knockback of 7");
                        push.x = -7.0f * object_get_direction(o_har);
                        object_set_vel(o_har, push);
                    }
                }
            }
            // shadow grab is a projectile
            h->throw_duration = move->throw_duration;
        }

        projectile_mark_hit(o_pjt);

        har_event_take_hit(h, move, true, ctrl);
        har_event_land_hit(other, move, true, ctrl_other);

        har_spawn_scrap(o_har, hit_coord, move->block_stun);
        h->damage_received = 1;

        if(player_frame_isset(o_pjt, "uz")) {
            // associate this with the enemy HAR
            h->linked_obj = o_pjt->id;
            projectile_link_object(o_pjt, o_har);
        }

        // Switch to successor animation if one exists for this projectile
        // CAT CLOSE is a check for shadow grab, but there's probably a special flag
        if(move->successor_id && move->category != CAT_CLOSE) {
            af_move *next_move = af_get_move(prog_owner_af_data, move->successor_id);
            object_set_animation(o_pjt, &next_move->ani);
            object_set_repeat(o_pjt, 0);
            o_pjt->animation_state.finished = 0;
            projectile_clear_hit(o_pjt);
            log_debug("SUCCESSOR: Selecting anim %d with string %s", object_get_animation(o_pjt)->id,
                      str_c(&object_get_animation(o_pjt)->animation_string));
        }
    }
}

void har_collide_with_hazard(object *o_har, object *o_hzd) {
    har *h = object_get_userdata(o_har);
    bk *bk_data = object_get_userdata(o_hzd);
    bk_info *anim = bk_get_info(bk_data, o_hzd->cur_animation->id);

    if(h->state == STATE_STANDING_UP) {
        // can't hit em while they're down
        return;
    }

    if(h->state == STATE_VICTORY || h->state == STATE_DEFEAT || h->state == STATE_SCRAP ||
       h->state == STATE_DESTRUCTION || h->state == STATE_DONE || h->state == STATE_WALLDAMAGE) {
        // Hazards should not affect HARs at the end of a match
        return;
    }

    // Check if collisions are switched off for the hazard
    if(player_frame_isset(o_hzd, "n")) {
        return;
    }

    // Check for collisions by sprite collision points
    int level = 2;
    vec2i hit_coord;
    if(!h->damage_received && intersect_sprite_hitpoint(o_hzd, o_har, level, &hit_coord)) {
        har_take_damage(o_har, &anim->footer_string, anim->hazard_damage, anim->hazard_damage);
        controller *ctrl = game_player_get_ctrl(game_state_get_player(o_har->gs, h->player_id));
        har_event_hazard_hit(h, anim, ctrl);
        // fire enemy hazard hit event
        object *enemy_obj = game_state_find_object(
            o_har->gs, game_player_get_har_obj_id(game_state_get_player(o_har->gs, !h->player_id)));
        har *enemy_h = object_get_userdata(enemy_obj);
        controller *enemy_ctrl = game_player_get_ctrl(game_state_get_player(enemy_obj->gs, enemy_h->player_id));
        har_event_enemy_hazard_hit(enemy_h, enemy_ctrl);
        if(anim->chain_no_hit) {
            object_set_animation(o_hzd, &bk_get_info(bk_data, anim->chain_no_hit)->ani);
            object_set_repeat(o_hzd, 0);
        }
        har_spawn_scrap(o_har, hit_coord, 9);
        h->damage_received = 1;
    } else if(anim->chain_hit && intersect_sprite_hitpoint(o_har, o_hzd, level, &hit_coord)) {
        // we can punch this! Only set on fire pit orb
        anim = bk_get_info(bk_data, anim->chain_hit);
        o_hzd->animation_state.enemy_obj_id = o_har->animation_state.enemy_obj_id;
        object_set_animation(o_hzd, &anim->ani);
        object_set_repeat(o_hzd, 0);
        o_hzd->animation_state.finished = 0;
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

    // Check if this is hazard to har collision
    if(object_get_layers(obj_a) & LAYER_HAZARD) {
        har_collide_with_hazard(obj_b, obj_a);
        return;
    }
    if(object_get_layers(obj_b) & LAYER_HAZARD) {
        har_collide_with_hazard(obj_a, obj_b);
        return;
    }

    // Check for closeness between HARs and handle it
    har_check_closeness(obj_a, obj_b);

    // Handle har collisions
    if(!har_collide_with_har(obj_a, obj_b, 0)) {
        // if A didn't hit with a priority move, check B
        har_collide_with_har(obj_b, obj_a, 0);
    }
}

static void process_range(const har *h, damage_tracker *damage, vga_palette *pal, uint8_t step) {
    // For player 0, we should use palette indexes 1 through 47. For player 1, 49 through 95 (skip black).
    // If pe flag is on, we need to switch to handling the other HAR.
    const int start = 48 * (h->player_id ^ h->p_har_switch) + 1;
    const int end = start + 47;
    if(h->p_color_fn) {
        vga_palette_tint_range(pal, h->p_pal_ref, start, end, step);
    } else {
        vga_palette_mix_range(pal, h->p_pal_ref, start, end, step);
    }
    damage_add_range(damage, start, end);
}

static void har_palette_transform(damage_tracker *damage, vga_palette *pal, void *userdata) {
    object *obj = userdata;
    const har *h = object_get_userdata(obj);

    if(object_has_effect(obj, EFFECT_POSITIONAL_LIGHTING)) {
        int const mid = NATIVE_W / 2;
        int x_dist = abs(mid - object_px(obj));
        int y_dist = ARENA_FLOOR - object_py(obj);
        int32_t blend_factor = (mid - x_dist - y_dist) / 2 - 30;

        blend_factor = max2(blend_factor, -41);
        uint8_t gray = blend_factor < 1 ? 1 : 53;
        blend_factor = abs(blend_factor);

        // convert from original game's 6 bit palette to 8 bit
        gray *= 4;
        blend_factor *= 4;

        const int start = 48 * (h->player_id ^ h->p_har_switch) + 1;
        const int end = start + 47;

        vga_palette_light_range(pal, gray, start, end, blend_factor);
        damage_add_range(damage, start, end);
    }

    float step;
    if(h->p_fade_in_ticks_left > 0) {
        step = 1.0f - h->p_fade_in_ticks_left / (float)h->p_fade_in_ticks;
        process_range(h, damage, pal, clamp(step * 255, 0, 255));
    } else if(h->p_sustain_ticks_left > 0) {
        process_range(h, damage, pal, 255);
    } else if(h->p_fade_out_ticks_left > 0) {
        step = h->p_fade_out_ticks_left / (float)h->p_fade_out_ticks;
        process_range(h, damage, pal, clamp(step * 255, 0, 255));
    }
}

void har_handle_stun(object *obj) {
    har *h = object_get_userdata(obj);

    if(h->health <= 0) { // Lock the stun meter to near-empty when KO'd
        h->endurance = h->endurance_max - 2;
    } else {
        if(h->endurance < 1) { // We are currently dizzy, recover gradually
            if(obj->cur_animation->id == ANIM_STUNNED) {
                h->endurance += h->stun_factor * 1.4;
            }
        } else { // Not yet stunned, handle regular stun recovery
            float stunfactor = 1.0 * h->stun_factor * h->endurance / h->endurance_max / 256.0;
            if((obj->cur_animation->id == ANIM_IDLE) || (obj->cur_animation->id == ANIM_CROUCHING) ||
               (obj->cur_animation->id == ANIM_VICTORY)) {
                float hpfactor = ((h->health_max * 5.0 / 9.0) + h->health) / h->health_max;

                h->endurance -= ((hpfactor * stunfactor) + STUN_RECOVERY_CONSTANT) * 256;
            } else if((obj->cur_animation->id == ANIM_CROUCHING_BLOCK) ||
                      (obj->cur_animation->id == ANIM_STANDING_BLOCK)) {
                float hpfactor = (h->health_max * 25.0 / 27.0 + h->health) / h->health_max;

                h->endurance -= ((hpfactor * stunfactor) + STUN_RECOVERY_BLOCKING_CONSTANT) * 256;
            } else if(obj->cur_animation->id == ANIM_JUMPING) {
                float hpfactor = ((h->health_max * 25.0 / 81.0) + h->health) / h->health_max;

                h->endurance -= ((hpfactor * stunfactor) + STUN_RECOVERY_CONSTANT) * 256;
            } else if(obj->cur_animation->id < (ANIM_SCREW | ANIM_JUMPING)) { // Cover all other cases
                float hpfactor = ((h->health_max * 5.0 / 27.0) + h->health) / h->health_max;

                h->endurance -= ((hpfactor * stunfactor) + STUN_RECOVERY_CONSTANT) * 256;
            }
            if(h->endurance < 0) {
                h->endurance = 0;
            }
        }
    }

    if(h->state == STATE_STUNNED) {
        h->stun_timer++;
        if(h->stun_timer % 10 == 0) {
            vec2i pos = object_get_pos(obj);
            pos.y -= 60;
            har_spawn_oil(obj, pos, 5, RENDER_LAYER_BOTTOM);
        }

        if(h->endurance >= 0) {
            har_stunned_done(obj);
        }
    }
}

void har_tick(object *obj) {
    har *h = object_get_userdata(obj);
    controller *ctrl = game_player_get_ctrl(game_state_get_player(obj->gs, h->player_id));

    if(object_has_effect(obj, EFFECT_POSITIONAL_LIGHTING) || h->p_fade_in_ticks_left > 0 ||
       h->p_fade_out_ticks_left > 0 || h->p_sustain_ticks_left > 0) {
        object_set_palette_transform_cb(obj, har_palette_transform);
    } else {
        object_set_palette_transform_cb(obj, NULL);
    }

    if(h->p_fade_in_ticks_left > 0)
        h->p_fade_in_ticks_left--;
    if(h->p_sustain_ticks_left > 0)
        h->p_sustain_ticks_left--;
    if(h->p_fade_out_ticks_left > 0)
        h->p_fade_out_ticks_left--;

    if(!h->in_stasis_ticks && !h->throw_duration) {
        har_handle_stun(obj);
    }

    if(h->in_stasis_ticks > 0) {
        h->in_stasis_ticks--;
        if(h->in_stasis_ticks) {
            object_set_halt(obj, 1);
            object_add_animation_effects(obj, EFFECT_STASIS);
        } else {
            object_set_halt(obj, 0);
            object_del_animation_effects(obj, EFFECT_STASIS);
        }
    }

    // See if we are being grabbed. We detect this by checking the
    // "e" tag -- force to enemy position.
    // player_frame_isset(obj, "e");
    h->is_grabbed = h->throw_duration > 0;

    if(h->throw_duration > 0) {
        h->throw_duration--;

        if(h->throw_duration == 0) {
            // we've already called har_take_damage, so just apply the damage and check for defeat
            h->health -= h->last_damage_value;
            int stun_amount = h->last_stun_value;
            if(h->state == STATE_RECOIL && object_is_airborne(obj)) {
                stun_amount /= 2;
            }
            stun_amount = (stun_amount * 2 + 12) * 256;
            h->endurance += stun_amount;
            if(h->health <= 0) {
                // Take a screencap of enemy har
                game_player *other_player = game_state_get_player(obj->gs, !h->player_id);
                object *other_har = game_state_find_object(obj->gs, other_player->har_obj_id);
                har_screencaps_capture(&other_player->screencaps, other_har, obj, SCREENCAP_BLOW);

                // Slow down game more for last shot
                log_debug("Slowdown: Slowing from %d to %d.", game_state_get_speed(obj->gs),
                          h->health == 0 ? game_state_get_speed(obj->gs) - 10 : game_state_get_speed(obj->gs) - 6);
                game_state_slowdown(obj->gs, 12,
                                    h->health == 0 ? game_state_get_speed(obj->gs) - 10
                                                   : game_state_get_speed(obj->gs) - 6);

                har_event_defeat(h, ctrl);
            }
        }
    }

    if(player_frame_isset(obj, "aa")) {
        h->air_attacked = 0; // This tag allows you to attack again
    }
    // Make sure HAR doesn't walk through walls
    // TODO: Roof!
    vec2i pos = object_get_pos(obj);
    int ab_flag = player_frame_isset(obj, "ab");
    if(h->state != STATE_DEFEAT && !ab_flag) {
        int wall_flag = player_frame_isset(obj, "aw");
        int wall = 0;
        if(pos.x < ARENA_LEFT_WALL) {
            pos.x = ARENA_LEFT_WALL;
            obj->wall_collision = true;
        } else if(pos.x > ARENA_RIGHT_WALL) {
            pos.x = ARENA_RIGHT_WALL;
            wall = 1;
            obj->wall_collision = true;
        }

        af_move *move = af_get_move(h->af_data, obj->cur_animation->id);
        if(obj->wall_collision) {
            obj->wall_collision = false;
            if(wall_flag && move->next_move) {
                // log_debug("wall hit chaining to next animation %d", move->next_move);
                har_set_ani(obj, move->next_move, 0);
                h->executing_move = 1;
            } else {
                object_set_pos(obj, pos);
                har_event_hit_wall(h, wall, ctrl);
            }
        }
    }

    // Check for HAR specific palette tricks
    if(player_frame_isset(obj, "ptr") || player_frame_isset(obj, "ptd") || player_frame_isset(obj, "ptp")) {
        h->p_pal_ref = player_frame_get(obj, "pd");
        h->p_har_switch = player_frame_isset(obj, "pe");
        h->p_fade_out_ticks = h->p_fade_out_ticks_left = player_frame_get(obj, "ptr");
        h->p_fade_in_ticks = h->p_fade_in_ticks_left = player_frame_get(obj, "ptd");
        h->p_sustain_ticks_left = player_frame_get(obj, "ptp");
        // h->p_max_intensity = player_frame_get(obj, "pp");
        // h->p_base_intensity = player_frame_get(obj, "pb");
        h->p_color_fn = player_frame_isset(obj, "pa");
    }

    // Object took walldamage, but has now landed
    if(h->state == STATE_WALLDAMAGE && !object_is_airborne(obj)) {
        h->state = STATE_RECOIL;
    }

    // Reset air_attacked when not in the air to prevent HAR from freezing
    if(!object_is_airborne(obj) && h->air_attacked) {
        har_event_air_attack_done(h, ctrl);
        h->air_attacked = 0;
    }

    // tick down disabled moves
    if(hashmap_size(&h->disabled_animations)) {
        iterator it;
        hashmap_iter_begin(&h->disabled_animations, &it);
        hashmap_pair *pair = NULL;
        foreach(it, pair) {
            uint16_t *value = pair->value;
            if(*value <= 1) {
                hashmap_delete(&h->disabled_animations, &it);
            } else {
                (*value)--;
            }
        }
    }

    // Leave shadow trail
    // IF trail is on, copy current sprite to a new animation, and set animation string
    // to show the sprite with animation string that interpolates opacity down
    // Mark new object as the owner of the animation, so that the animation gets
    // removed when the object is finished.
    sprite *cur_sprite;
    if(object_has_effect(obj, EFFECT_TRAIL) && obj->age % 2 == 0 &&
       (cur_sprite = animation_get_sprite(obj->cur_animation, obj->cur_sprite_id))) {
        sprite *nsp = sprite_copy(cur_sprite);
        surface_flatten_to_mask(nsp->data, 1);
        object *nobj = omf_calloc(1, sizeof(object));
        object_create(nobj, obj->gs, object_get_pos(obj), vec2f_create(0, 0));
        object_set_stl(nobj, object_get_stl(obj));
        object_set_animation(nobj, create_animation_from_single(nsp, obj->cur_animation->start_pos));
        object_set_animation_owner(nobj, OWNER_OBJECT);
        object_set_custom_string(nobj, "bs100A1-bf0A15");
        object_add_animation_effects(nobj, EFFECT_SHADOW);
        object_set_direction(nobj, object_get_direction(obj));
        object_dynamic_tick(nobj);
        game_state_add_object(obj->gs, nobj, RENDER_LAYER_BOTTOM, 0, 0);
    }
}

static bool add_input_to_buffer(char *buf, char c) {
    // only add it if it is not the current head of the array
    if(buf[0] == c) {
        return false;
    }

    // use memmove to move everything over one spot in the array, leaving the first slot free
    memmove((buf) + 1, buf, 9);
    // write the new first element
    buf[0] = c;
    return true;
}

static bool add_input(char *buf, int act_type, int direction) {

    if(act_type == ACT_NONE) {
        return false;
    }
    if(direction == OBJECT_FACE_LEFT) {
        // flip left/right to simplify below switch
        if(act_type & ACT_LEFT) {
            act_type &= ~ACT_LEFT;
            act_type |= ACT_RIGHT;
        } else if(act_type & ACT_RIGHT) {
            act_type &= ~ACT_RIGHT;
            act_type |= ACT_LEFT;
        }
    }
    // for the reason behind the numbers, look at a numpad sometime
    switch(act_type & ~(ACT_PUNCH | ACT_KICK)) {
        case ACT_NONE:
            // make sure punches and kicks on their own frame are separated from previous input
            return add_input_to_buffer(buf, '5');
        case ACT_UP:
            return add_input_to_buffer(buf, '8');
        case ACT_DOWN:
            return add_input_to_buffer(buf, '2');
        case ACT_LEFT:
            return add_input_to_buffer(buf, '4');
        case ACT_RIGHT:
            return add_input_to_buffer(buf, '6');
        case ACT_UP | ACT_RIGHT:
            return add_input_to_buffer(buf, '9');
        case ACT_UP | ACT_LEFT:
            return add_input_to_buffer(buf, '7');
        case ACT_DOWN | ACT_RIGHT:
            return add_input_to_buffer(buf, '3');
        case ACT_DOWN | ACT_LEFT:
            return add_input_to_buffer(buf, '1');
        case ACT_STOP:
            // might just be kick or punch, check below
            break;
        default:
            log_warn("Ignored input: buf %s, act_type 0x%x, direction %d", buf, act_type, direction);
            assert(false);
    }

    if(act_type == ACT_STOP) {
        return add_input_to_buffer(buf, '5');
    }
    return false;
}

bool is_har_idle_grounded(object *obj) {
    return ((obj->cur_animation->id == ANIM_IDLE) || (obj->cur_animation->id == ANIM_CROUCHING) ||
            (obj->cur_animation->id == ANIM_WALKING)) &&
           game_state_hars_are_alive(obj->gs);
}

bool is_har_idle_air(object *obj) {
    return (obj->cur_animation->id == ANIM_JUMPING) && game_state_hars_are_alive(obj->gs);
}

bool is_move_chain_allowed(object *obj, af_move *move) {
    har *h = object_get_userdata(obj);

    bool allowed_in_idle = true;
    if(move->pos_constraints & 0x2) { // Not allowed to perform this move normally
        allowed_in_idle = false;
    }

    unsigned int len;
    uint16_t *count;
    int move_key = move->id;
    if(hashmap_get_int(&h->disabled_animations, move_key, (void **)(&count), &len) == 0) {
        return false;
    }

    if(h->is_wallhugging != 1 && move->pos_constraints & 0x1) {
        log_debug("Position contraint prevents move when not wallhugging!");
        // required to be wall hugging
        return false;
    }

    // check if the current frame allows chaining
    bool allowed = false;
    if(player_frame_isset(obj, "jn") && move->id == player_frame_get(obj, "jn")) {
        allowed = true;
    } else {
        switch(move->category) {
            case CAT_JUMPING:
                // JZ should be checked in a bunch of other places but since it's only used for one move we'll cheat.
                if(player_frame_isset(obj, "jz") || player_frame_isset(obj, "jj") ||
                   (is_har_idle_air(obj) && allowed_in_idle && !h->air_attacked)) {
                    allowed = true;
                }
                break;
            case CAT_CLOSE:
                if(player_frame_isset(obj, "jg") || (is_har_idle_grounded(obj) && allowed_in_idle)) {
                    object *enemy_obj = game_state_find_object(
                        obj->gs, game_player_get_har_obj_id(game_state_get_player(obj->gs, !h->player_id)));
                    if(enemy_obj->pos.y == ARENA_FLOOR && !har_is_invincible(enemy_obj, move) &&
                       is_in_range(obj, move)) {
                        allowed = true;
                    }
                }
                break;
            case CAT_LOW:
                if(player_frame_isset(obj, "jl") || (is_har_idle_grounded(obj) && allowed_in_idle)) {
                    allowed = true;
                }
                break;
            case CAT_MEDIUM:
                if(player_frame_isset(obj, "jm") || (is_har_idle_grounded(obj) && allowed_in_idle)) {
                    allowed = true;
                }
                break;
            case CAT_HIGH:
                if(player_frame_isset(obj, "jh") || (is_har_idle_grounded(obj) && allowed_in_idle)) {
                    allowed = true;
                }
                break;
            case CAT_SCRAP:
                if(player_frame_isset(obj, "jf") && h->state != STATE_DONE) {
                    allowed = true;
                }
                break;
            case CAT_DESTRUCTION:
                if(player_frame_isset(obj, "jf2")) {
                    allowed = true;
                }
                break;
            default:
                break;
        }
    }
    return allowed;
}

af_move *match_move(object *obj, char prefix, char *inputs) {
    har *h = object_get_userdata(obj);
    af_move *move = NULL;
    size_t len;

    for(int i = 0; i < 70; i++) {
        if((move = af_get_move(h->af_data, i))) {
            len = str_size(&move->move_string);
            if(str_at(&move->move_string, 0) == prefix &&
               (len == 1 || !strncmp(str_c(&move->move_string) + 1, inputs, len - 1))) {
                if(is_move_chain_allowed(obj, move)) {
                    if(str_size(&move->move_string) > 1) {
                        // matched a move that was not just a 5P or 5K
                        // so truncate the buffer
                        h->inputs[0] = 0;
                    }
                    return move;
                }
            }
        }
    }
    return NULL;
}

af_move *scrap_destruction_cheat(object *obj, char input) {
    har *h = object_get_userdata(obj);
    for(int i = 0; i < 70; i++) {
        af_move *move;
        if((move = af_get_move(h->af_data, i))) {
            if(move->category == CAT_SCRAP && h->state == STATE_VICTORY && input == 'K' &&
               (player_frame_isset(obj, "jf") || (player_frame_isset(obj, "jn") && i == player_frame_get(obj, "jn")))) {
                return move;
            }

            if(move->category == CAT_DESTRUCTION && h->state == STATE_SCRAP && input == 'P' &&
               (player_frame_isset(obj, "jf2") ||
                (player_frame_isset(obj, "jn") && i == player_frame_get(obj, "jn")))) {
                return move;
            }
        }
    }
    return NULL;
}

int maybe_har_change_state(int oldstate, int direction, char act_type) {
    int state = 0;
    switch(act_type) {
        case '1':
            state = STATE_CROUCHBLOCK;
            break;
        case '2':
            state = STATE_CROUCHING;
            break;
        case '3':
            state = STATE_CROUCHING;
            break;
        case '5':
            state = STATE_STANDING;
            break;
        case '4':
            state = STATE_WALKFROM;
            break;
        case '6':
            state = STATE_WALKTO;
            break;
        case '8':
            state = STATE_JUMPING;
            break;
        case '7':
            state = STATE_JUMPING;
            break;
        case '9':
            state = STATE_JUMPING;
            break;
        default:
            // if the input buffer is empty, assume standing
            state = STATE_STANDING;
    }
    if(oldstate != state) {
        // we changed state
        return state;
    }
    return 0;
}

int har_act(object *obj, int act_type) {
    har *h = object_get_userdata(obj);
    controller *ctrl = game_player_get_ctrl(game_state_get_player(obj->gs, h->player_id));

    // Prefetch enemy object & har links, they may be needed
    object *enemy_obj =
        game_state_find_object(obj->gs, game_player_get_har_obj_id(game_state_get_player(obj->gs, !h->player_id)));
    har *enemy_har = (har *)enemy_obj->userdata;

    int direction = object_get_direction(obj);
    // always queue input, I guess
    bool input_changed = add_input(h->inputs, act_type, direction);

    char prefix = 1; // should never match anything, even the empty string
    if(act_type & ACT_KICK) {
        prefix = 'K';
    } else if(act_type & ACT_PUNCH) {
        prefix = 'P';
    }

    uint32_t input_staleness = obj->gs->tick - h->input_change_tick;
    if(input_changed) {
        h->input_change_tick = obj->gs->tick;
    }

    if(h->endurance < 0) {
        if(prefix == 'K' || prefix == 'P') { // Mash to recover from stun faster!
            h->endurance += 512;
        }
        return 0;
    }

    if(object_get_halt(obj)) {
        // frozen, ignore input
        return 0;
    }

    if(is_har_idle_grounded(obj) && (object_distance(obj, enemy_obj) > 4)) {
        har_face_enemy(obj, enemy_obj);
    }

    // Don't allow movement if arena is starting or ending
    int arena_state = arena_get_state(game_state_get_scene(obj->gs));
    if(arena_state == ARENA_STATE_STARTING) {
        return 0;
    }

    // If the last key input before this one is older than 9 ticks don't consider any previous inputs, the buffer is
    // essentially just the last input and any kick/punch key This means that you have 9 ticks between the last 2 key
    // inputs to complete a move sequence
    char truncated_inputs[2] = {h->inputs[0], '\0'};
    af_move *move = match_move(obj, prefix, input_staleness <= 9 ? h->inputs : truncated_inputs);

    if(game_state_get_player(obj->gs, h->player_id)->ez_destruct && move == NULL &&
       (h->state == STATE_VICTORY || h->state == STATE_SCRAP)) {
        move = scrap_destruction_cheat(obj, prefix);
    }

    if(move) {

        if(h->state == STATE_WALKTO || h->state == STATE_WALKFROM) {
            // switch to standing to cancel any walk velocity changes
            h->state = STATE_STANDING;
        }

        // Set correct animation etc.
        // executing_move = 1 prevents new moves while old one is running.
        har_set_ani(obj, move->id, 0);
        // TODO the original will eventually discard inputs, but we don't understand how yet
        // so we simply do nothing for now
        // h->inputs[0] = '\0';
        h->executing_move = 1;

        // If animation is scrap or destruction, then remove our customizations
        // from gravity/fall speed, and just use the HARs native value.
        if(move->category == CAT_SCRAP || move->category == CAT_DESTRUCTION) {
            obj->horizontal_velocity_modifier = 1.0f;
            obj->vertical_velocity_modifier = 1.0f;
            object_set_gravity(obj, h->af_data->fall_speed);
            object_set_gravity(enemy_obj, enemy_har->af_data->fall_speed);
        }

        if(move->category == CAT_SCRAP) {
            log_debug("going to scrap state");
            h->state = STATE_SCRAP;
            har_event_scrap(h, ctrl);
        } else if(move->category == CAT_DESTRUCTION) {
            log_debug("going to destruction state");
            h->state = STATE_DESTRUCTION;
            har_event_destruction(h, ctrl);
        } else {
            har_event_attack(h, move, ctrl);
        }

        // make the other har participate in the scrap/destruction if there's not a walk involved
        if((move->category == CAT_SCRAP || move->category == CAT_DESTRUCTION) && h->walk_destination < 0) {
            object_set_animation(enemy_obj, &af_get_move(enemy_har->af_data, ANIM_DAMAGE)->ani);
            object_set_repeat(enemy_obj, 0);
            object_set_custom_string(enemy_obj, str_c(&move->footer_string));
            object_dynamic_tick(enemy_obj);
        }

        if(h->state == STATE_NONE) {
            if(move->category == CAT_HIGH || move->category == CAT_MEDIUM || move->category == CAT_CLOSE) {
                h->state = STATE_STANDING;
            } else if(move->category == CAT_LOW) {
                h->state = STATE_CROUCHING;
            }
        }
        // we actually did something interesting
        // return 1 so we can use this as sync point for netplay
        return 1;
    }

    // Don't allow new movement while we're still executing a move
    if(h->executing_move) {
        if(obj->pos.y < ARENA_FLOOR) {
            if(h->state < STATE_JUMPING) {
                log_debug("standing move led to airborne one");
                h->state = STATE_JUMPING;
            } else if(h->state != STATE_JUMPING) {
                log_debug("state is %d", h->state);
            }
        }
        return 0;
    }

    char last_input = get_last_input(h);
    if(object_is_airborne(obj)) {
        // airborne

        // HAR can have STATE_NONE here if they started an airborne attack from a crouch, like katana's corkscrew blade
        // or wall spin
        if(h->state == STATE_NONE) {
            // so set them into jumping state
            h->state = STATE_JUMPING;
        }

        // Send an event if the har tries to turn in the air by pressing either left/right/downleft/downright
        int opp_id = h->player_id ? 0 : 1;
        object *opp =
            game_state_find_object(obj->gs, game_player_get_har_obj_id(game_state_get_player(obj->gs, opp_id)));
        if(last_input == '4' || last_input == '6' || last_input == '1' || last_input == '3') {
            if(object_get_pos(obj).x > object_get_pos(opp).x) {
                if(direction != OBJECT_FACE_LEFT) {
                    har_event_air_turn(h, ctrl);
                    return 1;
                }
            } else {
                if(direction != OBJECT_FACE_RIGHT) {
                    har_event_air_turn(h, ctrl);
                    return 1;
                }
            }
        }

        return 0;
    }

    if(!(is_har_idle_grounded(obj) || is_har_idle_air(obj))) {
        return 0;
    }

    float vx, vy;
    // no moves matched, do player movement
    int newstate;
    if((newstate = maybe_har_change_state(h->state, direction, last_input))) {
        h->state = newstate;
        switch(newstate) {
            case STATE_CROUCHBLOCK:
                har_set_ani(obj, ANIM_CROUCHING, 1);
                object_set_vel(obj, vec2f_create(0, 0));
                break;
            case STATE_CROUCHING:
                har_set_ani(obj, ANIM_CROUCHING, 1);
                object_set_vel(obj, vec2f_create(0, 0));
                break;
            case STATE_STANDING:
                har_set_ani(obj, ANIM_IDLE, 1);
                object_set_stride(obj, h->stride);
                object_set_vel(obj, vec2f_create(0, 0));
                obj->slide_state.vel.x = 0;
                break;
            case STATE_WALKTO:
                har_set_ani(obj, ANIM_WALKING, 1);
                object_set_stride(obj, h->stride);
                har_event_walk(h, 1, ctrl);
                break;
            case STATE_WALKFROM:
                har_set_ani(obj, ANIM_WALKING, 1);
                object_set_stride(obj, h->stride);
                har_event_walk(h, -1, ctrl);
                break;
            case STATE_JUMPING:
                har_set_ani(obj, ANIM_JUMPING, 0);
                vx = 0.0f;
                vy = h->jump_speed;
                int jump_dir = 0;
                if(last_input == '9') {
                    vx = (h->fwd_speed * direction);
                    object_set_tick_pos(obj, 110);
                    object_set_stride(obj, 7); // Pass 7 frames per tick
                    jump_dir = 1;
                } else if(last_input == '7') {
                    // If we are jumping backwards, start animation from end
                    // at -100 frames (seems to be about right)
                    object_set_playback_direction(obj, PLAY_BACKWARDS);
                    object_set_tick_pos(obj, -110);
                    vx = (h->back_speed * direction * -1);
                    object_set_stride(obj, 7); // Pass 7 frames per tick
                    jump_dir = -1;
                } else if(last_input == '8') {
                    // If we are jumping upwards
                    object_set_tick_pos(obj, 110);
                    if(h->id == HAR_GARGOYLE) {
                        object_set_stride(obj, 7);
                    }
                }
                if(input_staleness <= 6 && (h->inputs[1] == '1' || h->inputs[1] == '2' || h->inputs[1] == '3')) {
                    // jumping from crouch makes you jump 25% higher
                    vy = h->superjump_speed;
                }
                object_set_vel(obj, vec2f_create(vx, vy));
                har_event_jump(h, jump_dir, ctrl);
                break;
        }
        return 1;
    }

    // if enemy is airborn we fire extra walk event to check whether we need to turn
    // fixes some rare behaviour where you cannot kick-counter someone who jumps over you
    int opp_id = h->player_id ? 0 : 1;
    object *opp = game_state_find_object(obj->gs, game_player_get_har_obj_id(game_state_get_player(obj->gs, opp_id)));
    if(object_is_airborne(opp)) {
        // har_event_walk(h, 1, ctrl);
    }

    return 0;
}

void har_face_enemy(object *obj, object *obj_enemy) {
    har *h = object_get_userdata(obj);

    if(h->in_stasis_ticks > 0)
        return;

    vec2i pos = object_get_pos(obj);
    vec2i pos_enemy = object_get_pos(obj_enemy);
    int new_facing = pos.x > pos_enemy.x ? OBJECT_FACE_LEFT : OBJECT_FACE_RIGHT;
    if(obj->direction != new_facing) {
        log_debug("HARS facing player %d %s", h->player_id, new_facing == OBJECT_FACE_LEFT ? "LEFT" : "RIGHT");
        object_set_direction(obj, new_facing);
    }
}

void har_finished(object *obj) {
    har *h = object_get_userdata(obj);
    controller *ctrl = game_player_get_ctrl(game_state_get_player(obj->gs, h->player_id));

    h->executing_move = 0;

    if(obj->enqueued) {
        har_set_ani(obj, obj->enqueued, 0);
        if(obj->enqueued == ANIM_STANDUP) {
            object *enemy_obj = game_state_find_object(
                obj->gs, game_player_get_har_obj_id(game_state_get_player(obj->gs, !h->player_id)));
            har_face_enemy(obj, enemy_obj);
        }
        obj->enqueued = 0;
    } else if(h->state == STATE_SCRAP || h->state == STATE_DESTRUCTION) {
        // play victory animation again, but do not allow any more moves to be executed
        h->state = STATE_DONE;
        har_set_ani(obj, ANIM_VICTORY, 0);
        har_event_done(h, ctrl);
    } else if(h->state == STATE_VICTORY || h->state == STATE_DONE) {
        // prevent object from being freed, hold last sprite of animation indefinitely
        obj->animation_state.finished = 0;
        if(obj->cur_animation->id != ANIM_VICTORY) {
            // we've won but the game hasn't set us to victory yet, so do idle
            har_set_ani(obj, ANIM_IDLE, 1);
            return;
        }
    } else if(h->state == STATE_RECOIL && h->health <= 0) {
        h->state = STATE_DEFEAT;
        har_set_ani(obj, h->custom_defeat_animation ? h->custom_defeat_animation : ANIM_DEFEAT, 0);
    } else if(h->state == STATE_RECOIL && player_get_last_frame_letter(obj) == 'M') {
        har_event_recover(h, ctrl);
        h->state = STATE_STANDING_UP;
        object_set_custom_string(obj, "zzO7-bj2zzO2");
    } else if((h->state == STATE_RECOIL || h->state == STATE_STANDING_UP) && h->endurance < 0) {
        if(h->state == STATE_RECOIL) {
            har_event_recover(h, ctrl);
        }
        h->state = STATE_STUNNED;
        har_set_ani(obj, ANIM_STUNNED, 1);
        har_event_stun(h, ctrl);

        // fire enemy stunned event
        object *enemy_obj =
            game_state_find_object(obj->gs, game_player_get_har_obj_id(game_state_get_player(obj->gs, !h->player_id)));
        har *enemy_h = object_get_userdata(enemy_obj);
        controller *enemy_ctrl = game_player_get_ctrl(game_state_get_player(enemy_obj->gs, enemy_h->player_id));
        har_event_enemy_stun(enemy_h, enemy_ctrl);
    } else if(h->state != STATE_CROUCHING && h->state != STATE_CROUCHBLOCK) {
        // Don't transition to standing state while in midair
        if(object_is_airborne(obj) && h->state == STATE_RECOIL) {
            if(h->health <= 0 || h->endurance <= 0) {
                // leave them in the last frame until they hit the ground
                obj->animation_state.finished = 0;
            } else {
                // XXX if we don't switch to STATE_JUMPING after getting damaged in the air, then the HAR_LAND_EVENT
                // will never get fired.
                h->state = STATE_JUMPING;
                har_set_ani(obj, ANIM_JUMPING, 0);
            }
        } else if(h->state == STATE_RECOIL) {
            har_event_recover(h, ctrl);
            h->state = STATE_STANDING;
            har_set_ani(obj, ANIM_IDLE, 1);
            har_act(obj, ACT_NONE);
        } else if(object_is_airborne(obj)) {
            // finished an attack animation in the air
            h->state = STATE_JUMPING;
            har_set_ani(obj, ANIM_JUMPING, 0);
        } else if(h->health <= 0) {
            // player has no health and is grounded.. so they must be defeated.
            h->state = STATE_DEFEAT;
            har_set_ani(obj, h->custom_defeat_animation ? h->custom_defeat_animation : ANIM_DEFEAT, 0);
        } else {
            // the HAR is on the ground, so clear the air attack tracker
            h->air_attacked = 0;
            // we don't know what state the HAR should be in now, so set it to NONE
            h->state = STATE_NONE;
            har_set_ani(obj, ANIM_IDLE, 1);
            har_act(obj, ACT_NONE);
        }
    } else {
        // we don't know what state the HAR should be in now, so set it to NONE
        h->state = STATE_NONE;
        har_set_ani(obj, ANIM_CROUCHING, 1);
        har_act(obj, ACT_NONE);
    }

    object *obj_enemy =
        game_state_find_object(obj->gs, game_state_get_player(obj->gs, h->player_id == 1 ? 0 : 1)->har_obj_id);
    har *har_enemy = object_get_userdata(obj_enemy);

    if(har_enemy->state != STATE_DEFEAT && har_enemy->state != STATE_VICTORY && har_enemy->state != STATE_DONE) {
        // har_act MUST provide a new state
        assert(h->state != STATE_NONE);
    }
}

void har_install_hook(har *h, har_hook_cb hook, void *data) {
    har_hook hk;
    hk.cb = hook;
    hk.data = data;
    list_append(&h->har_hooks, &hk, sizeof(har_hook));

    /*h->hook_cb = hook;*/
    /*h->hook_cb_data = data;*/
}

int har_clone(object *src, object *dst) {
    har *oldlocal = object_get_userdata(src);
    har *local = omf_calloc(1, sizeof(har));
    memcpy(local, oldlocal, sizeof(har));
    list_create(&local->har_hooks);
    hashmap_create(&local->disabled_animations);
    iterator it;
    hashmap_iter_begin(&oldlocal->disabled_animations, &it);
    hashmap_pair *pair = NULL;
    foreach(it, pair) {
        hashmap_put(&local->disabled_animations, pair->key, sizeof(int), pair->value, sizeof(uint16_t));
    }

    object_set_userdata(dst, local);
    object_set_spawn_cb(dst, cb_har_spawn_object, dst);
    vector_create(&local->child_objects, sizeof(uint32_t));
    uint32_t *child_id;
    vector_iter_begin(&oldlocal->child_objects, &it);
    foreach(it, child_id) {
        vector_append(&local->child_objects, child_id);
    }
    object_set_destroy_cb(dst, cb_har_destroy_object, dst);
    object_set_disable_cb(dst, cb_har_disable_animation, dst);
    local->delay = 0;
    return 0;
}

int har_clone_free(object *obj) {
    har *har = object_get_userdata(obj);
    list_free(&har->har_hooks);
    vector_free(&har->child_objects);
    hashmap_free(&har->disabled_animations);
    omf_free(har);
    object_set_userdata(obj, NULL);
    return 0;
}

void har_bootstrap(object *obj) {
    obj->clone = har_clone;
    obj->clone_free = har_clone_free;
}

int har_create(object *obj, af *af_data, int dir, int har_id, int pilot_id, int player_id) {
    // Create local data
    har *local = omf_calloc(1, sizeof(har));
    object_set_userdata(obj, local);
    har_bootstrap(obj);

    game_player *gp = game_state_get_player(obj->gs, player_id);
    local->af_data = af_data;

    vector_create(&local->child_objects, sizeof(uint32_t));

    // Save har id
    local->id = har_id;
    local->player_id = player_id;
    local->pilot_id = pilot_id;
    sd_pilot *pilot = gp->pilot;

    // power 2 applies to player 1's health
    int power = obj->gs->match_settings.power2;
    if(player_id == 1) {
        // power 1 applies to player 2's health
        power = obj->gs->match_settings.power1;
    }

    float jump_multiplier = 1.0;
    float vitality_multiplier = 1.0;
    float power_multiplier = 1.0;
    if(!is_tournament(obj->gs)) {
        jump_multiplier = obj->gs->match_settings.jump_height / 100.0;
        vitality_multiplier = obj->gs->match_settings.vitality / 100.0;
        // see https://www.omf2097.com/wiki/doku.php?id=omf2097:stats
        power_multiplier = 2.25 - 0.25 * power; // TODO: skip when either player is AI

        //  The stun cap is calculated as follows
        //  HAR Endurance * 3.6 * (Pilot Endurance + 16) / 23
        local->endurance_max = (af_data->endurance * 3.6 * (pilot->endurance + 16) / 23);
        local->stun_factor = 1.2 * 256 * (pilot->endurance + 30) / 40;
    } else {
        local->endurance_max = (af_data->endurance * 3.6 * (pilot->endurance + 25) / 37);
        local->endurance_max = (local->endurance_max * (pilot->stun_resistance + 2)) / 3;
        local->stun_factor = (pilot->stun_resistance + 3.0) * 0.2 * 0.9 * 256.0 * (pilot->endurance + 30) / 40;
    }

    // Health, endurance
    // HP is
    // (HAR hp * (Pilot Endurance + 25) / 35) * 1.1
    local->health_max = local->health =
        ((af_data->health * (pilot->endurance + 25) / 35) * 1.1) * power_multiplier * vitality_multiplier;

    // log_debug("HAR health is %d with pilot endurance %d and base health %d", local->health, pilot->endurance,
    // af_data->health);
    log_debug("HAR endurance is %d with pilot endurance %d and base endurance %f", local->endurance_max,
              pilot->endurance, af_data->endurance);
    log_debug("HAR stun factor is %d", local->stun_factor);

    // fwd speed = (Agility + 20) / 30 * fwd speed
    // back speed = (Agility + 20) / 30 * back speed
    // up speed = (Agility + 35) / 45 * up speed (edited)
    // fall speed = (Agility + 20) / 30 * fall speed
    //
    // Insanius: there's a value labeled here horizontal_agility_modifier which is  (Agility + 35) / 45 (edited)
    // Insanius: And vertical_agility_modifier which is (Agility + 20) / 30 (edited)
    // Insanius: I suspect these labels may not be accurate but we'll see
    // Insanius: I went ahead and changed the formulas to use division instead of multiplication since it's more precise
    // for us Insanius: jump speed = speed up * vertical_agility_modifier * 212 / 256 Insanius: superjump speed = speed
    // up * vertical_agility_modifier * 266 / 256
    float horizontal_agility_modifier = ((float)gp->pilot->agility + 35) / 45;
    float vertical_agility_modifier = ((float)gp->pilot->agility + 20) / 30;
    obj->horizontal_velocity_modifier = horizontal_agility_modifier;
    obj->vertical_velocity_modifier = vertical_agility_modifier;
    local->jump_speed = (((float)gp->pilot->agility + 35) / 45) * af_data->jump_speed * jump_multiplier * 216 / 256;
    local->superjump_speed =
        (((float)gp->pilot->agility + 35) / 45) * af_data->jump_speed * jump_multiplier * 266 / 256;
    local->fall_speed = (((float)gp->pilot->agility + 20) / 30) * af_data->fall_speed;
    local->fwd_speed = (((float)gp->pilot->agility + 20) / 30) * af_data->forward_speed;
    local->back_speed = (((float)gp->pilot->agility + 20) / 30) * af_data->reverse_speed;
    local->stride = (gp->pilot->agility + 20) / 30;
    log_debug("setting HAR stride to %d", local->stride);
    local->close = 0;
    local->hard_close = 0;
    local->state = STATE_STANDING;
    local->executing_move = 0;
    local->air_attacked = 0;
    local->is_wallhugging = 0;
    local->is_grabbed = 0;

    local->in_stasis_ticks = 0;
    local->throw_duration = 0;

    local->delay = 0;

    local->walk_destination = -1;
    local->walk_done_anim = 0;

    // Last damage value, for convenience
    local->last_damage_value = 0.0f;

    // p<x> stuff
    local->p_fade_in_ticks_left = local->p_fade_in_ticks = 0;
    local->p_fade_out_ticks_left = local->p_fade_out_ticks = 0;
    local->p_sustain_ticks_left = 0;

    list_create(&local->har_hooks);

    hashmap_create(&local->disabled_animations);

    local->stun_timer = 0;

    object_set_group(obj, GROUP_HAR);

    // Set palette offset 0 for player1, 48 for player2
    object_set_pal_offset(obj, player_id * 48);
    object_set_pal_limit(obj, (player_id + 1) * 48);

    // Object related stuff
    object_set_gravity(obj, local->fall_speed);
    object_set_layers(obj, LAYER_HAR | (player_id == 0 ? LAYER_HAR1 : LAYER_HAR2));
    object_set_direction(obj, dir);
    object_set_repeat(obj, 1);
    object_set_stl(obj, local->af_data->sound_translation_table);
    object_set_shadow(obj, 1);

    // New object spawner callback
    object_set_spawn_cb(obj, cb_har_spawn_object, obj);
    object_set_destroy_cb(obj, cb_har_destroy_object, obj);
    object_set_disable_cb(obj, cb_har_disable_animation, obj);

    // Set running animation
    har_set_ani(obj, ANIM_IDLE, 1);
    object_set_stride(obj, local->stride);

    // fill the input buffer with 'pauses'
    memset(local->inputs, '5', 10);
    local->inputs[10] = '\0';

    // Callbacks and userdata
    object_set_free_cb(obj, har_free);
    object_set_act_cb(obj, har_act);
    object_set_dynamic_tick_cb(obj, har_tick);
    object_set_move_cb(obj, har_move);
    object_set_collide_cb(obj, har_collide);
    object_set_finish_cb(obj, har_finished);

#ifdef DEBUGMODE
    object_set_debug_cb(obj, har_debug);
    surface_create(&local->hit_pixel, 1, 1);
    surface_clear(&local->hit_pixel);
    image img;
    surface_to_image(&local->hit_pixel, &img);
    image_set_pixel(&img, 0, 0, 0xf3);
    surface_create(&local->har_origin, 4, 4);
    surface_clear(&local->har_origin);
    surface_to_image(&local->har_origin, &img);
    image_set_pixel(&img, 0, 0, 0xf6);
    image_set_pixel(&img, 0, 1, 0xf6);
    image_set_pixel(&img, 0, 2, 0xf6);
    image_set_pixel(&img, 0, 3, 0xf6);
    image_set_pixel(&img, 1, 3, 0xf6);
    image_set_pixel(&img, 2, 3, 0xf6);
    image_set_pixel(&img, 3, 3, 0xf6);

#endif

    // Enable Electra Electricity & Pyros Fire palette tricks
    object_add_animation_effects(obj, EFFECT_HAR_QUIRKS);

    // fixup a bunch of stuff based on player stats

    float leg_power = 0.0f;
    float arm_power = 0.0f;
    // cheap way to check if we're in tournament mode
    if(is_tournament(obj->gs)) {
        // (Limb Power + 3) * .192
        leg_power = (pilot->leg_power + 3) * 0.192f;
        arm_power = (pilot->arm_power + 3) * 0.192f;
    }

    af_move *move;
    int extra_index;
    int fight_mode = obj->gs->match_settings.fight_mode;
    // apply pilot stats and HAR upgrades/enhancements/hyper mode to the HAR
    for(int i = 0; i < MAX_AF_MOVES; i++) {
        move = af_get_move(af_data, i);
        if(move != NULL) {
            if(!is_tournament(obj->gs)) {
                // Single Player
                // Damage = Base Damage * (20 + Power) / 30 + 1
                //  Stun = (Base Damage + 6) * 512
                if(move->damage) {
                    move->stun = move->damage;
                    move->damage = move->damage * (20 + pilot->power) / 30 + 1;
                }
                // projectiles have hyper mode, but may have extra_string_selector of 0
                if(move->ani.extra_string_count > 0 && move->extra_string_selector != 1 &&
                   move->extra_string_selector != 2) {
                    str *str = vector_get(&move->ani.extra_strings, fight_mode ? 1 : 0);
                    if(str && str_size(str) != 0 && !str_equal_c(str, "!")) {
                        // its not the empty string and its not the string '!'
                        // so we should use it
                        str_set(&move->ani.animation_string, vector_get(&move->ani.extra_strings, fight_mode ? 1 : 0));
                        log_debug("using %s mode string '%s' for animation %d on har %d",
                                  fight_mode ? "hyper" : "normal", str_c(str), i, har_id);
                    }
                }
            } else {
                // normal or hyper
                extra_index = fight_mode ? 1 : 0;
                // Tournament Mode
                // Damage = (Base Damage * (25 + Power) / 35 + 1) * leg/arm power / armor
                // Stun = ((Base Damage * (35 + Power) / 45) * 2 + 12) * 256
                if(move->damage) {
                    move->stun = move->damage * (35 + pilot->power) / 45;
                }

                // check for enhancements
                if(move->ani.extra_string_count > 0 && move->extra_string_selector != 1 &&
                   move->extra_string_selector != 2) {
                    // if you have 1 enhancement choose extra string 2
                    // if you have 2 enhancements choose extra string 3
                    // if you have 3 enhancements choose extra string 4
                    // if that string doesn't exist, pick the highest one
                    if(pilot->enhancements[har_id] > 0) {
                        // find the last populated enhancement index, or 1 (hyper)
                        extra_index = min2(1 + pilot->enhancements[har_id], move->ani.extra_string_count - 1);
                    }

                    str *str = vector_get(&move->ani.extra_strings, extra_index);
                    if(str && str_size(str) != 0 && !str_equal_c(str, "!")) {
                        // its not the empty string and its not the string '!'
                        // so we should use it
                        str_set(&move->ani.animation_string, str);
                        if(pilot->enhancements[har_id] > 0) {
                            log_debug("using enhancement %d string '%s' for animation %d on har %d",
                                      pilot->enhancements[har_id], str_c(str), i, har_id);
                        } else {
                            log_debug("using %s mode string '%s' for animation %d on har %d",
                                      fight_mode ? "hyper" : "normal", str_c(str), i, har_id);
                        }
                    }
                }
                switch(move->extra_string_selector) {
                    case 0:
                        break;
                    case 1:
                        // arm speed and power
                        if(move->damage) {
                            move->damage = (move->damage * (25 + pilot->power) / 35 + 1) * arm_power;
                        }
                        if(move->ani.extra_string_count > 0) {
                            // sometimes there's not enough extra strings, so take the last available
                            str_set(&move->ani.animation_string,
                                    vector_get(&move->ani.extra_strings,
                                               min2(pilot->arm_speed, move->ani.extra_string_count - 1)));
                        }
                        break;
                    case 2:
                        // leg speed and power
                        if(move->damage) {
                            move->damage = (move->damage * (25 + pilot->power) / 35 + 1) * leg_power;
                        }
                        if(move->ani.extra_string_count > 0) {
                            // sometimes there's not enough extra strings, so take the last available
                            str_set(&move->ani.animation_string,
                                    vector_get(&move->ani.extra_strings,
                                               min2(pilot->leg_speed, move->ani.extra_string_count - 1)));
                        }
                        break;
                    case 3:
                        // apply arm power for damage
                        if(move->damage) {
                            move->damage = (move->damage * (25 + pilot->power) / 35 + 1) * arm_power;
                        }
                        break;
                    case 4:
                        // apply leg power for damage
                        if(move->damage) {
                            move->damage = (move->damage * (25 + pilot->power) / 35 + 1) * leg_power;
                        }
                        break;
                    case 5:
                        // apply leg and arm power for damage
                        if(move->damage) {
                            move->damage = (move->damage * (25 + pilot->power) / 35 + 1) * arm_power * leg_power;
                        }
                        break;
                }
            }
            if(str_equal_c(&move->ani.animation_string, "A1")) {
                // this is a nonsense string, so we should not allow this animation to be matched
                // scramble the move string
                log_debug("disabling move %d on har %d because it has no-op animation string '%s'", i, har_id,
                          str_c(&move->ani.animation_string));
                str_set_c(&move->move_string, "!");
            }
        }
    }

    // All done
    return 0;
}

void har_reset(object *obj) {
    har *h = object_get_userdata(obj);
    object_set_gravity(obj, h->fall_speed);
    h->close = 0;
    h->hard_close = 0;
    h->state = STATE_STANDING;
    h->executing_move = 0;
    h->air_attacked = 0;
    h->is_wallhugging = 0;
    h->is_grabbed = 0;
    h->inputs[10] = '\0';
    h->health = h->health_max;
    h->endurance = 0;

    h->in_stasis_ticks = 0;
    h->throw_duration = 0;

    h->walk_destination = -1;
    h->walk_done_anim = 0;

    har_set_ani(obj, ANIM_IDLE, 1);
    object_set_stride(obj, h->stride);
}

void har_set_delay(object *obj, int delay) {
    har *h = object_get_userdata(obj);
    h->delay = delay;
}

uint8_t har_player_id(object *obj) {
    har *h = object_get_userdata(obj);
    return h->player_id;
}

int16_t har_health_percent(har *h) {
    return 100 * h->health / h->health_max;
}

void har_connect_child(object *obj, object *child) {
    har *h = object_get_userdata(obj);
    log_debug("linking child %d to HAR", child->id);
    vector_append(&h->child_objects, &child->id);
}
