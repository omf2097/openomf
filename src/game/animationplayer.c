#include "game/animationplayer.h"
#include "game/animation.h"
#include "utils/log.h"
#include <shadowdive/shadowdive.h>
#include "audio/music.h"
#include "audio/soundloader.h"
#include "video/video.h"
#include "video/texture.h"
#include <stdlib.h>

void cb_parser_tickjump(sd_stringparser_cb_param *param) {
    animationplayer *p = param->userdata;
    p->ticks = param->tag_value;
}

void cb_parser_sound(sd_stringparser_cb_param *param) {
    animationplayer *p = param->userdata;
    soundloader_play(p->ani->soundtable[param->tag_value]-1);
}

void cb_parser_music_off(sd_stringparser_cb_param *param) {
    music_stop();
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
}

void cb_parser_anim_create(sd_stringparser_cb_param *param) {
    animationplayer *p = param->userdata;
    int id = param->tag_value;
    animation *ani = array_get(p->anims, id);
    if(ani != NULL) {
        animationplayer *np = malloc(sizeof(animationplayer));
        animationplayer_create(id, np, ani, p->anims, p);
        np->x = ani->sdani->start_x;
        np->y = ani->sdani->start_y;
        list_push_last(&p->children, np);
        DEBUG("Create animation %d @ x,y = %d,%d", id, np->x, np->y);
    } else {
        PERROR("Attempted to create an instance of nonexistent animation!");
    }
}

void animationplayer_destroy_child(animationplayer *player, int id) {
    // Attempt to kill child from this node
    list_iterator it;
    list_iter(&player->children, &it);
    animationplayer *tmp;
    while((tmp = list_next(&it)) != 0) {
        if(tmp->id == id) {
            list_delete(&player->children, &it);
            animationplayer_free(tmp);
            free(tmp);
            return;
        }
    }
    
    // Animation was not found from this level. Check sister animations.
    if(player->parent) {
        animationplayer_destroy_child(player->parent, id);
    }
}

void cb_parser_anim_destroy(sd_stringparser_cb_param *param) {
    // Try to kill animation
    animationplayer *player = param->userdata;
    animationplayer_destroy_child(player, param->tag_value);
}

void cb_parser_blendmode_additive(sd_stringparser_cb_param *param) {
    animationplayer_state *state = param->userdata;
    state->blendmode = BLEND_ADDITIVE;
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
        p->finished = 1;
        DEBUG("Animationplayer %d asks for removal ...", p->id);
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
        video_queue_add(tex, p->x + sprite->pos_x, p->y + sprite->pos_y, p->state.blendmode);
        p->obj = tex;
        DEBUG("Frame %d, x,y = %d,%d, blendmode = %s", real_frame, p->x + sprite->pos_x, p->y + sprite->pos_y, (p->state.blendmode == BLEND_ALPHA ? "alpha" : "additive"));
    } else {
        PERROR("No texture @ %u", real_frame);
    }
}

int animationplayer_create(unsigned int id, animationplayer *player, animation *animation, array *anims, animationplayer *parent) {
    player->ani = animation;
    player->id = id;
    player->parser = sd_stringparser_create();
    player->ticks = 1;
    player->obj = 0;
    player->finished = 0;
    player->anims = anims;
    player->parent = parent;
    list_create(&player->children);
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
    
    // State changes
    sd_stringparser_set_cb(player->parser, SD_CB_BLEND_ADDITIVE, cb_parser_blendmode_additive, &player->state);
    
    // Frame change
    sd_stringparser_set_frame_change_cb(player->parser, cb_parser_frame_change, player);
    return 0;
}

void animationplayer_free(animationplayer *player) {
    // Free all children
    list_iterator it;
    list_iter(&player->children, &it);
    animationplayer *tmp;
    while((tmp = list_next(&it)) != 0) {
        animationplayer_free(tmp);
        free(tmp);
    }
    list_free(&player->children);
    
    // Free sprite
    incinerate_obj(player);
    
    // Free parser
    sd_stringparser_delete(player->parser);
    player->parser = 0;
}

void reset_state(animationplayer_state *state) {
    state->blendmode = BLEND_ALPHA;
}

void animationplayer_run(animationplayer *player) {
    if(player && player->parser && !player->finished) {
        reset_state(&player->state);
        sd_stringparser_run(player->parser, player->ticks-1);
        player->ticks++;
    }

    list_iterator it;
    list_iter(&player->children, &it);
    animationplayer *tmp;
    while((tmp = list_next(&it)) != 0) {
        animationplayer_run(tmp);
        if(tmp->finished) {
            list_delete(&player->children, &it);
            DEBUG("Player %d deleted animationplayer %d.", player->id, tmp->id);
            animationplayer_free(tmp);
            free(tmp);
        }
    }
}