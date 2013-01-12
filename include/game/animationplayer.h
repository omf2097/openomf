#ifndef _ANIMATIONPLAYER_H
#define _ANIMATIONPLAYER_H

typedef struct texture_t texture;
typedef struct animation_t animation;
typedef struct sd_stringparser_t sd_stringparser;

typedef struct animationplayer_state_t {

} animationplayer_state;

typedef struct animationplayer_t {
    unsigned int id;
    animation *ani;
    sd_stringparser *parser;
    unsigned int ticks;
    animationplayer_state state;
    
    unsigned int x,y;
    texture *obj;
    
    int anim_destroy_req;
    int anim_create_req;
} animationplayer;

int animationplayer_create(unsigned int id, animationplayer *player, animation *animation);
void animationplayer_free(animationplayer *player);
void animationplayer_run(animationplayer *player);

#endif // _ANIMATIONPLAYER_H