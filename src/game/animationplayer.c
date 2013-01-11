#include "game/animationplayer.h"
#include "game/animation.h"
#include "utils/log.h"
#include <shadowdive/shadowdive.h>

#define MS_PER_OMF_TICK 30

void parser_cb(sd_stringparser_cb_param *param) { 
    DEBUG("d Called."); 
}

int animationplayer_create(animationplayer *player, animation *animation) {
    player->ani = animation;
    player->parser = sd_stringparser_create();
    if(sd_stringparser_set_string(player->parser, animation->bka->animation->anim_string)) {
        sd_stringparser_delete(player->parser);
        PERROR("Unable to initialize stringparser w/ '%s'", animation->bka->animation->anim_string);
        return 1;
    }
    
    sd_stringparser_set_cb(player->parser, "d", parser_cb, 0);
    return 0;
}

void animationplayer_free(animationplayer *player) {
    sd_stringparser_delete(player->parser);
}

void animationplayer_run(animationplayer *player, unsigned int delta) {
    player->ticks += delta;
    sd_stringparser_run(player->parser, player->ticks / MS_PER_OMF_TICK);
}