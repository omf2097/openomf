#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "game/objects/har.h"
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

void har_tick(object *obj) {
    //har *h = object_get_userdata(obj);
    vec2f vel = object_get_vel(obj);
    obj->pos.x += vel.x;
    obj->pos.y += vel.y;
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
            if (!strncmp(str_c(&move->move_string), har->inputs, len)) {
                DEBUG("matched move %d with string %s", i, str_c(&move->move_string));
                DEBUG("input was %s", har->inputs);
                object_set_animation(obj, &af_get_move(&har->af_data, i)->ani);
                object_set_repeat(obj, 0);
                har->inputs[0]='\0';
                return;
            }
        }
    }

    // no moves matched, do player movement
    float vx;
    switch (act_type) {
        case ACT_DOWN:
        case ACT_DOWNRIGHT:
        case ACT_DOWNLEFT:
            object_set_animation(obj, &af_get_move(&har->af_data, ANIM_CROUCHING)->ani);
            object_set_repeat(obj, 1);
            object_set_vel(obj, vec2f_create(0,0));
            break;
        case ACT_STOP:
            if (anim != ANIM_IDLE) {
                object_set_animation(obj, &af_get_move(&har->af_data, ANIM_IDLE)->ani);
                object_set_repeat(obj, 1);
                object_set_vel(obj, vec2f_create(0,0));
                obj->slide_state.vel.x = 0;
            }
            break;
        case ACT_LEFT:
            if (anim != ANIM_WALKING) {
                object_set_animation(obj, &af_get_move(&har->af_data, ANIM_WALKING)->ani);
                object_set_repeat(obj, 1);
            }
            vx = har->af_data.reverse_speed*-1/(float)320;
            if (direction == OBJECT_FACE_LEFT) {
                vx = (har->af_data.forward_speed*-1)/(float)320;
            }
            object_set_vel(obj, vec2f_create(vx,0));
            break;
        case ACT_RIGHT:
            if (anim != ANIM_WALKING) {
                object_set_animation(obj, &af_get_move(&har->af_data, ANIM_WALKING)->ani);
                object_set_repeat(obj, 1);
            }
            vx = har->af_data.forward_speed/(float)320;
            if (direction == OBJECT_FACE_LEFT) {
                vx = har->af_data.reverse_speed/(float)320;
            }
            object_set_vel(obj, vec2f_create(vx,0));
            break;
    }
}

void har_finished(object *obj) {
    har *har = object_get_userdata(obj);
    object_set_animation(obj, &af_get_move(&har->af_data, ANIM_IDLE)->ani);
    object_set_repeat(obj, 1);
}

int har_create(object *obj, palette *pal, int dir, int har_id) {
    // Create local data
    har *local = malloc(sizeof(har));

    // Load AF
    if(load_af_file(&local->af_data, har_id)) {
        PERROR("Unable to load HAR %s (%d)!", get_id_name(har_id), har_id);
        free(local);
        return 1;
    }

    // Health, endurance
    local->health_max = local->health = 1000;
    local->endurance_max = local->endurance = 1000;

    // Object related stuff
    object_set_gravity(obj, 1.0f);
    object_set_layers(obj, LAYER_HAR);
    object_set_direction(obj, dir);
    object_set_repeat(obj, 1);
    object_set_stl(obj, local->af_data.sound_translation_table);

    // Set running animation 
    object_set_animation(obj, &af_get_move(&local->af_data, ANIM_IDLE)->ani);
    object_set_palette(obj, pal, 0);

    // fill the input buffer with 'pauses'
    memset(local->inputs, '5', 10);
    local->inputs[10] = '\0';

    // Callbacks and userdata
    object_set_userdata(obj, local);
    object_set_free_cb(obj, har_free);
    object_set_act_cb(obj, har_act);
    object_set_tick_cb(obj, har_tick);
    object_set_finish_cb(obj, har_finished);

    // All done
    DEBUG("Har %s (%d) loaded!", get_id_name(har_id), har_id);
    return 0;
}
