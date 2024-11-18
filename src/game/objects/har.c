#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio/audio.h"
#include "controller/controller.h"
#include "formats/af.h"
#include "formats/transparent.h"
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
void har_palette_transform(damage_tracker *damage, vga_palette *pal, void *obj);

void har_free(object *obj) {
    har *h = object_get_userdata(obj);
    list_free(&h->har_hooks);
#ifdef DEBUGMODE
    surface_free(&h->cd_debug);
#endif
    omf_free(h);
    object_set_userdata(obj, NULL);
}

/* hooks */

void fire_hooks(har *h, har_event event, controller *ctrl) {
    iterator it;
    har_hook *hook;

    list_iter_begin(&h->har_hooks, &it);
    while((hook = iter_next(&it)) != NULL) {
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
        h->endurance = h->endurance_max;
        h->state = STATE_STANDING;
        har_set_ani(har_obj, ANIM_IDLE, 1);
    }
}

void har_action_hook(object *obj, int action) {
    har *h = object_get_userdata(obj);
    if(h->action_hook_cb) {
        h->action_hook_cb(action, h->action_hook_cb_data);
    }
    int pos = obj->age % OBJECT_EVENT_BUFFER_SIZE;
    h->act_buf[pos].actions[h->act_buf[pos].count] = (unsigned char)action;
    h->act_buf[pos].count++;
    h->act_buf[pos].age = obj->age;
}

// Simple helper function
void har_set_ani(object *obj, int animation_id, int repeat) {
    har *h = object_get_userdata(obj);
    af_move *move = af_get_move(h->af_data, animation_id);
    char *s = (char *)str_c(&move->move_string);
    uint8_t has_corner_hack = obj->animation_state.shadow_corner_hack;
    object_set_animation(obj, &move->ani);
    obj->animation_state.shadow_corner_hack = has_corner_hack;
    if(s != NULL && strcmp(s, "!") != 0 && strcmp(s, "0") != 0 && h->delay > 0) {
        DEBUG("delaying move %d %s by %d ticks", move->id, s, h->delay);
        object_set_delay(obj, h->delay);
    }

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
    h->flinching = 0;
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
    if(h->state == STATE_CROUCHBLOCK && move->category != CAT_JUMPING && h->executing_move == 0) {
        return 1;
    }
    if(h->state == STATE_WALKFROM && move->category != CAT_LOW && h->executing_move == 0) {
        return 1;
    }
    return 0;
}

