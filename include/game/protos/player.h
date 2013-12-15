#ifndef _PLAYER_H
#define _PLAYER_H

#include "utils/vec.h"

typedef struct object_t object;
typedef struct sd_stringparser_t sd_stringparser;

typedef void (*object_state_add_cb)(object *parent, int id, vec2i pos, int g, void *userdata);
typedef void (*object_state_del_cb)(object *parent, int id, void *userdata);

typedef struct player_sprite_state_t {
    int blendmode;
    int flipmode;
    int method_flags;
    int blend_start;
    int blend_finish;
    int timer;
    int duration;

    int pal_ref_index; // bpd
    int pal_entry_count; // bpn
    int pal_start_index; // bps
    int pal_begin; // bpb
    int pal_end; // bpd
    int pal_tint; // bz
} player_sprite_state;

typedef struct player_slide_op_t {
    vec2f vel;
    int timer;
} player_slide_state;

typedef struct player_animation_state_t {
    unsigned int finished;
    unsigned int ticks;
    unsigned int ticks_len;
    unsigned int repeat;
    unsigned int reverse;
    unsigned int end_frame;
    int previous;
    sd_stringparser *parser;
    int enemy_x, enemy_y;

    void *spawn_userdata;
    void *destroy_userdata;
    object_state_add_cb spawn;
    object_state_del_cb destroy;
} player_animation_state;

void player_create(object *obj);
void player_free(object *obj);
void player_reload(object *obj);
void player_reload_with_str(object *obj, const char *str);
void player_reset(object *obj);
void player_run(object *obj);
void player_set_repeat(object *obj, int repeat);
int player_get_repeat(object *obj);
void player_set_end_frame(object *obj, int end_frame);
void player_next_frame(object *obj);
void player_goto_frame(object *obj, int frame_id);
int player_get_frame(object *obj);
void player_jump_to_tick(object *obj, int tick);
char player_get_frame_letter(object *obj);
unsigned int player_get_len_ticks(object *obj);

#endif // _PLAYER_H