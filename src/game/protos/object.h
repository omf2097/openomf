#ifndef OBJECT_H
#define OBJECT_H

#include "game/protos/player.h"
#include "game/utils/serial.h"
#include "resources/animation.h"
#include "resources/sprite.h"
#include "utils/hashmap.h"
#include "utils/random.h"
#include "utils/vec.h"
#include "video/screen_palette.h"
#include "video/surface.h"

#define OBJECT_DEFAULT_LAYER 0x01
#define OBJECT_NO_GROUP -1

#define OBJECT_EVENT_BUFFER_SIZE 16

enum
{
    OBJECT_FACE_LEFT = -1,
    OBJECT_FACE_RIGHT = 1
};

enum
{
    OWNER_EXTERNAL,
    OWNER_OBJECT
};

enum
{
    PLAY_BACKWARDS,
    PLAY_FORWARDS
};

enum
{
    EFFECT_NONE = 0,
    EFFECT_SHADOW = 0x1,
    EFFECT_DARK_TINT = 0x2,
    EFFECT_POSITIONAL_LIGHTING = 0x4,
    EFFECT_STASIS = 0x8,
    EFFECT_SATURATE = 0x10,
};

typedef struct object_t object;
typedef struct game_state_t game_state;

typedef void (*object_free_cb)(object *obj);
typedef int (*object_act_cb)(object *obj, int action);
typedef void (*object_tick_cb)(object *obj);
typedef void (*object_move_cb)(object *obj);
typedef void (*object_collide_cb)(object *a, object *b);
typedef void (*object_finish_cb)(object *obj);
typedef void (*object_debug_cb)(object *obj);
typedef int (*object_palette_transform_cb)(object *obj, screen_palette *pal);
typedef int (*object_clone_cb)(object *src, object *dst);
typedef int (*object_clone_free_cb)(object *obj);

struct object_t {
    uint32_t id;
    game_state *gs;

    vec2f start;
    vec2f pos;
    vec2f vel;
    float vertical_velocity_modifier;
    float horizontal_velocity_modifier;
    int8_t direction;
    int8_t group;

    // Set by q tag
    int8_t hit_frames;
    int8_t can_hit;

    int8_t orbit;
    float orbit_tick;
    vec2f orbit_dest;
    vec2f orbit_dest_dir;
    vec2f orbit_pos;
    vec2f orbit_pos_vary;

    struct random_t rand_state;

    float x_percent;
    float y_percent;
    float gravity;

    // Bitmask for several video effects (shadow, etc.)
    int video_effects;

    uint8_t layers;
    uint8_t cur_animation_own;

    animation *cur_animation;
    int cur_sprite_id;
    const char *sound_translation_table;
    uint8_t sprite_override; //< Tells whether cur_sprite should be kept constant regardless of anim string.

    // 0 if this object is not attached to any other objects
    // non zero object id if it is. In this case, velocity and direction will be matched.
    uint32_t attached_to_id;

    uint8_t pal_offset;
    uint8_t pal_limit;
    uint8_t cur_remap;
    uint8_t halt;
    int16_t halt_ticks;
    uint8_t stride;
    uint8_t cast_shadow;
    // pointer into obj->cur_sprite->data
    surface *cur_surface;

    player_sprite_state sprite_state;
    player_animation_state animation_state;
    player_slide_state slide_state;
    player_enemy_slide_state enemy_slide_state;

    // state ringbuffer
    uint32_t age;

    char *custom_str;

    void *userdata;
    object_free_cb free;
    object_act_cb act;
    object_tick_cb static_tick;
    object_tick_cb dynamic_tick;
    object_collide_cb collide;
    object_finish_cb finish;
    object_move_cb move;
    object_debug_cb debug;
    object_palette_transform_cb pal_transform;
    object_clone_cb clone;
    object_clone_free_cb clone_free;
};

