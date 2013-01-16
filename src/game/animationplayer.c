#include "game/animationplayer.h"
#include "game/animation.h"
#include "utils/log.h"
#include <shadowdive/shadowdive.h>
#include "audio/music.h"
#include "audio/soundloader.h"
#include "video/video.h"
#include "audio/sound.h"
#include "video/texture.h"
#include <stdlib.h>

#include "game/scene.h"


void cmd_tickjump(animationplayer *player, int tick) {
    player->ticks = tick;
}


void cmd_sound(animationplayer *player, int sound) {
    soundloader_play(player->ani->soundtable[sound]-1, player->snd);
}

 // -100 to 100, 0 is the middle
void cmd_sound_pan(animationplayer *player, int pan) {
    if(pan < -100) { pan = -100; }
    if(pan > 100) { pan = 100; }
    player->snd->pan_start = ((char)pan)/100.0f;
    player->snd->pan_end = player->snd->pan_start;
}

 // -100 to 100, 0 is the middle
void cmd_sound_pan_end(animationplayer *player, int pan) {
    if(pan < -100) { pan = -100; }
    if(pan > 100) { pan = 100; }
    player->snd->pan_end = ((char)pan)/100.0f;
    player->snd->pan = player->snd->pan_start;
}

// between 0 and 100 (capped to 100)
void cmd_sound_vol(animationplayer *player, int vol) {
    if(vol < 0) { vol = 0; }
    if(vol > 100) { vol = 100; }
    player->snd->vol = vol/100.0f;
}

// between -16 and 239
void cmd_sound_freq(animationplayer *player, int f) {
    if(f < -16) { f = -16; }
    if(f > 239) { f = 239; }
    float nf = (f/239.0f)*3.0f + 1.0f; // 3x freq multiplier seems to be close to omf
    if(nf < 0.5f) { nf = 0.5f; }
    if(nf > 2.0f) { nf = 2.0f; }
    player->snd->freq = nf;
}

void cmd_music_off() {
    music_stop();
}

