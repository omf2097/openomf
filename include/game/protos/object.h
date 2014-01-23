#ifndef _OBJECT_H
#define _OBJECT_H

#include "resources/animation.h"
#include "resources/sprite.h"
#include "video/screen_palette.h"
#include "game/protos/player.h"
#include "utils/vec.h"
#include "utils/hashmap.h"
#include "video/surface.h"
#include "game/serial.h"

#define OBJECT_DEFAULT_LAYER 0x01
#define OBJECT_NO_GROUP -1

#define OBJECT_EVENT_BUFFER_SIZE 16

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

enum {
    PLAY_BACKWARDS,
    PLAY_FORWARDS
};

typedef struct object_t object;
typedef struct game_state_t game_state;

typedef void (*object_free_cb)(object *obj);
typedef int  (*object_act_cb)(object *obj, int action);
typedef void (*object_tick_cb)(object *obj);
typedef void (*object_move_cb)(object *obj);
typedef void (*object_collide_cb)(object *a, object *b);
typedef void (*object_finish_cb)(object *obj);
typedef int  (*object_serialize_cb)(object *obj, serial *ser);
typedef int  (*object_unserialize_cb)(object *obj, serial *ser, int animation_id, game_state *gs);
typedef void (*object_debug_cb)(object *obj);

struct object_t {
    game_state *gs;

    vec2f start;
    vec2f pos;
    vec2f vel;
    uint8_t vstate;
    uint8_t hstate;
    int8_t direction;
    int8_t group;
    int8_t orbit;
    float orbit_tick;
    vec2f orbit_dest;
    vec2f orbit_dest_dir;
    vec2f orbit_pos;
    vec2f orbit_pos_vary;

    float y_percent;
    float gravity;
    
    uint8_t layers;
    
    uint8_t cur_animation_own;

    animation *cur_animation;
    sprite *cur_sprite;
    char *sound_translation_table;

    uint8_t pal_offset;
    uint8_t cur_remap;
    uint8_t halt;
    uint8_t stride;
    uint8_t cast_shadow;
    surface *cur_surface;

    player_sprite_state sprite_state;
    player_animation_state animation_state;
    player_slide_state slide_state;

    // state ringbuffer
    uint32_t age;
    // should cover the last 400ms (25 * 16 = 400)
    // if the user's ping is > 800ms they're pretty screwed
    serial event_buffer[OBJECT_EVENT_BUFFER_SIZE];

    char *custom_str;
    uint8_t singleton;

    void *userdata;
    object_free_cb free;
    object_act_cb act;
    object_tick_cb tick;
    object_collide_cb collide;
    object_finish_cb finish;
    object_move_cb move;
    object_serialize_cb serialize;
    object_unserialize_cb unserialize;
    object_debug_cb debug;
};

void object_create(object *obj, game_state *gs, vec2i pos, vec2f vel);
void object_render(object *obj);
void object_render_shadow(object *obj);
void object_debug(object *obj);
void object_tick(object *obj);
void object_set_tick_pos(object *obj, int tick);
void object_move(object *obj);
int object_palette_transform(object *obj, screen_palette *pal);
void object_render(object *obj);
void object_collide(object *a, object *b);
int object_act(object *obj, int action);
int object_finished(object *obj);
void object_free(object *obj);

int object_serialize(object *obj, serial *ser);
int object_unserialize(object *obj, serial *ser, game_state *gs);

void object_set_stride(object *obj, int stride);
void object_set_delay(object *obj, int delay);
void object_set_playback_direction(object *obj, int dir);

void object_set_stl(object *obj, char *ptr);
char* object_get_stl(object *obj);
void object_set_animation_owner(object *obj, int owner);
void object_set_animation(object *obj, animation *ani);
animation *object_get_animation(object *obj);
void object_set_custom_string(object *obj, const char *str);
void object_select_sprite(object *obj, int id);

void object_set_halt(object *obj, int halt);
int object_get_halt(object *obj);

void object_set_layers(object *obj, int layers);
void object_set_group(object *obj, int group);
void object_set_gravity(object *obj, float gravity);

void object_set_userdata(object *obj, void *ptr);
void *object_get_userdata(object *obj);
void object_set_free_cb(object *obj, object_free_cb cbfunc);
void object_set_act_cb(object *obj, object_act_cb cbfunc);
void object_set_tick_cb(object *obj, object_tick_cb cbfunc);
void object_set_collide_cb(object *obj, object_collide_cb cbfunc);
void object_set_finish_cb(object *obj, object_finish_cb cbfunc);
void object_set_move_cb(object *obj, object_move_cb cbfunc);
void object_set_debug_cb(object *obj, object_debug_cb cbfunc);
void object_set_serialize_cb(object *obj, object_serialize_cb cbfunc);
void object_set_unserialize_cb(object *obj, object_unserialize_cb cbfunc);

void object_set_repeat(object *obj, int repeat);
int object_get_repeat(object *obj);
void object_set_direction(object *obj, int dir);
int object_get_direction(object *obj);

int object_get_gravity(object *obj);
int object_get_group(object *obj);
int object_get_layers(object *obj);

void object_set_shadow(object *obj, int enable);
int object_get_shadow(object *obj);

void object_set_gate_value(object *obj, int gate_value);
int object_get_gate_value(object *obj);

void object_set_pal_offset(object *obj, int offset);
int object_get_pal_offset(object *obj);

void object_reset_hstate(object *obj);
void object_reset_vstate(object *obj);
int object_get_vstate(object *obj);
int object_get_hstate(object *obj);

vec2i object_get_size(object *obj);
vec2i object_get_pos(object *obj);
vec2f object_get_vel(object *obj);

void object_set_pos(object *obj, vec2i pos);
void object_set_vel(object *obj, vec2f vel);

uint32_t object_get_age(object *obj);
serial* object_get_last_serialization_point(object *obj);
serial* object_get_serialization_point(object *obj, unsigned int ticks_ago);

void object_set_spawn_cb(object *obj, object_state_add_cb cbf, void *userdata);
void object_set_destroy_cb(object *obj, object_state_del_cb cbf, void *userdata);

#endif // _OBJECT_H