int har_is_invincible(object *obj, af_move *move) {
    if(player_frame_isset(obj, "zz")) {
        // blocks everything
        return 1;
    }
    switch(move->category) {
        // XX 'zg' is not handled here, but the game doesn't use it...
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

// Callback for spawning new objects, eg. projectiles
void cb_har_spawn_object(object *parent, int id, vec2i pos, vec2f vel, uint8_t mp_flags, int s, int g, void *userdata) {
    har *h = userdata;
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
        object_set_gravity(obj, g / 256.0f);
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
        projectile_create(obj, h);

        // allow projectiles to spawn projectiles, eg. shadow's scrap animation
        object_set_spawn_cb(obj, cb_har_spawn_object, h);

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

    // When kreissack explodes, spwan some scrap too. Just to make it awesome.
    if(h->id == 10 && id >= 25 && id <= 30) {
        har_spawn_scrap(parent, pos, 12);
    }
}

void har_floor_landing_effects(object *obj) {
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
    float d = ((float)obj->pos.x) / 640.0f;
    float pos_pan = d - 0.25f;
    audio_play_sound(56, 0.3f, pos_pan, 2.2f);
}

void har_move(object *obj) {
    vec2f vel = object_get_vel(obj);
    obj->pos.x += vel.x;
    obj->pos.y += vel.y;
    har *h = object_get_userdata(obj);

    // Check for wall hits
    if(obj->pos.x <= ARENA_LEFT_WALL || obj->pos.x >= ARENA_RIGHT_WALL) {
        h->is_wallhugging = 1;
        if(player_frame_isset(obj, "cw") && player_frame_isset(obj, "d")) {
            DEBUG("disabling d tag on animation because of wall hit");
            obj->animation_state.disable_d = 1;
        }

    } else {
        h->is_wallhugging = 0;
    }

    // Handle floor collisions
    if(obj->pos.y > ARENA_FLOOR) {
        controller *ctrl = game_player_get_ctrl(game_state_get_player(obj->gs, h->player_id));
        if(h->state != STATE_FALLEN) {
            // We collided with ground, so set vertical velocity to 0 and
            // make sure object is level with ground
            obj->pos.y = ARENA_FLOOR;
            object_set_vel(obj, vec2f_create(vel.x, 0));
        }

        // Change animation from jump to walk or idle,
        // depending on horizontal velocity
        if(h->state == STATE_JUMPING) {
            /*if(object_get_hstate(obj) == OBJECT_MOVING) {*/
            /*h->state = STATE_WALKING;*/
            /*har_set_ani(obj, ANIM_WALKING, 1);*/
            /*} else {*/
            h->state = STATE_STANDING;
            har_set_ani(obj, ANIM_IDLE, 1);
            object_set_stride(obj, h->stride);
            har_action_hook(obj, ACT_STOP);
            har_event_land(h, ctrl);
            har_floor_landing_effects(obj);

            // make sure HAR's are facing each other
            object *obj_enemy =
                game_state_find_object(obj->gs, game_state_get_player(obj->gs, h->player_id == 1 ? 0 : 1)->har_obj_id);
            if(object_get_direction(obj) == object_get_direction(obj_enemy)) {
                DEBUG("HARS facing same direction");
                vec2i pos = object_get_pos(obj);
                vec2i pos_enemy = object_get_pos(obj_enemy);
                if(pos.x > pos_enemy.x) {
                    DEBUG("HARS facing player %d LEFT", h->player_id);
                    object_set_direction(obj, OBJECT_FACE_LEFT);
                } else {
                    DEBUG("HARS facing player %d RIGHT", h->player_id);
                    object_set_direction(obj, OBJECT_FACE_RIGHT);
                }

                DEBUG("HARS facing enemy player %d", abs(h->player_id - 1));
                object_set_direction(obj_enemy, object_get_direction(obj) * -1);
            }
            /*}*/
        } else if(h->state == STATE_FALLEN || h->state == STATE_RECOIL) {
            float dampen = 0.2f;
            vec2f vel = object_get_vel(obj);
            vec2i pos = object_get_pos(obj);
            if(pos.y > ARENA_FLOOR) {
                pos.y = ARENA_FLOOR;
                vel.y = -vel.y * dampen;
                vel.x = vel.x * dampen;
                har_floor_landing_effects(obj);
            }

            if(pos.x <= ARENA_LEFT_WALL || pos.x >= ARENA_RIGHT_WALL) {
                vel.x = 0.0;
            }

            object_set_pos(obj, pos);
            object_set_vel(obj, vel);

            // prevent har from sliding after defeat, unless they're 'fallen'
            if(h->state != STATE_DEFEAT && h->state != STATE_FALLEN && h->health <= 0 && player_is_last_frame(obj)) {

                h->state = STATE_DEFEAT;
                har_set_ani(obj, ANIM_DEFEAT, 0);
                har_event_defeat(h, ctrl);
            } else if(pos.y >= (ARENA_FLOOR - 5) && IS_ZERO(vel.x) && player_is_last_frame(obj)) {
                if(h->state == STATE_FALLEN) {
                    if(h->health <= 0) {
                        // fallen, but done bouncing
                        h->state = STATE_DEFEAT;
                        har_set_ani(obj, ANIM_DEFEAT, 0);
                        har_event_defeat(h, ctrl);
                    } else {
                        h->state = STATE_STANDING_UP;
                        har_set_ani(obj, ANIM_STANDUP, 0);
                        har_event_land(h, ctrl);
                    }
                } else {
                    har_finished(obj);
                }
            }
        }
    } else {
        object_set_vel(obj, vec2f_create(vel.x, vel.y + obj->gravity));
    }
}

void har_take_damage(object *obj, const str *string, float damage, float stun) {
    har *h = object_get_userdata(obj);

    // Got hit, disable stasis activator on this bot
    h->in_stasis_ticks = 1;

    // Save damage taken
    h->last_damage_value = damage;

    if(h->linked_obj) {
        object *linked = game_state_find_object(obj->gs, h->linked_obj);
        if(linked) {
            // end the animation of the linked object, so it can go to the successor
            linked->animation_state.finished = 1;
        }
    }

    game_player *player = game_state_get_player(obj->gs, h->player_id);
    // If god mode is not on, take damage
    if(!player->god) {
        if(player->pilot->photo) {
            // in tournament mode, damage is mitigated by armor
            // (Armor + 2.5) * .25
            DEBUG("applying %f to %d modulated by armor %f", damage, h->health, 0.25f * (2.5f + player->pilot->armor));
            h->health -= damage / (0.25f * (2.5f + player->pilot->armor));
        } else {
            h->health -= damage;
        }
    }

    // Handle health changes
    if(h->health <= 0) {
        h->health = 0;
        h->endurance = 0.0f;
    }

    DEBUG("applying %f stun damage to %f", stun, h->endurance);
    h->endurance -= stun;
    if(h->endurance < 1.0f) {
        if(h->state == STATE_STUNNED) {
            // refill endurance
            h->endurance = h->endurance_max;
        } else {
            h->endurance = 0.0f;
        }
    }

    // Take a screencap of enemy har
    if(h->health == 0) {
        game_player *other_player = game_state_get_player(obj->gs, !h->player_id);
        object *other_har = game_state_find_object(obj->gs, other_player->har_obj_id);
        har_screencaps_capture(&other_player->screencaps, other_har, obj, SCREENCAP_BLOW);
    }

    // If damage is high enough, slow down the game for a bit
    // Also slow down game more for last shot
    if(damage > 24.0f || h->health == 0) {
        DEBUG("Slowdown: Slowing from %d to %d.", game_state_get_speed(obj->gs),
              h->health == 0 ? game_state_get_speed(obj->gs) - 10 : game_state_get_speed(obj->gs) - 6);
        game_state_slowdown(obj->gs, 12,
                            h->health == 0 ? game_state_get_speed(obj->gs) - 10 : game_state_get_speed(obj->gs) - 6);
    }

    // chronos' stasis does not have a hit animation
    if(str_size(string) > 0) {
        h->state = STATE_RECOIL;
        // Set hit animation
        object_set_animation(obj, &af_get_move(h->af_data, ANIM_DAMAGE)->ani);
        object_set_repeat(obj, 0);
        if(h->health <= 0) {
            // taken from MASTER.DAT
            size_t last_line = 0;
            if(!str_last_of(string, '-', &last_line)) {
                last_line = 0;
            }

            str n;
            str_from_slice(&n, string, 0, last_line);
            // XXX changed the last frame to 200 ticks to ensure the HAR falls down
            str_append_c(&n, "-x-20ox-20L1-ox-20L2-x-20zzs4l25sp13M1-zzM200");
            object_set_custom_string(obj, str_c(&n));
            str_free(&n);

            if(object_is_airborne(obj)) {
                // airborne defeat
                obj->vel.y = -7;
                object_set_stride(obj, 1);
                h->state = STATE_FALLEN;
            }
        } else if(object_is_airborne(obj)) {
            DEBUG("airborne knockback");
            // append the 'airborne knockback' string to the hit string, replacing the final frame
            size_t last_line = 0;
            if(!str_last_of(string, '-', &last_line)) {
                last_line = 0;
            }

            str n;
            str_from_slice(&n, string, 0, last_line);
            str_append_c(&n, "-L2-M5-L2");
            object_set_custom_string(obj, str_c(&n));
            str_free(&n);

            obj->vel.y = -7 * object_get_direction(obj);
            h->state = STATE_FALLEN;
            object_set_stride(obj, 1);
        } else {
            object_set_custom_string(obj, str_c(string));
        }
        object_dynamic_tick(obj);
        h->flinching = 1;

        // XXX hack - if the first frame has the 'k' tag, treat it as some vertical knockback
        // we can't do this in player.c because it breaks the jaguar leap, which also uses the 'k' tag.
        const sd_script_frame *frame = sd_script_get_frame(&obj->animation_state.parser, 0);
        if(frame != NULL && sd_script_isset(frame, "k")) {
            obj->vel.y -= 7;
        }
    }
}

void har_spawn_oil(object *obj, vec2i pos, int amount, float gravity, int layer) {
    har *h = object_get_userdata(obj);

    // burning oil
    for(int i = 0; i < amount; i++) {
        // Calculate velocity etc.
        float rv = rand_int(100) / 100.0f - 0.5;
        float velx = (5 * cosf(90 + i - (amount) / 2 + rv)) * object_get_direction(obj);
        float vely = -12 * sinf(i / amount + rv);

        // Make sure the oil drops have somekind of velocity
        // (to prevent floating scrap objects)
        if(vely < 0.1 && vely > -0.1)
            vely += 0.21;

        // Create the object
        object *scrap = omf_calloc(1, sizeof(object));
        int anim_no = ANIM_BURNING_OIL;
        object_create(scrap, obj->gs, pos, vec2f_create(velx, vely));
        object_set_animation(scrap, &af_get_move(h->af_data, anim_no)->ani);
        object_set_stl(scrap, object_get_stl(obj));
        object_set_gravity(scrap, gravity);
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
    har_spawn_oil(obj, pos, oil_amount, 1, RENDER_LAYER_TOP);

    // scrap metal
    // TODO this assumes the default scrap level and does not consider BIG[1-9]
    int scrap_amount = 0;
    int destr = is_destruction(obj->gs);
    if(destr) {
        scrap_amount = 30;
    } else if(amount > 11 && amount < 14) {
        scrap_amount = 1;
    } else if(amount > 13 && amount < 16) {
        scrap_amount = 2;
    } else if(amount > 15) {
        scrap_amount = 3;
    }
    for(int i = 0; i < scrap_amount; i++) {
        // Calculate velocity etc.
        float rv = rand_int(100) / 100.0f - 0.5;
        float velx = (5 * cosf(90 + i - (scrap_amount) / 2 + rv)) * object_get_direction(obj);
        float vely = -12 * sinf(i / scrap_amount + rv);

        // Make destruction moves look more impressive :P
        if(destr) {
            velx *= 5;
            vely *= 5;
        }

        // Make sure scrap has somekind of velocity
        // (to prevent floating scrap objects)
        if(vely < 0.1 && vely > -0.1)
            vely += 0.21;

        // Create the object
        object *scrap = omf_calloc(1, sizeof(object));
        int anim_no = rand_int(3) + ANIM_SCRAP_METAL;
        object_create(scrap, obj->gs, pos, vec2f_create(velx, vely));
        object_set_animation(scrap, &af_get_move(h->af_data, anim_no)->ani);
        object_set_stl(scrap, object_get_stl(obj));
        object_set_gravity(scrap, 1);
        object_set_pal_offset(scrap, object_get_pal_offset(obj));
        object_set_pal_limit(obj, object_get_pal_limit(obj));
        object_set_layers(scrap, LAYER_SCRAP);
        object_dynamic_tick(scrap);
        object_set_shadow(scrap, 1);
        scrap_create(scrap);
        game_state_add_object(obj->gs, scrap, RENDER_LAYER_TOP, 0, 0);
    }
}

void har_block(object *obj, vec2i hit_coord) {
    har *h = obj->userdata;
    if(h->state == STATE_WALKFROM) {
        object_set_animation(obj, &af_get_move(h->af_data, ANIM_STANDING_BLOCK)->ani);
    } else {
        object_set_animation(obj, &af_get_move(h->af_data, ANIM_CROUCHING_BLOCK)->ani);
    }
    // the HARs have a lame blank frame in their animation string, so use a custom one
    object_set_custom_string(obj, "A5");
    object_set_repeat(obj, 0);
    object_dynamic_tick(obj);
    // blocking spark
    if(h->damage_received) {
        // don't make another scrape
        return;
    }
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
    audio_play_sound(3, 0.7f, 0.5f, 1.0f);
    game_state_add_object(obj->gs, scrape, RENDER_LAYER_MIDDLE, 0, 0);
    h->damage_received = 1;
    if(h->state == STATE_CROUCHBLOCK) {
        h->flinching = 1;
    }
}

void har_check_closeness(object *obj_a, object *obj_b) {
    vec2i pos_a = object_get_pos(obj_a);
    vec2i pos_b = object_get_pos(obj_b);
    har *a = object_get_userdata(obj_a);
    har *b = object_get_userdata(obj_b);
    sprite *sprite_a = animation_get_sprite(obj_a->cur_animation, obj_a->cur_sprite_id);
    sprite *sprite_b = animation_get_sprite(obj_b->cur_animation, obj_b->cur_sprite_id);
    int hard_limit = 35; // Push opponent if HARs too close. Harrison-Stetson method value.
    int soft_limit = 45; // Sets HAR A as being close to HAR B if closer than this.

    if(!sprite_a || !sprite_b) {
        // no sprite, eg chronos' teleport
        return;
    }

    if(b->state == STATE_RECOIL || a->state == STATE_RECOIL || b->state == STATE_FALLEN || b->state == STATE_JUMPING ||
       a->state == STATE_DEFEAT || b->state == STATE_DEFEAT || a->state == STATE_FALLEN ||
       a->state == STATE_WALLDAMAGE || b->state == STATE_WALLDAMAGE) {
        return;
    }

    vec2i size_b = sprite_get_size(sprite_b);

    // the 50 here is to reverse the damage done in har_fix_sprite_coords
    int y1 = pos_a.y + sprite_a->pos.y + 50;
    int y2 = pos_b.y - size_b.y;

    // handle one HAR landing on top of another
    // XXX make this code less redundant
    if(a->state == STATE_JUMPING && object_get_direction(obj_a) == OBJECT_FACE_LEFT) {
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
    } else if(a->state == STATE_JUMPING && object_get_direction(obj_a) == OBJECT_FACE_RIGHT) {
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

    // Reset closeness state
    a->close = 0;
    a->hard_close = 0;

    // If HARs get too close together, handle it
    if(har_is_walking(a) && object_get_direction(obj_a) == OBJECT_FACE_LEFT) {
        if(pos_a.x < pos_b.x + hard_limit && pos_a.x > pos_b.x) {
            // don't allow hars to overlap in the corners
            if(pos_b.x > ARENA_LEFT_WALL) {
                pos_b.x = pos_a.x - hard_limit;
                object_set_pos(obj_b, pos_b);
            } else {
                pos_a.x = pos_b.x + hard_limit;
                object_set_pos(obj_a, pos_a);
            }
            a->hard_close = 1;
        }
        if(pos_a.x < pos_b.x + soft_limit && pos_a.x > pos_b.x) {
            if(b->state == STATE_STANDING || b->state == STATE_STUNNED || har_is_walking(b) || har_is_crouching(b)) {
                a->close = 1;
            }
            a->hard_close = 1;
        }
    }
    if(har_is_walking(a) && object_get_direction(obj_a) == OBJECT_FACE_RIGHT) {
        if(pos_a.x + hard_limit > pos_b.x && pos_a.x < pos_b.x) {
            // don't allow hars to overlap in the corners
            if(pos_b.x < ARENA_RIGHT_WALL) {
                pos_b.x = pos_a.x + hard_limit;
                object_set_pos(obj_b, pos_b);
            } else {
                pos_a.x = pos_b.x - hard_limit;
                object_set_pos(obj_a, pos_a);
            }
            a->hard_close = 1;
        }
        if(pos_a.x + soft_limit > pos_b.x && pos_a.x < pos_b.x) {
            if(b->state == STATE_STANDING || b->state == STATE_STUNNED || har_is_walking(b) || har_is_crouching(b)) {
                a->close = 1;
            }
            a->hard_close = 1;
        }
    }
}

#ifdef DEBUGMODE
void har_debug(object *obj) {
    har *h = object_get_userdata(obj);
    image img;
    surface_to_image(&h->cd_debug, &img);

    uint8_t c = 0xCF;
    uint8_t red = 0xCF;
    uint8_t blank = 0;

    // video_draw(&h->cd_debug, 0, 0);

    if(obj->cur_sprite_id < 0) {
        return;
    }
    // Make sure there are hitpoints to check.
    if(vector_size(&obj->cur_animation->collision_coords) == 0) {
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

    // Iterate through hitpoints
    iterator it;
    collision_coord *cc;
    vector_iter_begin(&obj->cur_animation->collision_coords, &it);

    int found = 0;
    while((cc = iter_next(&it)) != NULL) {
        if(cc->frame_index != obj->cur_sprite_id)
            continue;
        found = 1;
    }

    image_clear(&img, blank);

    if(!found) {
        return;
    }

    vector_iter_begin(&obj->cur_animation->collision_coords, &it);
    while((cc = iter_next(&it)) != NULL) {
        if(cc->frame_index != obj->cur_sprite_id)
            continue;
        image_set_pixel(&img, pos_a.x + (cc->pos.x * flip), pos_a.y + cc->pos.y, c);
        // DEBUG("%d drawing hit point at %d %d ->%d %d", obj->cur_sprite->id, pos_a.x, pos_a.y, pos_a.x + (cc->pos.x *
        // flip), pos_a.y + cc->pos.y);
    }

    image_set_pixel(&img, pos_a.x, pos_a.y, red);

    video_draw(&h->cd_debug, 0, 0);
}
#endif // DEBUGMODE

void har_collide_with_har(object *obj_a, object *obj_b, int loop) {
    har *a = object_get_userdata(obj_a);
    har *b = object_get_userdata(obj_b);

    controller *ctrl_a = game_player_get_ctrl(game_state_get_player(obj_a->gs, a->player_id));
    controller *ctrl_b = game_player_get_ctrl(game_state_get_player(obj_b->gs, b->player_id));

    if(b->state == STATE_FALLEN || b->state == STATE_STANDING_UP || b->state == STATE_WALLDAMAGE) {
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
       (intersect_sprite_hitpoint(obj_a, obj_b, level, &hit_coord) || move->category == CAT_CLOSE ||
        (player_frame_isset(obj_a, "ue") && b->state != STATE_JUMPING))) {

        if(har_is_blocking(b, move) &&
           // earthquake smash is unblockable
           !player_frame_isset(obj_a, "ue")) {
            har_event_enemy_block(a, move, false, ctrl_a);
            har_event_block(b, move, false, ctrl_b);
            har_block(obj_b, hit_coord);
            if(b->is_wallhugging) {
                vec2f push = object_get_vel(obj_a);
                push.x += 2.0f * object_get_direction(obj_b);
                object_set_vel(obj_a, push);
            } else {
                vec2f push = object_get_vel(obj_b);
                push.x += 2.0f * object_get_direction(obj_a);
                object_set_vel(obj_b, push);
            }
            return;
        }

        // is the HAR invulnerable to this kind of attack?
        if(har_is_invincible(obj_b, move)) {
            return;
        }

        vec2i hit_coord2 = vec2i_create(0, 0);

        if(b->damage_done == 0 && loop == 0 && intersect_sprite_hitpoint(obj_b, obj_a, level, &hit_coord2)) {
            DEBUG("both hars hit at the same time!");
            har_collide_with_har(obj_b, obj_a, 1);
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
            if(b->state == STATE_RECOIL || b->is_wallhugging) {
                // back the attacker off a little
                vec2f push = object_get_vel(obj_a);
                push.x += 2.0f * object_get_direction(obj_b);
                object_set_vel(obj_a, push);
            }
            if(b->is_wallhugging) {
                vec2f push = object_get_vel(obj_a);
                push.x += 3.0f * object_get_direction(obj_b);
                object_set_vel(obj_a, push);
            } else {
                vec2f push = object_get_vel(obj_b);
                push.x += 3.0f * object_get_direction(obj_a);
                object_set_vel(obj_b, push);
            }
        }

        har_take_damage(obj_b, &move->footer_string, move->damage, move->stun);

        if(hit_coord.x != 0 || hit_coord.y != 0) {
            har_spawn_scrap(obj_b, hit_coord, move->block_stun);
        }

        DEBUG("HAR %s to HAR %s collision at %d,%d!", har_get_name(a->id), har_get_name(b->id), hit_coord.x,
              hit_coord.y);
        DEBUG("HAR %s animation set to %s", har_get_name(b->id), str_c(&move->footer_string));

        if(move->next_move) {
            DEBUG("HAR %s going to next move %d", har_get_name(b->id), move->next_move);
            object_set_animation(obj_a, &af_get_move(a->af_data, move->next_move)->ani);
            object_set_repeat(obj_a, 0);
            // bail out early, the next move can still brutalize the oppopent so don't set them immune to further damage
            // this fixes flail's charging punch and katana's wall spin, but thorn's spike charge still works
            return;
        }

        a->damage_done = 1;
        b->damage_received = 1;

        if(move->category == CAT_CLOSE) {
            // never flinch from a throw
            b->flinching = 0;
            a->flinching = 0;
        }
    }
}

void har_collide_with_projectile(object *o_har, object *o_pjt) {
    har *h = object_get_userdata(o_har);
    const af *prog_owner_af_data = projectile_get_af_data(o_pjt);
    har *other = object_get_userdata(
        game_state_find_object(o_har->gs, game_state_get_player(o_har->gs, abs(h->player_id - 1))->har_obj_id));

    if(h->state == STATE_FALLEN || h->state == STATE_STANDING_UP || h->state == STATE_WALLDAMAGE) {
        // can't hit em while they're down
        return;
    }

    // Check for collisions by sprite collision points
    int level = 2;
    vec2i hit_coord;
    if(intersect_sprite_hitpoint(o_pjt, o_har, level, &hit_coord)) {
        af_move *move = af_get_move(prog_owner_af_data, o_pjt->cur_animation->id);

        controller *ctrl = game_player_get_ctrl(game_state_get_player(o_har->gs, h->player_id));
        controller *ctrl_other = game_player_get_ctrl(game_state_get_player(o_pjt->gs, other->player_id));
        if(har_is_blocking(h, move)) {
            har_event_enemy_block(other, move, true, ctrl_other);
            har_event_block(h, move, true, ctrl);
            har_block(o_har, hit_coord);
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

        DEBUG("PROJECTILE %d to HAR %s collision at %d,%d!", object_get_animation(o_pjt)->id, har_get_name(h->id),
              hit_coord.x, hit_coord.y);

        if(move->next_move) {
            DEBUG("PROJECTILE %d going to next move %d on HIT", object_get_animation(o_pjt)->id, move->next_move);
            object_set_animation(o_pjt, &af_get_move(prog_owner_af_data, move->next_move)->ani);
            object_set_repeat(o_pjt, 0);
            return;
        }

        vec2f vel = object_get_vel(o_har);
        vel.x = 0.0f;
        object_set_vel(o_har, vel);

        // Just take damage normally if there is no footer string in successor
        DEBUG("projectile dealt damage of %f", move->damage);
        har_take_damage(o_har, &move->footer_string, move->damage, move->stun);

        projectile_mark_hit(o_pjt);

        har_event_take_hit(h, move, true, ctrl);
        har_event_land_hit(other, move, true, ctrl_other);

        har_spawn_scrap(o_har, hit_coord, move->block_stun);
        h->damage_received = 1;

        // Exception case for chronos' time freeze
        if(player_frame_isset(o_pjt, "af")) {
            h->in_stasis_ticks = 75;
        }

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
            DEBUG("SUCCESSOR: Selecting anim %d with string %s", object_get_animation(o_pjt)->id,
                  str_c(&object_get_animation(o_pjt)->animation_string));
        }
    }
}

void har_collide_with_hazard(object *o_har, object *o_hzd) {
    har *h = object_get_userdata(o_har);
    bk *bk_data = object_get_userdata(o_hzd);
    bk_info *anim = bk_get_info(bk_data, o_hzd->cur_animation->id);

    if(h->state == STATE_FALLEN || h->state == STATE_STANDING_UP) {
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
        har_take_damage(o_har, &anim->footer_string, anim->hazard_damage, (anim->hazard_damage + 6) * 512);
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
    har_check_closeness(obj_b, obj_a);

    // Handle har collisions
    har_collide_with_har(obj_a, obj_b, 0);
    har_collide_with_har(obj_b, obj_a, 0);
}

void har_palette_transform(damage_tracker *damage, vga_palette *pal, void *obj) {
    har *h = object_get_userdata(obj);
    int step, max;
    vga_color ref, tmp;

    // Select palette start and length.
    // For player 0, we should use palette indexes 0-47. For player 1, 48-96.
    // If pe flag is on, we need to switch to handling the other HAR.
    int pal_start = 48 * (h->player_id ^ h->p_har_switch);
    int pal_length = 47 + h->player_id;

    // Handle palette transformation
    ref = pal->colors[h->p_pal_ref];
    step = (h->p_color_ref * 4) * ((float)h->p_ticks_left / (float)h->p_ticks_length);
    if(h->p_color_fn) {
        for(int i = pal_start; i < pal_start + pal_length; i++) {
            max = max3(pal->colors[i].r, pal->colors[i].g, pal->colors[i].b);
            tmp.r = (max * (step * (ref.r * pal->colors[i].r) / 255.0f) / 255.0f) * pal->colors[i].r;
            tmp.g = (max * (step * (ref.g * pal->colors[i].g) / 255.0f) / 255.0f) * pal->colors[i].g;
            tmp.b = (max * (step * (ref.b * pal->colors[i].b) / 255.0f) / 255.0f) * pal->colors[i].b;
            pal->colors[i].r = min2(max2(tmp.r, 0), 255);
            pal->colors[i].g = min2(max2(tmp.g, 0), 255);
            pal->colors[i].b = min2(max2(tmp.b, 0), 255);
        }
    } else {
        for(int i = pal_start; i < pal_start + pal_length; i++) {
            tmp.r = (step * ref.r) / 255.0f + (255 - step) * (pal->colors[i].r / 255.0f);
            tmp.g = (step * ref.g) / 255.0f + (255 - step) * (pal->colors[i].g / 255.0f);
            tmp.b = (step * ref.b) / 255.0f + (255 - step) * (pal->colors[i].b / 255.0f);
            pal->colors[i].r = min2(max2(tmp.r, 0), 255);
            pal->colors[i].g = min2(max2(tmp.g, 0), 255);
            pal->colors[i].b = min2(max2(tmp.b, 0), 255);
        }
    }

    // Mark the palette as damaged.
    damage_set_range(damage, pal_start, pal_start + pal_length);
}

void har_tick(object *obj) {
    har *h = object_get_userdata(obj);
    controller *ctrl = game_player_get_ctrl(game_state_get_player(obj->gs, h->player_id));

    if(h->p_ticks_left > 0) {
        object_set_palette_transform_cb(obj, har_palette_transform);
        h->p_ticks_left--;
    } else {
        object_set_palette_transform_cb(obj, NULL);
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
    h->is_grabbed = player_frame_isset(obj, "e");

    // Make sure HAR doesn't walk through walls
    // TODO: Roof!
    vec2i pos = object_get_pos(obj);
    if(h->state != STATE_DEFEAT) {
        int wall_flag = player_frame_isset(obj, "aw");
        int wall = 0;
        int hit = 0;
        if(pos.x < ARENA_LEFT_WALL) {
            pos.x = ARENA_LEFT_WALL;
            hit = 1;
        } else if(pos.x > ARENA_RIGHT_WALL) {
            pos.x = ARENA_RIGHT_WALL;
            wall = 1;
            hit = 1;
        }
        object_set_pos(obj, pos);

        if(hit && wall_flag) {
            af_move *move = af_get_move(h->af_data, obj->cur_animation->id);
            if(move->next_move) {
                DEBUG("wall hit chaining to next animation %d", move->next_move);
                har_set_ani(obj, move->next_move, 0);
            }
        }

        if(hit) {
            har_event_hit_wall(h, wall, ctrl);
        }
    }

    // Check for HAR specific palette tricks
    if(player_frame_isset(obj, "ptr")) {
        h->p_pal_ref = player_frame_isset(obj, "pd") ? player_frame_get(obj, "pd") : 0;
        h->p_har_switch = player_frame_isset(obj, "pe");
        h->p_color_ref = player_frame_get(obj, "ptr");
        h->p_ticks_length = player_frame_isset(obj, "pp") ? player_frame_get(obj, "pp") : 0;
        h->p_ticks_left = h->p_ticks_length;
        h->p_color_fn = player_frame_isset(obj, "pa");
    }

    // Object took walldamage, but has now landed
    if(h->state == STATE_WALLDAMAGE && !object_is_airborne(obj)) {
        h->state = STATE_FALLEN;
    }

    // Reset air_attacked when not in the air to prevent HAR from freezing
    if(!object_is_airborne(obj) && h->air_attacked) {
        har_event_air_attack_done(h, ctrl);
        h->air_attacked = 0;
    }

    if((h->state == STATE_DONE) && player_is_last_frame(obj) && obj->animation_state.entered_frame == 1) {
        // match is over
        har_event_done(h, ctrl);
    }

    if(pos.y < ARENA_FLOOR && h->state == STATE_RECOIL) {
        DEBUG("switching to fallen");
        h->state = STATE_FALLEN;
        har_event_recover(h, ctrl);
    }

    if(h->state == STATE_STUNNED) {
        h->stun_timer++;
        if(h->stun_timer % 10 == 0) {
            vec2i pos = object_get_pos(obj);
            pos.y -= 60;
            har_spawn_oil(obj, pos, 5, 0.5f, RENDER_LAYER_BOTTOM);
        }
        if(h->stun_timer > 100) {
            har_stunned_done(obj);
        }
    }

    // Stop HAR from sliding if touching the ground
    if(h->state != STATE_JUMPING && h->state != STATE_FALLEN && h->state != STATE_RECOIL) {

        // af_move *move = af_get_move(h->af_data, obj->cur_animation->id);
        if(h->state == STATE_CROUCHBLOCK) {
            vec2f vel = object_get_vel(obj);
            if(vel.x != 0.0f) {
                vel.x -= 0.2f * -object_get_direction(obj);
            }
            object_set_vel(obj, vel);
        } else if(!har_is_walking(h) && h->executing_move == 0) {
            vec2f vel = object_get_vel(obj);
            vel.x = 0;
            object_set_vel(obj, vel);
        }
    }

    // Endurance restore
    if(h->endurance < h->endurance_max &&
       !(h->executing_move || h->state == STATE_RECOIL || h->state == STATE_STUNNED || h->state == STATE_FALLEN ||
         h->state == STATE_STANDING_UP || h->state == STATE_DEFEAT)) {
        h->endurance += 0.025f; // made up but plausible number
    }

    // Leave shadow trail
    // IF trail is on, copy current sprite to a new animation, and set animation string
    // to show the sprite with animation string that interpolates opacity down
    // Mark new object as the owner of the animation, so that the animation gets
    // removed when the object is finished.
    if(object_has_effect(obj, EFFECT_TRAIL) && obj->age % 2 == 0) {
        sprite *cur_sprite = animation_get_sprite(obj->cur_animation, obj->cur_sprite_id);
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

    // Network motion replay
    int act_pos = obj->age % OBJECT_EVENT_BUFFER_SIZE;
    if(h->act_buf[act_pos].age == obj->age) {
        DEBUG("REPLAYING %d inputs", h->act_buf[act_pos].count);
        for(int i = 0; i < h->act_buf[act_pos].count; i++) {
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
    memmove((buf) + 1, buf, 9);
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
        case ACT_UP | ACT_RIGHT:
            if(direction == OBJECT_FACE_LEFT) {
                add_input_to_buffer(buf, '7');
            } else {
                add_input_to_buffer(buf, '9');
            }
            break;
        case ACT_UP | ACT_LEFT:
            if(direction == OBJECT_FACE_LEFT) {
                add_input_to_buffer(buf, '9');
            } else {
                add_input_to_buffer(buf, '7');
            }
            break;
        case ACT_DOWN | ACT_RIGHT:
            if(direction == OBJECT_FACE_LEFT) {
                add_input_to_buffer(buf, '1');
            } else {
                add_input_to_buffer(buf, '3');
            }
            break;
        case ACT_DOWN | ACT_LEFT:
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

af_move *match_move(object *obj, char *inputs) {
    har *h = object_get_userdata(obj);
    af_move *move = NULL;
    size_t len;
    for(int i = 0; i < 70; i++) {
        if((move = af_get_move(h->af_data, i))) {
            len = move->move_string.len;
            if(!strncmp(str_c(&move->move_string), inputs, len)) {
                if(move->category == CAT_CLOSE && h->close != 1) {
                    // not standing close enough
                    continue;
                }
                if(move->category == CAT_JUMPING && h->state != STATE_JUMPING) {
                    // not jumping
                    continue;
                }
                if(move->category != CAT_JUMPING && h->state == STATE_JUMPING) {
                    // jumping but this move is not a jumping move
                    continue;
                }
                if(move->category == CAT_SCRAP && h->state != STATE_VICTORY) {
                    continue;
                }

                if(move->category == CAT_DESTRUCTION && h->state != STATE_SCRAP) {
                    continue;
                }

                if(move->category == CAT_FIRE_ICE) {
                    continue;
                }

                if(h->state != STATE_JUMPING && move->pos_constraints & 0x2) {
                    DEBUG("Position contraint prevents move when not jumping!");
                    // required to be jumping
                    continue;
                }
                if(h->is_wallhugging != 1 && move->pos_constraints & 0x1) {
                    DEBUG("Position contraint prevents move when not wallhugging!");
                    // required to be wall hugging
                    continue;
                }

                if(h->executing_move && !h->enqueued) {
                    // check if the current frame allows chaining
                    int allowed = 0;
                    if(player_frame_isset(obj, "jn") && i == player_frame_get(obj, "jn")) {
                        allowed = 1;
                    } else {
                        switch(move->category) {
                            case CAT_LOW:
                                if(player_frame_isset(obj, "jl")) {
                                    allowed = 1;
                                }
                                break;
                            case CAT_MEDIUM:
                                if(player_frame_isset(obj, "jm")) {
                                    allowed = 1;
                                }
                                break;
                            case CAT_HIGH:
                                if(player_frame_isset(obj, "jh")) {
                                    allowed = 1;
                                }
                                break;
                            case CAT_SCRAP:
                                if(player_frame_isset(obj, "jf")) {
                                    allowed = 1;
                                }
                                break;
                            case CAT_DESTRUCTION:
                                if(player_frame_isset(obj, "jf2")) {
                                    allowed = 1;
                                }
                                break;
                        }
                    }
                    if(player_get_current_tick(obj) >= player_get_len_ticks(obj)) {
                        DEBUG("enqueueing %d %s", i, str_c(&move->move_string));
                        h->enqueued = i;
                        return NULL;
                    }

                    if(!allowed) {
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

af_move *scrap_destruction_cheat(object *obj, char *inputs) {
    har *h = object_get_userdata(obj);
    for(int i = 0; i < 70; i++) {
        af_move *move;
        if((move = af_get_move(h->af_data, i))) {
            if(move->category == CAT_SCRAP && h->state == STATE_VICTORY && inputs[0] == 'K') {
                return move;
            }

            if(move->category == CAT_DESTRUCTION && h->state == STATE_SCRAP && inputs[0] == 'P') {
                return move;
            }
        }
    }
    return NULL;
}

int maybe_har_change_state(int oldstate, int direction, int act_type) {
    int state = 0;
    switch(act_type) {
        case ACT_DOWN | ACT_RIGHT:
            if(direction == OBJECT_FACE_LEFT) {
                state = STATE_CROUCHBLOCK;
            } else {
                state = STATE_CROUCHING;
            }
            break;
        case ACT_DOWN | ACT_LEFT:
            if(direction == OBJECT_FACE_RIGHT) {
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
            if(direction == OBJECT_FACE_LEFT) {
                state = STATE_WALKTO;
            } else {
                state = STATE_WALKFROM;
            }
            break;
        case ACT_RIGHT:
            if(direction == OBJECT_FACE_RIGHT) {
                state = STATE_WALKTO;
            } else {
                state = STATE_WALKFROM;
            }
            break;
        case ACT_UP:
            state = STATE_JUMPING;
            break;
        case ACT_UP | ACT_LEFT:
            state = STATE_JUMPING;
            break;
        case ACT_UP | ACT_RIGHT:
            state = STATE_JUMPING;
            break;
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

    int direction = object_get_direction(obj);
    // always queue input, I guess
    add_input(h->inputs, act_type, direction);

    if(!(h->state == STATE_STANDING || har_is_walking(h) || har_is_crouching(h) || h->state == STATE_JUMPING ||
         h->state == STATE_VICTORY || h->state == STATE_SCRAP) ||
       object_get_halt(obj)) {
        // doing something else, ignore input
        return 0;
    }

    // Don't allow movement if arena is starting or ending
    int arena_state = arena_get_state(game_state_get_scene(obj->gs));
    if(arena_state == ARENA_STATE_STARTING) {
        return 0;
    }

    // don't allow multiple special moves in the air
    if(h->air_attacked) {
        return 0;
    }

    int oldstate = h->state;

    af_move *move = match_move(obj, h->inputs);

    if(game_state_get_player(obj->gs, h->player_id)->ez_destruct && move == NULL &&
       (h->state == STATE_VICTORY || h->state == STATE_SCRAP)) {
        move = scrap_destruction_cheat(obj, h->inputs);
    }

    if(move) {
        char *s = (char *)str_c(&move->move_string); // start
        for(int j = str_size(&move->move_string) - 1; j >= 0; j--) {
            switch(s[j]) {
                case '1':
                    if(direction == OBJECT_FACE_LEFT) {
                        har_action_hook(obj, ACT_DOWN | ACT_RIGHT);
                    } else {
                        har_action_hook(obj, ACT_DOWN | ACT_LEFT);
                    }
                    break;
                case '2':
                    har_action_hook(obj, ACT_DOWN);
                    break;
                case '3':
                    if(direction == OBJECT_FACE_LEFT) {
                        har_action_hook(obj, ACT_DOWN | ACT_LEFT);
                    } else {
                        har_action_hook(obj, ACT_DOWN | ACT_RIGHT);
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
                        har_action_hook(obj, ACT_UP | ACT_RIGHT);
                    } else {
                        har_action_hook(obj, ACT_UP | ACT_LEFT);
                    }
                    break;
                case '8':
                    har_action_hook(obj, ACT_UP);
                    break;
                case '9':
                    if(direction == OBJECT_FACE_LEFT) {
                        har_action_hook(obj, ACT_UP | ACT_LEFT);
                    } else {
                        har_action_hook(obj, ACT_UP | ACT_RIGHT);
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
        // Set correct animation etc.
        // executing_move = 1 prevents new moves while old one is running.
        har_set_ani(obj, move->id, 0);
        h->inputs[0] = '\0';
        h->executing_move = 1;

        // Move flag is on -- make the HAR move backwards to avoid overlap.
        if(move->collision_opts & 0x20) {
            obj->pos.x -= object_get_size(obj).x / 2 * object_get_direction(obj);
        }

        // Stop horizontal movement, when move is done
        // TODO: Make this work better
        vec2f spd = object_get_vel(obj);
        if(h->state != STATE_JUMPING) {
            spd.x = 0.0f;
        }
        object_set_vel(obj, spd);

        // Prefetch enemy object & har links, they may be needed
        object *enemy_obj =
            game_state_find_object(obj->gs, game_player_get_har_obj_id(game_state_get_player(obj->gs, !h->player_id)));
        har *enemy_har = (har *)enemy_obj->userdata;

        // If animation is scrap or destruction, then remove our customizations
        // from gravity/fall speed, and just use the HARs native value.
        if(move->category == CAT_SCRAP || move->category == CAT_DESTRUCTION) {
            obj->horizontal_velocity_modifier = 1.0f;
            obj->vertical_velocity_modifier = 1.0f;
            object_set_gravity(obj, h->af_data->fall_speed);
            object_set_gravity(enemy_obj, enemy_har->af_data->fall_speed);
        }

        if(move->category == CAT_SCRAP) {
            DEBUG("going to scrap state");
            h->state = STATE_SCRAP;
            har_event_scrap(h, ctrl);
        } else if(move->category == CAT_DESTRUCTION) {
            DEBUG("going to destruction state");
            h->state = STATE_DESTRUCTION;
            har_event_destruction(h, ctrl);
        } else {
            har_event_attack(h, move, ctrl);
        }

        // make the other har participate in the scrap/destruction
        if(move->category == CAT_SCRAP || move->category == CAT_DESTRUCTION) {
            af_move *move = af_get_move(h->af_data, obj->cur_animation->id);
            object_set_animation(enemy_obj, &af_get_move(enemy_har->af_data, ANIM_DAMAGE)->ani);
            object_set_repeat(enemy_obj, 0);
            object_set_custom_string(enemy_obj, str_c(&move->footer_string));
            object_dynamic_tick(enemy_obj);
        }

        // we actually did something interesting
        // return 1 so we can use this as sync point for netplay
        return 1;
    }

    // Don't allow new movement while we're still executing a move
    if(h->executing_move) {
        if(obj->pos.y < ARENA_FLOOR) {
            // XXX I think 'i' is for 'not interruptable'
            if(h->state < STATE_JUMPING && !player_frame_isset(obj, "i")) {
                DEBUG("standing move led to airborne one");
                h->state = STATE_JUMPING;
            } else if(h->state != STATE_JUMPING) {
                DEBUG("state is %d", h->state);
            }
        }
        return 0;
    }

    if(obj->pos.y < ARENA_FLOOR) {
        // airborne

        // Send an event if the har tries to turn in the air by pressing either left/right/downleft/downright
        int opp_id = h->player_id ? 0 : 1;
        object *opp =
            game_state_find_object(obj->gs, game_player_get_har_obj_id(game_state_get_player(obj->gs, opp_id)));
        if(act_type == ACT_LEFT || act_type == ACT_RIGHT || act_type == (ACT_DOWN | ACT_LEFT) ||
           act_type == (ACT_DOWN | ACT_RIGHT)) {
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

    if(arena_state == ARENA_STATE_ENDING) {
        return 0;
    }

    float vx, vy;
    // no moves matched, do player movement
    int newstate;
    if((newstate = maybe_har_change_state(h->state, direction, act_type))) {
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
                vx = h->fwd_speed * direction;
                object_set_vel(obj, vec2f_create(vx * (h->hard_close ? 0.5 : 1.0), 0));
                object_set_stride(obj, h->stride);
                har_event_walk(h, 1, ctrl);
                break;
            case STATE_WALKFROM:
                har_set_ani(obj, ANIM_WALKING, 1);
                vx = h->back_speed * direction * -1;
                object_set_vel(obj, vec2f_create(vx * (h->hard_close ? 0.5 : 1.0), 0));
                object_set_stride(obj, h->stride);
                har_event_walk(h, -1, ctrl);
                break;
            case STATE_JUMPING:
                har_set_ani(obj, ANIM_JUMPING, 0);
                vx = 0.0f;
                vy = h->jump_speed;
                int jump_dir = 0;
                if((act_type == (ACT_UP | ACT_LEFT) && direction == OBJECT_FACE_LEFT) ||
                   (act_type == (ACT_UP | ACT_RIGHT) && direction == OBJECT_FACE_RIGHT)) {
                    vx = (h->fwd_speed * direction);
                    object_set_tick_pos(obj, 110);
                    object_set_stride(obj, 7); // Pass 7 frames per tick
                    jump_dir = 1;
                } else if(act_type == (ACT_UP | ACT_LEFT) || act_type == (ACT_UP | ACT_RIGHT)) {
                    // If we are jumping backwards, start animation from end
                    // at -100 frames (seems to be about right)
                    object_set_playback_direction(obj, PLAY_BACKWARDS);
                    object_set_tick_pos(obj, -110);
                    vx = (h->back_speed * direction * -1);
                    object_set_stride(obj, 7); // Pass 7 frames per tick
                    jump_dir = -1;
                } else if(act_type == ACT_UP) {
                    // If we are jumping upwards
                    object_set_tick_pos(obj, 110);
                    if(h->id == HAR_GARGOYLE) {
                        object_set_stride(obj, 7);
                    }
                }
                if(oldstate == STATE_CROUCHING || oldstate == STATE_CROUCHBLOCK) {
                    // jumping frop crouch makes you jump 25% higher
                    vy = h->superjump_speed;
                }
                object_set_vel(obj, vec2f_create(vx, vy));
                har_event_jump(h, jump_dir, ctrl);
                break;
        }
        har_action_hook(obj, act_type);
        return 1;
    }

    // if enemy is airborn we fire extra walk event to check whether we need to turn
    // fixes some rare behaviour where you cannot kick-counter someone who jumps over you
    int opp_id = h->player_id ? 0 : 1;
    object *opp = game_state_find_object(obj->gs, game_player_get_har_obj_id(game_state_get_player(obj->gs, opp_id)));
    if(object_is_airborne(opp)) {
        har_event_walk(h, 1, ctrl);
    }

    return 0;
}

void har_finished(object *obj) {
    har *h = object_get_userdata(obj);
    controller *ctrl = game_player_get_ctrl(game_state_get_player(obj->gs, h->player_id));
    if(h->enqueued) {
        DEBUG("playing enqueued animation %d", h->enqueued);
        har_set_ani(obj, h->enqueued, 0);
        h->enqueued = 0;
        h->executing_move = 1;
        return;
    } else if(h->state == STATE_SCRAP || h->state == STATE_DESTRUCTION) {
        // play vistory animation again, but do not allow any more moves to be executed
        h->state = STATE_DONE;
        har_set_ani(obj, ANIM_VICTORY, 0);
    } else if(h->state == STATE_VICTORY || h->state == STATE_DONE) {
        // end the arena
        DEBUG("ending arena!");
        game_state_set_next(obj->gs, SCENE_MENU);
    } else if(h->state == STATE_RECOIL && h->health <= 0) {
        h->state = STATE_DEFEAT;
        har_set_ani(obj, ANIM_DEFEAT, 0);
        har_event_defeat(h, ctrl);
    } else if((h->state == STATE_RECOIL || h->state == STATE_STANDING_UP) && h->endurance < 1.0f) {
        if(h->state == STATE_RECOIL) {
            har_event_recover(h, ctrl);
        }
        h->state = STATE_STUNNED;
        h->stun_timer = 0;
        har_set_ani(obj, ANIM_STUNNED, 1);
        har_event_stun(h, ctrl);

        // fire enemy stunned event
        object *enemy_obj =
            game_state_find_object(obj->gs, game_player_get_har_obj_id(game_state_get_player(obj->gs, !h->player_id)));
        har *enemy_h = object_get_userdata(enemy_obj);
        controller *enemy_ctrl = game_player_get_ctrl(game_state_get_player(enemy_obj->gs, enemy_h->player_id));
        har_event_enemy_stun(enemy_h, enemy_ctrl);
    } else if(h->state == STATE_RECOIL) {
        har_event_recover(h, ctrl);
        h->state = STATE_STANDING;
        har_set_ani(obj, ANIM_IDLE, 1);
    } else if(h->state != STATE_CROUCHING && h->state != STATE_CROUCHBLOCK) {
        // Don't transition to standing state while in midair
        if(object_is_airborne(obj) && h->state == STATE_FALLEN) {
            // XXX if we don't switch to STATE_JUMPING after getting damaged in the air, then the HAR_LAND_EVENT will
            // never get fired.
            h->state = STATE_JUMPING;
            har_set_ani(obj, ANIM_JUMPING, 0);
        } else if(h->state != STATE_JUMPING) {
            h->state = STATE_STANDING;
            har_set_ani(obj, ANIM_IDLE, 1);
        } else {
            har_set_ani(obj, ANIM_IDLE, 1);
        }
    } else {
        har_set_ani(obj, ANIM_CROUCHING, 1);
        object_set_vel(obj, vec2f_create(0, 0));
    }
    h->executing_move = 0;
    h->flinching = 0;
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

int har_clone(object *src, object *dst) {
    har *local = omf_calloc(1, sizeof(har));
    memcpy(local, object_get_userdata(src), sizeof(har));
    list_create(&local->har_hooks);
    object_set_userdata(dst, local);
    object_set_spawn_cb(dst, cb_har_spawn_object, local);
    return 0;
}

int har_clone_free(object *obj) {
    har *har = object_get_userdata(obj);
    list_free(&har->har_hooks);
    omf_free(har);
    object_set_userdata(obj, NULL);
    return 0;
}

void har_bootstrap(object *obj) {
    obj->clone = har_clone;
    obj->clone_free = har_clone_free;
}

void har_copy_actions(object *new, object *old) {
    har *h_new = object_get_userdata(new);
    har *h_old = object_get_userdata(old);
    memcpy(h_new->act_buf, h_old->act_buf, sizeof(action_buffer) * OBJECT_EVENT_BUFFER_SIZE);
}

int har_create(object *obj, af *af_data, int dir, int har_id, int pilot_id, int player_id) {
    // Create local data
    har *local = omf_calloc(1, sizeof(har));
    object_set_userdata(obj, local);
    har_bootstrap(obj);

    game_player *gp = game_state_get_player(obj->gs, player_id);
    local->af_data = af_data;

    // Save har id
    local->id = har_id;
    local->player_id = player_id;
    local->pilot_id = pilot_id;
    sd_pilot *pilot = gp->pilot;

    // Health, endurance
    // HP is
    // (HAR hp * (Pilot Endurance + 25) / 35) * 1.1
    local->health_max = local->health = (af_data->health * (pilot->endurance + 25) / 35) * 1.1;
    // DEBUG("HAR health is %d with pilot endurance %d and base health %d", local->health, pilot->endurance,
    // af_data->health);
    //  The stun cap is calculated as follows
    //  HAR Endurance * 3.6 * (Pilot Endurance + 16) / 23
    local->endurance_max = local->endurance = af_data->endurance * 3.6 * (pilot->endurance + 16) / 23;
    DEBUG("HAR endurance is %f with pilot endurance %d and base endurance %f", local->endurance, pilot->endurance,
          af_data->endurance);
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
    local->jump_speed = (((float)gp->pilot->agility + 35) / 45) * af_data->jump_speed * 216 / 256;
    local->superjump_speed = (((float)gp->pilot->agility + 35) / 45) * af_data->jump_speed * 266 / 256;
    local->fall_speed = (((float)gp->pilot->agility + 20) / 30) * af_data->fall_speed;
    local->fwd_speed = (((float)gp->pilot->agility + 20) / 30) * af_data->forward_speed;
    local->back_speed = (((float)gp->pilot->agility + 20) / 30) * af_data->reverse_speed;
    // TODO calculate a better value here
    local->stride = lrint(1 + (gp->pilot->agility / 20));
    DEBUG("setting HAR stride to %d", local->stride);
    local->close = 0;
    local->hard_close = 0;
    local->state = STATE_STANDING;
    local->executing_move = 0;
    local->air_attacked = 0;
    local->is_wallhugging = 0;
    local->is_grabbed = 0;

    local->in_stasis_ticks = 0;

    local->delay = 0;

    local->enqueued = 0;

    local->action_hook_cb = NULL;
    local->action_hook_cb_data = NULL;

    // Last damage value, for convenience
    local->last_damage_value = 0.0f;

    // p<x> stuff
    local->p_ticks_left = 0;
    local->p_ticks_length = 0;

    /*local->hook_cb = NULL;*/
    /*local->hook_cb_data = NULL;*/

    list_create(&local->har_hooks);

    local->stun_timer = 0;

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
    object_set_spawn_cb(obj, cb_har_spawn_object, local);

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
    surface_create(&local->cd_debug, 320, 200, SPRITE_TRANSPARENT_INDEX);
    surface_clear(&local->cd_debug);
#endif

    for(int i = 0; i < OBJECT_EVENT_BUFFER_SIZE; i++) {
        local->act_buf[i].count = 0;
        local->act_buf[i].age = 0;
    }

    // fixup a bunch of stuff based on player stats

    bool is_tournament = false;
    float leg_power = 0.0f;
    float arm_power = 0.0f;
    // cheap way to check if we're in tournament mode
    if(pilot->photo != NULL) {
        is_tournament = true;
        // (Limb Power + 3) * .192
        leg_power = (pilot->leg_power + 3) * 0.192f;
        arm_power = (pilot->arm_power + 3) * 0.192f;
    }

    af_move *move;
    // apply pilot stats and HAR upgrades/enhancements to the HAR
    for(int i = 0; i < MAX_AF_MOVES; i++) {
        move = af_get_move(af_data, i);
        if(move != NULL && move->damage) {
            if(!is_tournament) {
                // Single Player
                // Damage = Base Damage * (20 + Power) / 30 + 1
                //  Stun = (Base Damage + 6) * 512
                move->stun = (move->damage + 6) * 512;
                move->damage = move->damage * (20 + pilot->power) / 30 + 1;
            } else {
                // Tournament Mode
                // Damage = (Base Damage * (25 + Power) / 35 + 1) * leg/arm power / armor
                // Stun = ((Base Damage * (35 + Power) / 45) * 2 + 12) * 256
                move->stun = ((move->damage * (35 + pilot->power) / 45) * 2 + 12) * 256;
                switch(move->extra_string_selector) {
                    case 0:
                        break;
                    case 1:
                        // arm speed and power
                        move->damage = (move->damage * (25 + pilot->power) / 35 + 1) * arm_power;
                        if(move->ani.extra_string_count > 0) {
                            str_free(&move->ani.animation_string);
                            // sometimes there's not enough extra strings, so take the last available
                            str_from(&move->ani.animation_string,
                                     vector_get(&move->ani.extra_strings,
                                                min2(pilot->arm_speed, move->ani.extra_string_count - 1)));
                        }
                        break;
                    case 2:
                        // leg speed and power
                        move->damage = (move->damage * (25 + pilot->power) / 35 + 1) * leg_power;
                        if(move->ani.extra_string_count > 0) {
                            str_free(&move->ani.animation_string);
                            // sometimes there's not enough extra strings, so take the last available
                            str_from(&move->ani.animation_string,
                                     vector_get(&move->ani.extra_strings,
                                                min2(pilot->leg_speed, move->ani.extra_string_count - 1)));
                        }
                        break;
                    case 3:
                        // check if you have the enhancement(s) (and apply arm power for damage)
                        move->damage = (move->damage * (25 + pilot->power) / 35 + 1) * arm_power;
                        // TODO if you have 1 enhancement choose extra string 2
                        // if you have 2 enhancements choose extra string 3
                        break;
                    case 4:
                        // check if you have the enhancement(s) (and apply leg power for damage)
                        move->damage = (move->damage * (25 + pilot->power) / 35 + 1) * leg_power;
                        // TODO if you have 1 enhancement choose extra string 2
                        // if you have 2 enhancements choose extra string 3
                        break;
                    case 5:
                        // check if you have the enhancement(s) (and apply leg and arm power for damage)
                        move->damage = (move->damage * (25 + pilot->power) / 35 + 1) * leg_power * leg_power;
                        // TODO if you have 1 enhancement choose extra string 2
                        // if you have 2 enhancements choose extra string 3
                        break;
                }
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
    h->endurance = h->endurance_max;

    h->in_stasis_ticks = 0;

    h->enqueued = 0;

    har_set_ani(obj, ANIM_IDLE, 1);
    object_set_stride(obj, h->stride);
}

uint8_t har_player_id(object *obj) {
    har *h = object_get_userdata(obj);
    return h->player_id;
}

int16_t har_health_percent(har *h) {
    return 100 * h->health / h->health_max;
}