void cmd_music_on(int music) {
    switch(music) {
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

void cmd_anim_create(animationplayer *player, int id, int mx, int my, scene *scene) {
    animation *ani = array_get(player->anims, id);
    if(ani != NULL) {
        animationplayer *np = malloc(sizeof(animationplayer));
        animationplayer_create(id, np, ani, player->anims);
        np->x = ani->sdani->start_x + mx;
        np->y = ani->sdani->start_y + my;
        np->scene = scene;
        np->add_player = player->add_player;
        np->del_player = player->del_player;
        player->add_player(player->scene, np);
        DEBUG("Create animation %d @ x,y = %d,%d", id, np->x, np->y);
        return;
    } 
    PERROR("Attempted to create an instance of nonexistent animation!");
}

void cmd_anim_destroy(animationplayer *player, int id) {
    player->del_player(player->scene, id);
    DEBUG("Animation %d killed animation %d.", player->id, id);
}

void incinerate_obj(animationplayer *player) {
    if(player->obj) {
        free(player->obj);
        player->obj = 0;
    }
}

sound_state *sound_state_create() {
    sound_state *s = malloc(sizeof(sound_state));
    memset(s, 0, sizeof(sound_state));
    s->vol = 1.0f;
    s->freq = 1.0f;
    return s;
}

int animationplayer_create(unsigned int id, animationplayer *player, animation *animation, array *anims) {
    player->snd = sound_state_create();
    player->ani = animation;
    player->id = id;
    player->parser = sd_stringparser_create();
    player->ticks = 1;
    player->obj = 0;
    player->finished = 0;
    player->anims = anims;
    player->scene = 0;
    player->slide_op.enabled = 0;
    player->slide_op.x_per_tick = 0;
    player->slide_op.y_per_tick = 0;
    if(sd_stringparser_set_string(player->parser, animation->sdani->anim_string)) {
        sd_stringparser_delete(player->parser);
        PERROR("Unable to initialize stringparser w/ '%s'", animation->sdani->anim_string);
        return 1;
    }
    DEBUG("P: '%s'", animation->sdani->anim_string);
    return 0;
}

int isset(sd_stringparser_frame *frame, const char *tag) {
    for(int i=0; i < frame->num_tags; i++) {
        if(strcmp(frame->tags[i], tag) == 0) {
            return 1;
        }
    }
    return 0;
}

int get(sd_stringparser_frame *frame, const char *tag) {
    for(int i=0; i < frame->num_tags; i++) {
        if(strcmp(frame->tags[i], tag) == 0) {
            return frame->tag_values[i];
        }
    }
    return 0;
}

void animationplayer_free(animationplayer *player) {
    // Free sprite
    incinerate_obj(player);
    
    // Free parser
    sd_stringparser_delete(player->parser);
    player->parser = 0;
    
    // Free Sound State
    free(player->snd);
}

void animationplayer_render(animationplayer *player) {
    if(player != NULL && player->finished) return;

    // Render self
    if(player->obj) {
        aniplayer_sprite *s = player->obj;
        video_render_sprite(s->tex, s->x, s->y, s->blendmode);
    }
}

void animationplayer_run(animationplayer *player) {
    if(player && player->finished) return;

    // Handle slide operation
    if(player->slide_op.enabled) {
        player->x += player->slide_op.x_per_tick;
        player->y += player->slide_op.y_per_tick;
        if(player->obj) {
            player->obj->x += player->slide_op.x_per_tick;
            player->obj->y += player->slide_op.y_per_tick;
        }
    }
    
    // Handle frame switch
    sd_stringparser_frame param;
    sd_stringparser_frame n_param;
    sd_stringparser_frame *f = &param;
    sd_stringparser_frame *n = &n_param;
    if(sd_stringparser_run(player->parser, player->ticks-1, &param) == 0) {
        int real_frame = param.frame - 65;
        
        // Disable stuff from previous frame
        player->slide_op.enabled = 0;
        
        // Do something if animation is finished!
        if(param.is_animation_end || player->finished) {
            player->finished = 1;
            DEBUG("Animationplayer %d asks for removal ...", player->id);
            goto exit_0;
        }
        
        // Kill old sprite, if defined
        incinerate_obj(player);
        
        // Tick management
        if(isset(f, "d"))   { cmd_tickjump(player, get(f, "d")); }
    
        // Animation management
        if(isset(f, "m")) {
            int mx = isset(f, "mx") ? get(f, "mx") : 0;
            int my = isset(f, "my") ? get(f, "my") : 0;
            cmd_anim_create(player, get(f, "m"), mx, my, player->scene);
        }
        if(isset(f, "md")) { 
            cmd_anim_destroy(player, get(f, "md")); 
        }
    
        // Handle music and sounds
        player->snd->freq = 1.0f;
        player->snd->pan = 0.0f;
        player->snd->pan_start = 0.0f;
        player->snd->pan_end = 0.0f;
        if(isset(f, "sf"))  { cmd_sound_freq(player, get(f, "sf"));      }
        if(isset(f, "l"))   { cmd_sound_vol(player, get(f, "l"));        }
        if(isset(f, "sb"))  { cmd_sound_pan(player, get(f, "sb"));       }
        if(isset(f, "sl"))  { cmd_sound_pan_end(player, get(f, "sl"));   }
        if(isset(f, "se"))  { cmd_sound_pan_end(player, get(f, "se")+1); }
        if(isset(f, "smo")) { cmd_music_on(get(f, "smo"));               }
        if(isset(f, "smf")) { cmd_music_off();                           }
        if(isset(f, "s"))   { cmd_sound(player, get(f, "s"));            }
        
        
        // Check if next frame contains X=nnn or Y=nnn 
        if(!param.is_final_frame) {
            sd_stringparser_peek(player->parser, param.id + 1, &n_param);
            player->slide_op.x_per_tick = 0;
            player->slide_op.y_per_tick = 0;
            int slide = 0;
            if(isset(n, "x=")) {
                slide = get(n, "x=");
                player->slide_op.x_per_tick = (abs(player->x) + abs(slide)) / param.duration * (slide < 0 ? -1 : 1);
                player->slide_op.enabled = 1;
            }
            if(isset(n, "y=")) { 
                slide = get(n, "y=");
                player->slide_op.y_per_tick = (abs(player->y) + abs(slide)) / param.duration * (slide < 0 ? -1 : 1);
                player->slide_op.enabled = 1;
            }
        }
        
        // Draw frame sprite
        if(real_frame < 25) {
            sd_sprite *sprite = player->ani->sdani->sprites[real_frame];
            texture *tex = array_get(&player->ani->sprites, real_frame);
            if(tex) {
                aniplayer_sprite *anisprite = malloc(sizeof(aniplayer_sprite));
                anisprite->x = player->x + sprite->pos_x;
                anisprite->y = player->y + sprite->pos_y;
                anisprite->blendmode = isset(f, "br") ? BLEND_ADDITIVE : BLEND_ALPHA;
                anisprite->tex = tex;
                player->obj = anisprite;
            } else {
                PERROR("No texture @ %u", real_frame);
            }
        } 
    }
    player->ticks++;
    
exit_0:
    return;
}