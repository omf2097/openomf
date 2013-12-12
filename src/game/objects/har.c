#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "game/objects/har.h"
#include "game/objects/scrap.h"
#include "game/objects/projectile.h"
#include "game/protos/intersect.h"
#include "game/scenes/arena.h"
#include "game/game_state.h"
#include "resources/af_loader.h"
#include "resources/ids.h"
#include "resources/animation.h"
#include "controller/controller.h"
#include "utils/log.h"

// For debug layer rendering
#ifdef DEBUGMODE
#include "video/video.h"
#endif

void har_free(object *obj) {
    har *h = object_get_userdata(obj);
    af_free(&h->af_data);
#ifdef DEBUGMODE
    texture_free(&h->debug_tex);
    image_free(&h->debug_img);
#endif
    free(h);
}

// Simple helper function
void har_set_ani(object *obj, int animation_id, int repeat) {
    har *h = object_get_userdata(obj);
    object_set_animation(obj, &af_get_move(&h->af_data, animation_id)->ani);
    object_set_repeat(obj, repeat);
    object_tick(obj);
    h->damage_done = 0;
    h->damage_received = 0;
    h->executing_move = 0;
}

// Callback for spawning new objects, eg. projectiles
void cb_har_spawn_object(object *parent, int id, vec2i pos, int g, void *userdata) {
    har *h = userdata;

    vec2i npos;
    npos.x = parent->pos.x + pos.x;
    npos.y = parent->pos.y + pos.y;

    // If this is a scrap item, handle it as such ...
    if(id == ANIM_SCRAP_METAL || id == ANIM_BOLT || id == ANIM_SCREW) {
        // Calculate velocity etc.
        float velx, vely;
        float rv = (rand() % 100) / 100.0f - 0.5;
        velx = 5 * cos(70 + rv);
        vely = -3 * sin(rv);
        if(vely < 0.1 && vely > -0.1)
            vely += 0.21;

        // Create the object
        object *scrap = malloc(sizeof(object));
        object_create(scrap, npos, vec2f_create(velx, vely));
        object_set_animation(scrap, &af_get_move(&h->af_data, id)->ani);
        object_set_palette(scrap, object_get_palette(parent), 0);
        object_set_stl(scrap, object_get_stl(parent));
        object_set_repeat(scrap, 1);
        object_set_gravity(scrap, 1);
        object_set_layers(scrap, LAYER_SCRAP);
        object_tick(scrap);
        scrap_create(scrap);
        game_state_add_object(scrap);
        return;
    }

    // ... otherwise expect it is a projectile
    af_move *move = af_get_move(&h->af_data, id);
    if(move != NULL) {
        npos.x += move->ani.start_pos.x;
        npos.y += move->ani.start_pos.y;
        object *obj = malloc(sizeof(object));
        object_create(obj, npos, vec2f_create(0,0));
        object_set_userdata(obj, h);
        object_set_stl(obj, object_get_stl(parent));
        object_set_palette(obj, object_get_palette(parent), 0);
        object_set_animation(obj, &move->ani);
        object_set_gravity(obj, g/50);
        // Set all projectiles to their own layer + har layer
        object_set_layers(obj, LAYER_PROJECTILE|(h->player_id == 0 ? LAYER_HAR2 : LAYER_HAR1)); 
        // To avoid projectile-to-projectile collisions, set them to same group
        object_set_group(obj, GROUP_PROJECTILE); 
        object_set_repeat(obj, 0);
        object_set_direction(obj, object_get_direction(parent));
        projectile_create(obj);
        game_state_add_object(obj);
    }
}

void har_move(object *obj) {
    vec2f vel = object_get_vel(obj);
    obj->pos.x += vel.x;
    obj->pos.y += (vel.y*0.003);
    if(obj->pos.y > 190) {
        // We collided with ground, so set vertical velocity to 0 and
        // make sure object is level with ground
        obj->pos.y = 190;
        object_set_vel(obj, vec2f_create(vel.x, 0));

        // Change animation from jump to walk or idle,
        // depending on horizontal velocity
        har *h = object_get_userdata(obj);
        if(h->state == STATE_JUMPING) {
            if(object_get_hstate(obj) == OBJECT_MOVING) {
                h->state = STATE_WALKING;
                har_set_ani(obj, ANIM_WALKING, 1);
            } else {
                h->state = STATE_STANDING;
                har_set_ani(obj, ANIM_IDLE, 1);
            }
        }
    } else {
        object_set_vel(obj, vec2f_create(vel.x, vel.y + obj->gravity));
    }
}

