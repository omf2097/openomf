#ifndef _OBJECT_H
#define _OBJECT_H

#include "resources/animation.h"
#include "resources/sprite.h"
#include "game/protos/player.h"
#include "utils/vec.h"
#include "utils/hashmap.h"
#include "video/texture.h"

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

enum {
    OWNER_EXTERNAL,
    OWNER_OBJECT
};

typedef struct object_t object;

typedef void (*object_free_cb)(object *obj);
typedef void (*object_act_cb)(object *obj, int action);
typedef void (*object_tick_cb)(object *obj);
typedef void (*object_move_cb)(object *obj);
typedef void (*object_collide_cb)(object *a, object *b);
typedef void (*object_finish_cb)(object *obj);

struct object_t {
    vec2f pos;
    vec2f vel;
    int vstate;
    int hstate;
    int direction;
    float y_percent;
    float gravity;
    
    int is_static;
    int group;
    int layers;
    
    int cur_animation_own;
    animation *cur_animation;
    sprite *cur_sprite;
    char *sound_translation_table;

    int texture_refresh;
    palette *cur_palette;
    int cur_remap;
    int halt;
    texture *cur_texture;

    player_sprite_state sprite_state;
    player_animation_state animation_state;
    player_slide_state slide_state;

    void *userdata;
    object_free_cb free;
    object_act_cb act;
    object_tick_cb tick;
    object_collide_cb collide;
    object_finish_cb finish;
    object_move_cb move;
};

void object_create(object *obj, vec2i pos, vec2f vel);
void object_render(object *obj);
void object_render_neutral(object *obj);
void object_tick(object *obj);
void object_move(object *obj);
void object_render(object *obj);
void object_collide(object *a, object *b);
void object_act(object *obj, int action);
int object_finished(object *obj);
void object_free(object *obj);

void object_set_stl(object *obj, char *ptr);
char* object_get_stl(object *obj);
void object_set_animation_owner(object *obj, int owner);
void object_set_animation(object *obj, animation *ani);
animation *object_get_animation(object *obj);
void object_set_custom_string(object *obj, const char *str);
void object_select_sprite(object *obj, int id);
void object_set_palette(object *obj, palette *pal, int remap);
palette* object_get_palette(object *obj);

void object_revalidate(object *obj);

void object_set_halt(object *obj, int halt);
int object_get_halt(object *obj);

void object_set_layers(object *obj, int layers);
void object_set_group(object *obj, int group);
void object_set_gravity(object *obj, float gravity);
void object_set_static(object *obj, int is_static);

void object_set_userdata(object *obj, void *ptr);
void *object_get_userdata(object *obj);
void object_set_free_cb(object *obj, object_free_cb cbfunc);
void object_set_act_cb(object *obj, object_act_cb cbfunc);
void object_set_tick_cb(object *obj, object_tick_cb cbfunc);
void object_set_collide_cb(object *obj, object_collide_cb cbfunc);
void object_set_finish_cb(object *obj, object_finish_cb cbfunc);
void object_set_move_cb(object *obj, object_move_cb cbfunc);

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
int object_get_vstate(object *obj);
int object_get_hstate(object *obj);

vec2i object_get_size(object *obj);
vec2i object_get_pos(object *obj);
vec2f object_get_vel(object *obj);

void object_set_pos(object *obj, vec2i pos);
void object_set_vel(object *obj, vec2f vel);

void object_set_spawn_cb(object *obj, object_state_add_cb cbf, void *userdata);
void object_set_destroy_cb(object *obj, object_state_del_cb cbf, void *userdata);

#endif // _OBJECT_H