void object_create(object *obj, game_state *gs, vec2i pos, vec2f vel);
void object_create_static(object *obj, game_state *gs);
void object_render(object *obj);
void object_render_shadow(object *obj);
void object_debug(object *obj);
void object_static_tick(object *obj);
void object_dynamic_tick(object *obj);
void object_set_tick_pos(object *obj, int tick);
void object_move(object *obj);
int object_palette_transform(object *obj, screen_palette *pal);
void object_collide(object *a, object *b);
int object_act(object *obj, int action);
int object_finished(object *obj);
void object_free(object *obj);

int object_clone(object *src, object *dst, game_state *gs);
int object_clone_free(object *obj);

void object_attach_to(object *obj, const object *attach_to);

void object_set_stride(object *obj, int stride);
void object_set_delay(object *obj, int delay);
void object_set_playback_direction(object *obj, int dir);

void object_set_stl(object *obj, const char *ptr);
const char *object_get_stl(const object *obj);

void object_set_animation_owner(object *obj, int owner);
void object_set_animation(object *obj, animation *ani);
animation *object_get_animation(object *obj);
void object_set_custom_string(object *obj, const char *str);
void object_select_sprite(object *obj, int id);
void object_set_sprite_override(object *obj, int override);

void object_set_halt_ticks(object *obj, int ticks);
int object_get_halt_ticks(object *obj);
void object_set_halt(object *obj, int halt);
int object_get_halt(const object *obj);

void object_set_effects(object *obj, int effects);
int object_get_effects(const object *obj);
void object_add_effects(object *obj, int effects);
void object_del_effects(object *obj, int effects);

void object_set_layers(object *obj, int layers);
void object_set_group(object *obj, int group);
void object_set_gravity(object *obj, float gravity);

void object_set_userdata(object *obj, void *ptr);
void *object_get_userdata(const object *obj);
void object_set_free_cb(object *obj, object_free_cb cbfunc);
void object_set_act_cb(object *obj, object_act_cb cbfunc);
void object_set_dynamic_tick_cb(object *obj, object_tick_cb cbfunc);
void object_set_static_tick_cb(object *obj, object_tick_cb cbfunc);
void object_set_collide_cb(object *obj, object_collide_cb cbfunc);
void object_set_finish_cb(object *obj, object_finish_cb cbfunc);
void object_set_move_cb(object *obj, object_move_cb cbfunc);
void object_set_debug_cb(object *obj, object_debug_cb cbfunc);
void object_set_pal_transform_cb(object *obj, object_palette_transform_cb cbfunc);

void object_set_repeat(object *obj, int repeat);
int object_get_repeat(const object *obj);
void object_set_direction(object *obj, int dir);
int object_get_direction(const object *obj);

float object_get_gravity(const object *obj);
int object_get_group(const object *obj);
int object_get_layers(const object *obj);

void object_set_singleton(object *obj, int singleton);
int object_get_singleton(const object *obj);

int object_is_airborne(const object *obj);

void object_set_shadow(object *obj, int enable);
int object_get_shadow(const object *obj);

void object_disable_rewind_tag(object *obj, int disable_d);
int object_is_rewind_tag_disabled(const object *obj);

void object_set_pal_offset(object *obj, int offset);
int object_get_pal_offset(const object *obj);

void object_set_pal_limit(object *obj, int limit);
int object_get_pal_limit(const object *obj);

vec2i object_get_size(const object *obj);
vec2i object_get_pos(const object *obj);
vec2f object_get_vel(const object *obj);

void object_set_pos(object *obj, vec2i pos);
void object_set_vel(object *obj, vec2f vel);

int object_w(const object *obj);
int object_h(const object *obj);
int object_px(const object *obj);
int object_py(const object *obj);
float object_vx(const object *obj);
float object_vy(const object *obj);

void object_set_px(object *obj, int val);
void object_set_py(object *obj, int val);
void object_set_vx(object *obj, float val);
void object_set_vy(object *obj, float val);

uint32_t object_get_age(object *obj);
serial *object_get_last_serialization_point(const object *obj);
serial *object_get_serialization_point(const object *obj, unsigned int ticks_ago);

void object_set_spawn_cb(object *obj, object_state_add_cb cbf, void *userdata);
void object_set_destroy_cb(object *obj, object_state_del_cb cbf, void *userdata);

#endif // OBJECT_H
