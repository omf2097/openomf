#ifndef _ANIMATIONPLAYER_H
#define _ANIMATIONPLAYER_H

#include "utils/list.h"
#include "utils/array.h"

typedef struct texture_t texture;
typedef struct animation_t animation;
typedef struct sd_stringparser_t sd_stringparser;
typedef struct animationplayer_t animationplayer;
typedef struct scene_t scene;

typedef struct aniplayer_sprite_t {
    int x,y;
    texture *tex;
    int blendmode;
} aniplayer_sprite;

typedef struct animationplayer_t {
    unsigned int id;
    unsigned int finished;
    unsigned int ticks;
    
    animation *ani;
    array *anims;
    sd_stringparser *parser;
    
    unsigned int x,y;
    aniplayer_sprite *obj;
    scene *scene;
    
    void (*del_player)(scene *scene, int id);
    void (*add_player)(scene *scene, animationplayer *player);
} animationplayer;

int animationplayer_create(unsigned int id, animationplayer *player, animation *animation, array *anims);
void animationplayer_free(animationplayer *player);
void animationplayer_run(animationplayer *player);
void animationplayer_render(animationplayer *player);

#endif // _ANIMATIONPLAYER_H