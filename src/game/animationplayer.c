#include "game/animationplayer.h"
#include "game/animation.h"
#include "utils/log.h"
#include <shadowdive/shadowdive.h>

#define MS_PER_OMF_TICK 30

void parser_cb(sd_stringparser_cb_param *param) {
    animationplayer *p = param->userdata;
    p->omf_ticks = param->tag_value * MS_PER_OMF_TICK;
    p->real_ticks = param->tag_value;
    DEBUG("d Called."); 
}

int animationplayer_create(animationplayer *player, animation *animation) {
    player->ani = animation;
    player->parser = sd_stringparser_create();
    player->real_ticks = 0;
    player->omf_ticks = 0;
    if(sd_stringparser_set_string(player->parser, animation->bka->animation->anim_string)) {
        sd_stringparser_delete(player->parser);
        PERROR("Unable to initialize stringparser w/ '%s'", animation->bka->animation->anim_string);
        return 1;
    }
    
    sd_stringparser_set_cb(player->parser, "d", parser_cb, player);
    return 0;
}

void animationplayer_free(animationplayer *player) {
    sd_stringparser_delete(player->parser);
}

void animationplayer_run(animationplayer *player, unsigned int delta) {
    player->real_ticks += delta;
    
    // Only run parser when omf ticks change
    unsigned int tmp = player->real_ticks / MS_PER_OMF_TICK;
    if(player->omf_ticks != tmp) {
        player->omf_ticks = tmp;
        sd_stringparser_run(player->parser, player->omf_ticks);
        DEBUG("Real ticks: %u, Omf ticks: %u", player->real_ticks, player->omf_ticks);
    }
}