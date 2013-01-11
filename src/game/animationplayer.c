#include "game/animationplayer.h"
#include "game/animation.h"
#include "utils/log.h"
#include <shadowdive/shadowdive.h>
#include "audio/music.h"
#include "audio/soundloader.h"

#define MS_PER_OMF_TICK 30

void cb_parser_tickjump(sd_stringparser_cb_param *param) {
    animationplayer *p = param->userdata;
    p->omf_ticks = param->tag_value;
    p->real_ticks = param->tag_value * MS_PER_OMF_TICK; 
    DEBUG("d Called."); 
}

void cb_parser_sound(sd_stringparser_cb_param *param) {
    animationplayer *p = param->userdata;
    soundloader_play(p->ani->soundtable[param->tag_value]-1);
    DEBUG("s Called.");
}

void cb_parser_music_off(sd_stringparser_cb_param *param) {
    music_stop();
    DEBUG("smf Called.");
}

void cb_parser_music_on(sd_stringparser_cb_param *param) {
    switch(param->tag_value) {
        case 0: music_stop(); break;
        case 1: music_play("resources/END.PSM"); break;
        case 2: music_play("resources/MENU.PSM"); break;
        case 3: music_play("resources/ARENA0.PSM"); break;
        case 4: music_play("resources/ARENA1.PSM"); break;
        case 5: music_play("resources/ARENA2.PSM"); break;
        case 6: music_play("resources/ARENA3.PSM"); break;
        case 7: music_play("resources/ARENA4.PSM"); break;
    }

    DEBUG("smo Called.");
}   

int animationplayer_create(animationplayer *player, animation *animation) {
    player->ani = animation;
    player->parser = sd_stringparser_create();
    player->real_ticks = 0;
    player->omf_ticks = 0;
    if(sd_stringparser_set_string(player->parser, animation->sdani->anim_string)) {
        sd_stringparser_delete(player->parser);
        PERROR("Unable to initialize stringparser w/ '%s'", animation->sdani->anim_string);
        return 1;
    }
    DEBUG("P: '%s'", animation->sdani->anim_string);
    
    sd_stringparser_set_cb(player->parser, SD_CB_JUMP_TICK, cb_parser_tickjump, player);
    sd_stringparser_set_cb(player->parser, SD_CB_MUSIC_ON, cb_parser_music_on, player);
    sd_stringparser_set_cb(player->parser, SD_CB_MUSIC_OFF, cb_parser_music_off, player);
    sd_stringparser_set_cb(player->parser, SD_CB_SOUND, cb_parser_sound, player);
    return 0;
}

void animationplayer_free(animationplayer *player) {
    sd_stringparser_delete(player->parser);
}

void animationplayer_run(animationplayer *player, unsigned int delta) {
    player->real_ticks += delta;
    
    // Only run parser when omf ticks change
    unsigned int tmp = (player->real_ticks / MS_PER_OMF_TICK) - player->omf_ticks;
    for(unsigned int i = 0; i < tmp; i++) {
        sd_stringparser_run(player->parser, player->omf_ticks++);
    }
}