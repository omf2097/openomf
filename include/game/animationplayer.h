#ifndef _ANIMATIONPLAYER_H
#define _ANIMATIONPLAYER_H

#include "utils/list.h"
#include "video/texture.h"
#include "audio/sound_state.h"
#include "game/physics/space.h"
#include "game/physics/object.h"

typedef struct aniplayer_sprite_t {
    int x,y;
    texture *tex;
    int blendmode;
    int flipmode;
} aniplayer_sprite;

typedef struct aniplayer_slide_op_t {
    int x_per_tick;
    int x_rem;
    int y_per_tick;
    int y_rem;
    int enabled;
} aniplayer_slide_op;

typedef struct animationplayer_t {
    object *pobj;
    unsigned int id;
    unsigned int finished;
    unsigned int ticks;
    
    unsigned int repeat;
    unsigned int reverse;
    int direction;
    unsigned int end_frame;
    
    animation *ani;
    sound_state *snd;
    sd_stringparser *parser;
    
    aniplayer_slide_op slide_op;
    
    int enemy_x, enemy_y;
    aniplayer_sprite *sprite_obj;
    void *userdata;
    
    void (*del_player)(void *userdata, int id);
    void (*add_player)(void *userdata, int id, int mx, int my, int mg);
} animationplayer;

int animationplayer_create(animationplayer *player, unsigned int id, animation *animation, object *pobj);
int animationplayer_set_string(animationplayer *player, const char *string);
void animationplayer_free(animationplayer *player);
void animationplayer_run(animationplayer *player);
void animationplayer_render(animationplayer *player);
void animationplayer_reset(animationplayer *player);
void animationplayer_set_repeat(animationplayer *player, unsigned int repeat);
void animationplayer_set_direction(animationplayer *player, int direction);
void animationplayer_set_end_frame(animationplayer *player, unsigned int end_frame);
void animationplayer_next_frame(animationplayer *player);
void animationplayer_goto_frame(animationplayer *player, unsigned int frame_id);
int animationplayer_get_frame(animationplayer *player);
char animationplayer_get_frame_letter(animationplayer *player);

#endif // _ANIMATIONPLAYER_H
