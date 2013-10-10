#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "game/objects/har.h"
#include "game/protos/intersect.h"
#include "resources/af_loader.h"
#include "resources/ids.h"
#include "resources/animation.h"
#include "controller/controller.h"
#include "utils/log.h"

void har_free(object *obj) {
    har *h = object_get_userdata(obj);
    af_free(&h->af_data);
    free(h);
}

void har_move(object *obj) {
    //har *h = object_get_userdata(obj);
    vec2f vel = object_get_vel(obj);
    obj->pos.x += vel.x;
    obj->pos.y += (vel.y*0.003);
    if (obj->pos.y > 190) {
        obj->pos.y = 190;
        object_set_vel(obj, vec2f_create(vel.x, 0));
    } else {
        object_set_vel(obj, vec2f_create(vel.x, vel.y + obj->gravity));
    }
}

// Simple helper function
void har_set_ani(object *obj, int animation_id, int repeat) {
    har *har = object_get_userdata(obj);
    object_set_animation(obj, &af_get_move(&har->af_data, animation_id)->ani);
    object_set_repeat(obj, repeat);
    har->damage_done = 0;
}

void har_take_damage(object *obj) {
    har *har = object_get_userdata(obj);
    har->health -= 10;
    har_set_ani(obj, ANIM_DAMAGE, 0);
}

void har_spawn_scrap(object *obj) {

}

void har_check_closeness(object *obj_a, object *obj_b) {
    vec2i pos_a = object_get_pos(obj_a);
    vec2i pos_b = object_get_pos(obj_b);
    har *a = object_get_userdata(obj_a);
    int hard_limit = 35; // Stops movement if HARs too close. Harrison-Stetson method value.
    int soft_limit = 45; // Sets HAR A as being close to HAR B if closer than this.
    int anim = object_get_animation(obj_a)->id;
    int jumping = (object_get_vstate(obj_a) == OBJECT_MOVING);

    // Reset closeness state
    a->close = 0;

    // If HARs get too close together, handle it
    if(anim == ANIM_WALKING && object_get_direction(obj_a) == OBJECT_FACE_LEFT) {
        if(pos_a.x < pos_b.x + hard_limit && pos_a.x > pos_b.x && !jumping) {
            pos_a.x = pos_b.x + hard_limit;
            object_set_pos(obj_a, pos_a);
        }
        if(pos_a.x < pos_b.x + soft_limit && pos_a.x > pos_b.x) {
            a->close = 1;
        }
    }
    if(anim == ANIM_WALKING && object_get_direction(obj_a) == OBJECT_FACE_RIGHT) {
        if(pos_a.x + hard_limit > pos_b.x && pos_a.x < pos_b.x && !jumping) {
            pos_a.x = pos_b.x - hard_limit;
            object_set_pos(obj_a, pos_a);
        }
        if(pos_a.x + soft_limit > pos_b.x && pos_a.x < pos_b.x) {
            a->close = 1;
        }
    }
}

void har_collide(object *obj_a, object *obj_b) {
    har *a = object_get_userdata(obj_a);
    har *b = object_get_userdata(obj_b);

    // Check for closeness between HARs and handle it
    har_check_closeness(obj_a, obj_b);
    har_check_closeness(obj_b, obj_a);

    // Check for collisions by sprite collision points
    if(a->damage_done == 0 && intersect_sprite_hitpoint(obj_a, obj_b)) {
        har_take_damage(obj_b);
        a->damage_done = 1;
        DEBUG("HAR %s to HAR %s collision!", get_id_name(a->id), get_id_name(b->id));
    }
    if(b->damage_done == 0 && intersect_sprite_hitpoint(obj_b, obj_a)) {
        har_take_damage(obj_a);
        b->damage_done = 1;
        DEBUG("HAR %s to HAR %s collision!", get_id_name(b->id), get_id_name(a->id));
    }
}

