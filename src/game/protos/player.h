#ifndef PLAYER_H
#define PLAYER_H

#include "formats/script.h"
#include "game/game_state.h"
#include "utils/vec.h"
#include <stdint.h>

typedef struct object_t object;

typedef void (*object_state_add_cb)(object *parent, int id, vec2i pos, vec2f vel, uint8_t mp_flags, int s, int g,
                                    void *userdata);
typedef void (*object_state_del_cb)(object *parent, int id, void *userdata);
typedef void (*object_state_disable_cb)(object *parent, uint8_t id, uint16_t duration, void *userdata);

typedef struct player_sprite_state_t {
    int flipmode;
    int timer;
    int duration;
    int screen_shake_horizontal;
    int screen_shake_vertical;

    vec2i o_correction; // Sprite specific position correction
    int disable_gravity;

    int blend_start;
    int blend_finish;

    int pal_ref_index;   // bpd
    int pal_entry_count; // bpn
    int pal_start_index; // bps
    int pal_begin;       // bpb
    int pal_end;         // bpd
    int pal_tint;        // bz

    bool pal_tricks_off; // bpo
    bool bd_flag;        // bd
} player_sprite_state;

typedef struct player_slide_op_t {
    vec2f vel;
    int timer;
} player_slide_state;

typedef struct player_animation_state_t {
    uint32_t previous_tick;
    uint32_t current_tick;
    int previous;
    int entered_frame;
    sd_script parser;
    uint8_t repeat;
    uint8_t reverse;
    uint8_t finished;
    uint8_t disable_d;
    uint8_t shadow_corner_hack;
    bool looping;

    uint8_t pal_copy_entries; // ba
    uint8_t pal_copy_start;   // bi
    uint8_t pal_copy_count;   // bc

    void *spawn_userdata;
    void *destroy_userdata;
    void *disable_userdata;
    uint32_t enemy_obj_id;
    object_state_add_cb spawn;
    object_state_del_cb destroy;
    object_state_disable_cb disable;
} player_animation_state;

void player_create(object *obj);
void player_clone(object *src, object *dst);
void player_free(object *obj);
void player_reload(object *obj);
void player_reload_with_str(object *obj, const char *str);
void player_reset(object *obj);
int player_frame_isset(const object *obj, const char *tag);
int player_frame_get(const object *obj, const char *tag);
void player_run(object *obj);
void player_set_repeat(object *obj, int repeat);
int player_get_repeat(const object *obj);
void player_next_frame(object *obj);
void player_goto_frame(object *obj, int frame_id);
int player_get_frame(const object *obj);
void player_jump_to_tick(object *obj, int tick);
char player_get_last_frame_letter(const object *obj);
unsigned int player_get_len_ticks(const object *obj);
void player_set_delay(object *obj, int delay);
bool player_is_looping(const object *obj);
uint32_t player_get_current_tick(const object *obj);

#endif // PLAYER_H
