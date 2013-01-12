#include "game/animationplayer.h"
#include "game/animation.h"
#include "utils/log.h"
#include <shadowdive/shadowdive.h>
#include "audio/music.h"
#include "audio/soundloader.h"
#include "video/video.h"
#include "video/texture.h"

void cb_parser_tickjump(sd_stringparser_cb_param *param) {
    animationplayer *p = param->userdata;
    p->ticks = param->tag_value;
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

void cb_parser_anim_create(sd_stringparser_cb_param *param) {
    animationplayer *p = param->userdata;
    p->anim_create_req = param->tag_value;
}

void cb_parser_anim_destroy(sd_stringparser_cb_param *param) {
    animationplayer *p = param->userdata;
    p->anim_destroy_req = param->tag_value;
}

void incinerate_obj(animationplayer *player) {
    if(player->obj) {
        video_queue_remove(player->obj);
        player->obj = 0;
    }
}

void cb_parser_frame_change(sd_stringparser_cb_param *param) {
    animationplayer *p = param->userdata;
    int real_frame = param->frame - 65;

    // Remove old anim frame
    incinerate_obj(p);
    
    // If last frame, ask for removal
    if(param->is_animation_end) {
        p->anim_destroy_req = p->id;
        return;
    }
    
    // 'Z' and up are invalid
    if(real_frame >= 25) return;
    
    // Get real sprite info
    sd_sprite *sprite = p->ani->sdani->sprites[real_frame];
    
    // Get texture
    texture *tex = array_get(&p->ani->sprites, real_frame);
    if(tex) {
        incinerate_obj(p);
        video_queue_add(tex, p->x + sprite->pos_x, p->y + sprite->pos_y, BLEND_ALPHA);
        p->obj = tex;
        DEBUG("Frame %d", real_frame);
    } else {
        PERROR("No texture @ %u", real_frame);
    }
}

int animationplayer_create(unsigned int id, animationplayer *player, animation *animation) {
    player->ani = animation;
    player->id = id;
    player->parser = sd_stringparser_create();
    player->ticks = 1;
    player->anim_destroy_req = -1;
    player->anim_create_req = -1;
    player->obj = 0;
    if(sd_stringparser_set_string(player->parser, animation->sdani->anim_string)) {
        sd_stringparser_delete(player->parser);
        PERROR("Unable to initialize stringparser w/ '%s'", animation->sdani->anim_string);
        return 1;
    }
    DEBUG("P: '%s'", animation->sdani->anim_string);
    
    // Callbacks
    sd_stringparser_set_cb(player->parser, SD_CB_JUMP_TICK, cb_parser_tickjump, player);
    sd_stringparser_set_cb(player->parser, SD_CB_MUSIC_ON, cb_parser_music_on, player);
    sd_stringparser_set_cb(player->parser, SD_CB_MUSIC_OFF, cb_parser_music_off, player);
    sd_stringparser_set_cb(player->parser, SD_CB_SOUND, cb_parser_sound, player);
    sd_stringparser_set_cb(player->parser, SD_CB_CREATE_ANIMATION, cb_parser_anim_create, player);
    sd_stringparser_set_cb(player->parser, SD_CB_DESTROY_ANIMATION, cb_parser_anim_destroy, player);
    sd_stringparser_set_frame_change_cb(player->parser, cb_parser_frame_change, player);
    return 0;
}

void animationplayer_free(animationplayer *player) {
    // Free stringparser
    incinerate_obj(player);
    sd_stringparser_delete(player->parser);
    player->parser = 0;
}

void animationplayer_run(animationplayer *player) {
    if(player && player->parser) {
        sd_stringparser_run(player->parser, player->ticks-1);
        player->ticks++;
    }
}