void har_tick(object *obj) {
    vec2i pos = object_get_pos(obj);
    if(pos.x <  15) pos.x = 15;
    if(pos.x > 305) pos.x = 305;
    object_set_pos(obj, pos);
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

void har_act(object *obj, int act_type) {
    har *har = object_get_userdata(obj);
    int anim = object_get_animation(obj)->id;
    int direction = object_get_direction(obj);
    if (!(anim == ANIM_IDLE ||
          anim == ANIM_CROUCHING ||
          anim == ANIM_WALKING ||
          anim == ANIM_JUMPING)) {
        // doing something else, ignore input
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
            if (direction == OBJECT_FACE_LEFT) {
                add_input(har, '6');
            } else {
                add_input(har, '4');
            }
            break;
        case ACT_RIGHT:
            if (direction == OBJECT_FACE_LEFT) {
                add_input(har, '4');
            } else {
                add_input(har, '6');
            }
            break;
        case ACT_UPRIGHT:
            if (direction == OBJECT_FACE_LEFT) {
                add_input(har, '7');
            } else {
                add_input(har, '9');
            }
            break;
        case ACT_UPLEFT:
            if (direction == OBJECT_FACE_LEFT) {
                add_input(har, '9');
            } else {
                add_input(har, '7');
            }
            break;
        case ACT_DOWNRIGHT:
            if (direction == OBJECT_FACE_LEFT) {
                add_input(har, '1');
            } else {
                add_input(har, '3');
            }
            break;
        case ACT_DOWNLEFT:
            if (direction == OBJECT_FACE_LEFT) {
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
            add_input(har, '5');
            break;
    }

    af_move *move;
    size_t len;
    for(int i = 0; i < 70; i++) {
        if((move = af_get_move(&har->af_data, i))) {
            len = move->move_string.len;
            if(!strncmp(str_c(&move->move_string), har->inputs, len)) {
                DEBUG("matched move %d with string %s", i, str_c(&move->move_string));
                DEBUG("input was %s", har->inputs);
                har_set_ani(obj, i, 0);
                har->inputs[0]='\0';
                return;
            }
        }
    }

    if (obj->pos.y < 190) {
        // airborne
        return;
    }

    // no moves matched, do player movement
    float vx, vy;
    switch (act_type) {
        case ACT_DOWN:
        case ACT_DOWNRIGHT:
        case ACT_DOWNLEFT:
            har_set_ani(obj, ANIM_CROUCHING, 1);
            object_set_vel(obj, vec2f_create(0,0));
            break;
        case ACT_STOP:
            if (anim != ANIM_IDLE) {
                har_set_ani(obj, ANIM_IDLE, 1);
                object_set_vel(obj, vec2f_create(0,0));
                obj->slide_state.vel.x = 0;
            }
            break;
        case ACT_LEFT:
            if (anim != ANIM_WALKING) {
                har_set_ani(obj, ANIM_WALKING, 1);
            }
            vx = har->af_data.reverse_speed*-1/(float)320;
            if (direction == OBJECT_FACE_LEFT) {
                vx = (har->af_data.forward_speed*-1)/(float)320;
            }
            object_set_vel(obj, vec2f_create(vx,0));
            break;
        case ACT_RIGHT:
            if (anim != ANIM_WALKING) {
                har_set_ani(obj, ANIM_WALKING, 1);
            }
            vx = har->af_data.forward_speed/(float)320;
            if (direction == OBJECT_FACE_LEFT) {
                vx = har->af_data.reverse_speed/(float)320;
            }
            object_set_vel(obj, vec2f_create(vx,0));
            break;
        case ACT_UP:
            vy = (float)har->af_data.jump_speed;
            object_set_gravity(obj, har->af_data.fall_speed);
            object_set_vel(obj, vec2f_create(0,vy));
            break;
        case ACT_UPLEFT:
            vy = (float)har->af_data.jump_speed;
            object_set_gravity(obj, har->af_data.fall_speed);
            vx = har->af_data.reverse_speed*-1/(float)320;
            if (direction == OBJECT_FACE_LEFT) {
                vx = (har->af_data.forward_speed*-1)/(float)320;
            }
            object_set_vel(obj, vec2f_create(vx,vy));
            break;
        case ACT_UPRIGHT:
            vy = (float)har->af_data.jump_speed;
            object_set_gravity(obj, har->af_data.fall_speed);
            vx = har->af_data.reverse_speed*-1/(float)320;
            vx = har->af_data.forward_speed/(float)320;
            if (direction == OBJECT_FACE_LEFT) {
                vx = har->af_data.reverse_speed/(float)320;
            }
            object_set_vel(obj, vec2f_create(vx,vy));
            break;
    }
}

void har_finished(object *obj) {
    har_set_ani(obj, ANIM_IDLE, 1);
}

int har_create(object *obj, palette *pal, int dir, int har_id) {
    // Create local data
    har *local = malloc(sizeof(har));
    object_set_userdata(obj, local);

    // Load AF
    if(load_af_file(&local->af_data, har_id)) {
        PERROR("Unable to load HAR %s (%d)!", get_id_name(har_id), har_id);
        free(local);
        return 1;
    }

    // Save har id
    local->id = har_id;

    // Health, endurance
    local->health_max = local->health = 1000;
    local->endurance_max = local->endurance = 1000;
    local->close = 0;

    // Object related stuff
    object_set_gravity(obj, 1.0f);
    object_set_layers(obj, LAYER_HAR);
    object_set_direction(obj, dir);
    object_set_repeat(obj, 1);
    object_set_stl(obj, local->af_data.sound_translation_table);

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

    // All done
    DEBUG("Har %s (%d) loaded!", get_id_name(har_id), har_id);
    return 0;
}
