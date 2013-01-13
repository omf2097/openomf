#ifndef _ANIMATIONPLAYER_H
#define _ANIMATIONPLAYER_H

#include "utils/list.h"
#include "utils/array.h"

typedef struct aniplayer_sprite_t {
    int x,y;
    texture *tex;
    int blendmode;
} aniplayer_sprite;

typedef struct aniplayer_slide_op_t {
    int x_per_tick;
    int y_per_tick;
    int enabled;
} aniplayer_slide_op;

typedef struct animationplayer_t {
    unsigned int id;
    unsigned int finished;
    unsigned int ticks;
    
    animation *ani;
    sound_state *snd;
    array *anims;
    sd_stringparser *parser;
    
    aniplayer_slide_op slide_op;
    
    int x,y;
    aniplayer_sprite *obj;
    scene *scene;
    
    void (*del_player)(scene *scene, int id);
    void (*add_player)(scene *scene, struct animationplayer_t *player);
} animationplayer;

int animationplayer_create(unsigned int id, animationplayer *player, animation *animation, array *anims);
void animationplayer_free(animationplayer *player);
void animationplayer_run(animationplayer *player);
void animationplayer_render(animationplayer *player);

#endif // _ANIMATIONPLAYER_H
