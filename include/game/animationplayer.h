#ifndef _ANIMATIONPLAYER_H
#define _ANIMATIONPLAYER_H

typedef struct animation_t animation;
typedef struct sd_stringparser_t sd_stringparser;

typedef struct animationplayer_state_t {

} animationplayer_state;

typedef struct animationplayer_t {
    animation *ani;
    sd_stringparser *parser;
    unsigned int real_ticks;
    unsigned int omf_ticks;
    animationplayer_state state;
} animationplayer;

int animationplayer_create(animationplayer *player, animation *animation);
void animationplayer_free(animationplayer *player);
void animationplayer_run(animationplayer *player, unsigned int delta);

#endif // _ANIMATIONPLAYER_H