void har_take_damage(object *obj, str* string, float damage) {
    har *h = object_get_userdata(obj);
    h->health -= damage;
    if(h->health <= 0) { h->health = 0; }

    // chronos' stasis does not have a hit animation
    if (string->data) {
        // Set hit animation
        object_set_animation(obj, &af_get_move(&h->af_data, ANIM_DAMAGE)->ani);
        object_set_repeat(obj, 0);
        object_set_custom_string(obj, str_c(string));
        object_tick(obj);
    }
}

void har_spawn_scrap(object *obj, vec2i pos) {
    float amount = 5;
    float rv = 0.0f;
    float velx, vely;
    har *h = object_get_userdata(obj);
    for(int i = 0; i < amount; i++) {
        // Calculate velocity etc.
        rv = (rand() % 100) / 100.0f - 0.5;
        velx = 5 * cos(90 + i-(amount) / 2 + rv);
        vely = -12 * sin(i / amount + rv);

        // Make sure scrap has somekind of velocity
        // (to prevent floating scrap objects)
        if(vely < 0.1 && vely > -0.1) vely += 0.21;

        // Create the object
        object *scrap = malloc(sizeof(object));
        int anim_no = rand() % 3 + ANIM_SCRAP_METAL;
        object_create(scrap, pos, vec2f_create(velx, vely));
        object_set_animation(scrap, &af_get_move(&h->af_data, anim_no)->ani);
        object_set_palette(scrap, object_get_palette(obj), 0);
        object_set_stl(scrap, object_get_stl(obj));
        object_set_repeat(scrap, 1);
        object_set_gravity(scrap, 1);
        object_set_layers(scrap, LAYER_SCRAP);
        object_tick(scrap);
        scrap_create(scrap);
        game_state_add_object(scrap);
    }
}

