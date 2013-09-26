#ifndef _OBJECT_H
#define _OBJECT_H

#include "resources/animation.h"
#include "resources/sprite.h"
#include "game/protos/player.h"
#include "utils/vec.h"

#define OBJECT_DEFAULT_LAYER 0x01
#define OBJECT_NO_GROUP -1

enum {
    OBJECT_STABLE = 0,
    OBJECT_MOVING
};

enum {
    OBJECT_FACE_LEFT = -1,
    OBJECT_FACE_RIGHT = 1
};

typedef struct object_t object;

typedef void (*object_free_cb)(object *object);
typedef void (*object_act_cb)(object *object, int action);
typedef void (*object_tick_cb)(object *object);

struct object_t {
    vec2f pos;
    vec2f vel;
    int vstate;
    int hstate;
    int direction;
    float gravity;
    
    int is_static;
    int group;
    int layers;
    
    animation *cur_animation;
    sprite *cur_sprite;
    char *sound_translation_table;

    player_sprite_state sprite_state;
    player_animation_state animation_state;

    void *userdata;
    object_free_cb free;
    object_act_cb act;
    object_tick_cb tick;
};

void object_create(object *obj, vec2i pos, vec2f vel);
void object_tick(object *obj);
void object_act(object *obj, int action);
void object_free(object *obj);

void object_set_animation(object *obj, animation *ani);
void object_select_sprite(object *obj, int id);

void object_set_layers(object *obj, int layers);
void object_set_group(object *obj, int group);
void object_set_gravity(object *obj, float gravity);
void object_set_static(object *obj, int is_static);

void object_set_userdata(object *obj, void *ptr);
void *object_get_userdata(object *obj);
void object_set_free_cb(object *obj, object_free_cb cbfunc);
void object_set_act_cb(object *obj, object_act_cb cbfunc);
void object_set_tick_cb(object *obj, object_tick_cb cbfunc);

void object_set_repeat(object *obj, int repeat);
int object_get_repeat(object *obj);

void object_set_direction(object *obj, int dir);
int object_get_direction(object *obj);

int object_is_static(object *obj);
int object_get_gravity(object *obj);
int object_get_group(object *obj);
int object_get_layers(object *obj);

void object_reset_hstate(object *obj);
void object_reset_vstate(object *obj);

int object_get_w(object *obj);
int object_get_h(object *obj);
void object_get_size(object *obj, int *w, int *h);

int  object_get_px(object *obj);
int  object_get_py(object *obj);
void object_set_px(object *obj, int px);
void object_set_py(object *obj, int py);

float object_get_vx(object *obj);
float object_get_vy(object *obj);
void  object_set_vx(object *obj, float vx);
void  object_set_vy(object *obj, float vy);

void object_get_pos(object *obj, int *px, int *py);
void object_set_pos(object *obj, int px, int py);
void object_add_pos(object *obj, int px, int py);

void object_get_vel(object *obj, float *vx, float *vy);
void object_set_vel(object *obj, float vx, float vy);
void object_add_vel(object *obj, float vx, float vy);

#endif // _OBJECT_H