void har_block(object *obj, vec2i hit_coord) {
    har *h = obj->userdata;
    if (h->state == STATE_WALKING) {
        object_set_animation(obj, &af_get_move(&h->af_data, ANIM_STANDING_BLOCK)->ani);
    } else {
        object_set_animation(obj, &af_get_move(&h->af_data, ANIM_CROUCHING_BLOCK)->ani);
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
    object_create(scrape, hit_coord, vec2f_create(0, 0));
    object_set_animation(scrape, &af_get_move(&h->af_data, ANIM_BLOCKING_SCRAPE)->ani);
    object_set_palette(scrape, object_get_palette(obj), 0);
    object_set_stl(scrape, object_get_stl(obj));
    object_set_direction(scrape, object_get_direction(obj));
    object_set_repeat(scrape, 0);
    object_set_gravity(scrape, 0);
    object_set_layers(scrape, LAYER_SCRAP);
    object_tick(scrape);
    object_tick(scrape);
    game_state_add_object(scrape);
    h->damage_received = 1;
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
    if(a->state == STATE_WALKING && object_get_direction(obj_a) == OBJECT_FACE_LEFT) {
        if(pos_a.x < pos_b.x + hard_limit && pos_a.x > pos_b.x) {
            pos_b.x = pos_a.x - hard_limit;
            object_set_pos(obj_b, pos_b);
            a->hard_close = 1;
        }
        if(pos_a.x < pos_b.x + soft_limit && pos_a.x > pos_b.x) {
            a->close = 1;
            a->hard_close = 1;
        }
    }
    if(a->state == STATE_WALKING && object_get_direction(obj_a) == OBJECT_FACE_RIGHT) {
        if(pos_a.x + hard_limit > pos_b.x && pos_a.x < pos_b.x) {
            pos_b.x = pos_a.x + hard_limit;
            object_set_pos(obj_b, pos_b);
            a->hard_close = 1;
        }
        if(pos_a.x + soft_limit > pos_b.x && pos_a.x < pos_b.x) {
            a->close = 1;
            a->hard_close = 1;
        }
    }
}

void har_collide_with_har(object *obj_a, object *obj_b) {
    sd_stringparser_frame frame_b;
    har *a = object_get_userdata(obj_a);
    har *b = object_get_userdata(obj_b);

    // Check for collisions by sprite collision points
    int level = 2;
    af_move *move = af_get_move(&(a->af_data), obj_a->cur_animation->id);
    vec2i hit_coord = vec2i_create(0, 0);
    if(a->damage_done == 0 &&
#ifdef DEBUGMODE
            (intersect_sprite_hitpoint(obj_a, obj_b, level, &hit_coord, &a->debug_img)
#else
            (intersect_sprite_hitpoint(obj_a, obj_b, level, &hit_coord)
#endif
            || move->category == CAT_CLOSE)) 
    {
        if (b->blocking && (b->state == STATE_WALKING || b->state == STATE_CROUCHING)) {
            har_block(obj_b, hit_coord);
            return;
        }

        if (move->next_move) {
            object_set_animation(obj_a, &af_get_move(&a->af_data, move->next_move)->ani);
            object_set_repeat(obj_a, 0);
            return;
        }

        obj_b->animation_state.enemy_x = obj_a->pos.x;
        obj_b->animation_state.enemy_y = obj_a->pos.y;
        har_take_damage(obj_b, &move->footer_string, move->damage);
        if (hit_coord.x != 0 || hit_coord.y != 0) {
            har_spawn_scrap(obj_b, hit_coord);
        }
        a->damage_done = 1;
        b->damage_received = 1;

        // XXX   a hackish way to determine if a HAR is fallen or not
        sd_stringparser_peek(obj_b->animation_state.parser, sd_stringparser_num_frames(obj_b->animation_state.parser)-1, &frame_b);
        if(frame_b.duration > 100) {
            b->state = STATE_FALLEN;
        } else {
            b->state = STATE_RECOIL;
        }
        DEBUG("HAR %s to HAR %s collision at %d,%d!", 
            get_id_name(a->id), 
            get_id_name(b->id),
            hit_coord.x,
            hit_coord.y);
        DEBUG("HAR %s animation set to %s", get_id_name(b->id), str_c(&move->footer_string));
    } else if (a->damage_done) {
        DEBUG("move already hit");
    }
}

void har_collide_with_projectile(object *o_har, object *o_pjt) {
    sd_stringparser_frame frame_h;
    har *h = object_get_userdata(o_har);
    har *prog_owner = projectile_get_har(o_pjt);

    // Check for collisions by sprite collision points
    int level = 2;
    vec2i hit_coord;
    if(h->damage_done == 0 && 
#ifdef DEBUGMODE
            intersect_sprite_hitpoint(o_pjt, o_har, level, &hit_coord, &h->debug_img))
#else
            intersect_sprite_hitpoint(o_pjt, o_har, level, &hit_coord))
#endif
    {
        if (h->blocking) {
            har_block(o_har, hit_coord);
            return;
        }

        af_move *move = af_get_move(&(prog_owner->af_data), o_pjt->cur_animation->id);

        har_take_damage(o_har, &move->footer_string, move->damage);
        har_spawn_scrap(o_har, hit_coord);
        o_har->animation_state.enemy_x = o_pjt->pos.x;
        o_har->animation_state.enemy_y = o_pjt->pos.y;
        h->damage_received = 1;

        // XXX   a hackish way to determine if a HAR is fallen or not
        sd_stringparser_peek(o_har->animation_state.parser, sd_stringparser_num_frames(o_har->animation_state.parser)-1, &frame_h);
        if (move->footer_string.data) { // hack for chronos' stasis
            if(frame_h.duration > 100) {
                h->state = STATE_FALLEN;
            } else {
                h->state = STATE_RECOIL;
            }
        }
        vec2f vel = object_get_vel(o_har);
        vel.x = 0.0f;
        object_set_vel(o_har, vel);
        o_pjt->animation_state.finished = 1;

        if (move->successor_id) {
            object_set_animation(o_pjt, &af_get_move(&prog_owner->af_data, move->successor_id)->ani);
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

    // Check for closeness between HARs and handle it
    har_check_closeness(obj_a, obj_b);
    har_check_closeness(obj_b, obj_a);

    // Handle har collisions
    har_collide_with_har(obj_a, obj_b);
    har_collide_with_har(obj_b, obj_a);
}

void har_tick(object *obj) {
    har *h = object_get_userdata(obj);
    // Make sure HAR doesn't walk through walls
    // TODO: Roof!
    vec2i pos = object_get_pos(obj);
    if(pos.x <  15) pos.x = 15;
    if(pos.x > 305) pos.x = 305;
    object_set_pos(obj, pos);

    // Make the HAR get up
    if(h->state == STATE_FALLEN && obj->animation_state.parser->current_frame.is_final_frame) {
        h->state = STATE_STANDING_UP;
        har_set_ani(obj, ANIM_STANDUP, 0);
    }

    // Stop HAR from sliding if touching the ground
    if(h->state != STATE_JUMPING) {
        if(h->state != STATE_WALKING || h->executing_move) {
            vec2f vel = object_get_vel(obj);
            vel.x = 0;
            object_set_vel(obj, vel);
        }
    }
}

void add_input(har *h, char c) {
    // only add it if it is not the current head of the array
    if(h->inputs[0] == c) {
        return;
    }

    // use memmove to move everything over one spot in the array, leaving the first slot free
    memmove((h->inputs)+1, h->inputs, 9);
    // write the new first element
    h->inputs[0] = c;
}

void har_act(object *obj, int act_type) {
    har *h = object_get_userdata(obj);
    int direction = object_get_direction(obj);
    if(!(h->state == STATE_STANDING ||
         h->state == STATE_CROUCHING ||
         h->state == STATE_WALKING ||
         h->state == STATE_JUMPING ||
         h->state == STATE_VICTORY ||
         h->state == STATE_SCRAP)) {
        // doing something else, ignore input
        return;
    }

    // Don't allow movement if arena is starting or ending
    int arena_state = arena_get_state(game_state_get_scene());
    if(arena_state == ARENA_STATE_STARTING) {
        return;
    }

    // Don't allow new moves while we're still executing a previous one.
    if(h->executing_move) {
        return;
    }

   // for the reason behind the numbers, look at a numpad sometime
    switch(act_type) {
        case ACT_UP:
            add_input(h, '8');
            break;
        case ACT_DOWN:
            add_input(h, '2');
            break;
        case ACT_LEFT:
            if(direction == OBJECT_FACE_LEFT) {
                add_input(h, '6');
            } else {
                add_input(h, '4');
            }
            break;
        case ACT_RIGHT:
            if(direction == OBJECT_FACE_LEFT) {
                add_input(h, '4');
            } else {
                add_input(h, '6');
            }
            break;
        case ACT_UPRIGHT:
            if(direction == OBJECT_FACE_LEFT) {
                add_input(h, '7');
            } else {
                add_input(h, '9');
            }
            break;
        case ACT_UPLEFT:
            if(direction == OBJECT_FACE_LEFT) {
                add_input(h, '9');
            } else {
                add_input(h, '7');
            }
            break;
        case ACT_DOWNRIGHT:
            if(direction == OBJECT_FACE_LEFT) {
                add_input(h, '1');
            } else {
                add_input(h, '3');
            }
            break;
        case ACT_DOWNLEFT:
            if(direction == OBJECT_FACE_LEFT) {
                add_input(h, '3');
            } else {
                add_input(h, '1');
            }
            break;
        case ACT_KICK:
            add_input(h, 'K');
            break;
        case ACT_PUNCH:
            add_input(h, 'P');
            break;
        case ACT_STOP:
            add_input(h, '5');
            break;
    }

    af_move *move;
    size_t len;
    for(int i = 0; i < 70; i++) {
        if((move = af_get_move(&h->af_data, i))) {
            len = move->move_string.len;
            if(!strncmp(str_c(&move->move_string), h->inputs, len)) {
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
                if (move->category != CAT_SCRAP && h->state == STATE_VICTORY) {
                    continue;
                }

                if (move->category != CAT_DESTRUCTION && h->state == STATE_SCRAP) {
                    continue;
                }

                DEBUG("matched move %d with string %s", i, str_c(&move->move_string));
                DEBUG("input was %s", h->inputs);

#ifdef DEBUGMODE
        DEBUG("UNKNOWN %d %d %d %d | %d %d %d %d | %d %d %d %d | %d %d %d %d | %d %d %d %d | %d",
              move->unknown[0], move->unknown[1], move->unknown[2], move->unknown[3],
              move->unknown[4], move->unknown[5], move->unknown[6], move->unknown[7],
              move->unknown[8], move->unknown[9], move->unknown[10], move->unknown[11],
              move->unknown[12], move->unknown[13], move->unknown[14], move->unknown[15],
              move->unknown[16], move->unknown[17], move->unknown[18], move->unknown[19],
              move->unknown[20]);
#endif

                // Stop horizontal movement, when move is done
                // TODO: Make this work better
                vec2f spd = object_get_vel(obj);
                //spd.x = 0.0f;
                object_set_vel(obj, spd);

                // Set correct animation etc.
                // executing_move = 1 prevents new moves while old one is running.
                har_set_ani(obj, i, 0);
                h->inputs[0] = '\0';
                h->executing_move = 1;

                if (move->category == CAT_SCRAP) {
                    DEBUG("going to scrap state");
                    h->state = STATE_SCRAP;
                    h->executing_move = 0;
                }
                if (move->category == CAT_DESTRUCTION) {
                    DEBUG("going to destruction state");
                    h->state = STATE_DESTRUCTION;
                }

                // make the other har participate in the scrap/destruction
                if (move->category == CAT_SCRAP || move->category == CAT_DESTRUCTION) {
                    int opp_id = h->player_id ? 0 : 1;
                    af_move *move = af_get_move(&(h->af_data), obj->cur_animation->id);
                    object *opp = game_player_get_har(game_state_get_player(opp_id));
                    opp->animation_state.enemy_x = obj->pos.x;
                    opp->animation_state.enemy_y = obj->pos.y;
                    object_set_animation(opp, &af_get_move(&((har*)opp->userdata)->af_data, ANIM_DAMAGE)->ani);
                    object_set_repeat(opp, 0);
                    object_set_custom_string(opp, str_c(&move->footer_string));
                    object_tick(opp);
                }

                return;
            }
        }
    }

    if(obj->pos.y < 190) {
        // airborne
        return;
    }

    if(arena_state == ARENA_STATE_ENDING) {
        return;
    }

    // no moves matched, do player movement
    float vx, vy;
    h->blocking = 0;
    switch(act_type) {
        case ACT_DOWNRIGHT:
            if(act_type == ACT_DOWNRIGHT && direction == OBJECT_FACE_LEFT) {
                h->blocking = 1;
            }
        case ACT_DOWNLEFT:
            if(act_type == ACT_DOWNLEFT && direction == OBJECT_FACE_RIGHT) {
                h->blocking = 1;
            }
        case ACT_DOWN:
            if(h->state != STATE_CROUCHING) {
                har_set_ani(obj, ANIM_CROUCHING, 1);
                object_set_vel(obj, vec2f_create(0,0));
                h->state = STATE_CROUCHING;
            }
            break;
        case ACT_STOP:
            if(h->state != STATE_STANDING) {
                har_set_ani(obj, ANIM_IDLE, 1);
                object_set_vel(obj, vec2f_create(0,0));
                obj->slide_state.vel.x = 0;
                h->state = STATE_STANDING;
            }
            break;
        case ACT_LEFT:
            if(h->state != STATE_WALKING) {
                har_set_ani(obj, ANIM_WALKING, 1);
                h->state = STATE_WALKING;
            }
            if(direction == OBJECT_FACE_LEFT) {
                vx = (h->af_data.forward_speed*-1)/(float)320;
            } else {
                h->blocking = 1;
                vx = h->af_data.reverse_speed*-1/(float)320;
            }
            object_set_vel(obj, vec2f_create(vx*(h->hard_close ? 0.5 : 1.0),0));
            break;
        case ACT_RIGHT:
            if(h->state != STATE_WALKING) {
                har_set_ani(obj, ANIM_WALKING, 1);
                h->state = STATE_WALKING;
            }
            if(direction == OBJECT_FACE_LEFT) {
                h->blocking = 1;
                vx = h->af_data.reverse_speed/(float)320;
            } else {
                vx = h->af_data.forward_speed/(float)320;
            }
            object_set_vel(obj, vec2f_create(vx*(h->hard_close ? 0.5 : 1.0),0));
            break;
        case ACT_UP:
            if(h->state != STATE_JUMPING) {
                har_set_ani(obj, ANIM_JUMPING, 1);
                vy = (float)h->af_data.jump_speed;
                object_set_vel(obj, vec2f_create(0,vy));
                h->state = STATE_JUMPING;
            }
            break;
        case ACT_UPLEFT:
            if(h->state != STATE_JUMPING) {
                har_set_ani(obj, ANIM_JUMPING, 1);
                vy = (float)h->af_data.jump_speed;
                vx = h->af_data.reverse_speed*-1/(float)320;
                if(direction == OBJECT_FACE_LEFT) {
                    vx = (h->af_data.forward_speed*-1)/(float)320;
                }
                object_set_vel(obj, vec2f_create(vx,vy));
                h->state = STATE_JUMPING;
            }
            break;
        case ACT_UPRIGHT:
            if(h->state != STATE_JUMPING) {
                har_set_ani(obj, ANIM_JUMPING, 1);
                vy = (float)h->af_data.jump_speed;
                vx = h->af_data.forward_speed/(float)320;
                if(direction == OBJECT_FACE_LEFT) {
                    vx = h->af_data.reverse_speed/(float)320;
                }
                object_set_vel(obj, vec2f_create(vx,vy));
                h->state = STATE_JUMPING;
            }
            break;
    }
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
        game_state_set_next(SCENE_MENU);
    } else if(h->state != STATE_CROUCHING) {
        // Don't transition to standing state while in midair
        if(h->state != STATE_JUMPING) { h->state = STATE_STANDING; }
        har_set_ani(obj, ANIM_IDLE, 1);
    } else {
        har_set_ani(obj, ANIM_CROUCHING, 1);
    }
    h->executing_move = 0;
}

void har_fix_sprite_coords(animation *ani, int fix_x, int fix_y) {
    iterator it;
    sprite *s;
    // Fix sprite positions
    vector_iter_begin(&ani->sprites, &it);
    while((s = iter_next(&it)) != NULL) {
        s->pos.x += fix_x;
        s->pos.y += fix_y;
    }
    // Fix collisions coordinates
    collision_coord *c;
    vector_iter_begin(&ani->collision_coords, &it);
    while((c = iter_next(&it)) != NULL) {
        c->pos.x += fix_x;
        c->pos.y += fix_y;
    }
}

#ifdef DEBUGMODE
void har_debug(object *obj) {
    har *h = object_get_userdata(obj);
    if(h->debug_enabled == 0) return;
    texture_init_from_img(&h->debug_tex, &h->debug_img);
    video_render_sprite(&h->debug_tex, 0, 0, BLEND_ALPHA_FULL);
    texture_free(&h->debug_tex);
    image_clear(&h->debug_img, color_create(0,0,0,0));
}
#endif

int har_create(object *obj, palette *pal, int dir, int har_id, int player_id) {
    // Create local data
    har *local = malloc(sizeof(har));
    object_set_userdata(obj, local);

    // Load AF
    if(load_af_file(&local->af_data, har_id)) {
        PERROR("Unable to load HAR %s (%d)!", get_id_name(har_id), har_id);
        free(local);
        return 1;
    }

    // Fix some coordinates on jump sprites
    har_fix_sprite_coords(&af_get_move(&local->af_data, ANIM_JUMPING)->ani, 0, -50);

    // Save har id
    local->id = har_id;
    local->player_id = player_id;

    // Health, endurance
    local->health_max = local->health = 1000;
    local->endurance_max = local->endurance = 1000;
    local->close = 0;
    local->hard_close =  0;
    local->state = STATE_STANDING;
    local->executing_move = 0;

    // Object related stuff
    object_set_gravity(obj, local->af_data.fall_speed);
    object_set_layers(obj, LAYER_HAR | (player_id == 0 ? LAYER_HAR1 : LAYER_HAR2));
    object_set_direction(obj, dir);
    object_set_repeat(obj, 1);
    object_set_stl(obj, local->af_data.sound_translation_table);

    // New object spawner callback
    object_set_spawn_cb(obj, cb_har_spawn_object, local);

    // Set running animation 
    object_set_palette(obj, pal, 0);
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

#ifdef DEBUGMODE
    object_set_debug_cb(obj, har_debug);
    texture_create(&local->debug_tex);
    image_create(&local->debug_img, 320, 200);
    image_clear(&local->debug_img, color_create(0,0,0,0));
    local->debug_enabled = 0;
#endif

    // All done
    DEBUG("Har %s (%d) loaded!", get_id_name(har_id), har_id);
    return 0;
}
