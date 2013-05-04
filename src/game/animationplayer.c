#include <stdlib.h>
#include <string.h>
#include <shadowdive/shadowdive.h>
#include "utils/log.h"
#include "audio/sound.h"
#include "audio/music.h"
#include "audio/soundloader.h"
#include "audio/sound_state.h"
#include "video/texture.h"
#include "video/video.h"
#include "game/animation.h"
#include "game/animationplayer.h"


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

int isset(sd_stringparser_frame *frame, const char *tag) {
    const sd_stringparser_tag_value *v;
    sd_stringparser_get_tag(frame->parser, frame->id, tag, &v);
    return v->is_set;
}

int get(sd_stringparser_frame *frame, const char *tag) {
    const sd_stringparser_tag_value *v;
    sd_stringparser_get_tag(frame->parser, frame->id, tag, &v);
    return v->value;
}

int dist(int a, int b) {
    return abs((a < b ? a : b) - (a > b ? a : b)) * (a < b ? 1 : -1);
}

int animationplayer_create(animationplayer *player, unsigned int id, animation *animation) {
    sd_stringparser_frame param;
    player->snd = sound_state_create();
    player->ani = animation;
    player->id = id;
    player->direction = 1;
    player->parser = sd_stringparser_create();
    player->ticks = 1;
    player->obj = 0;
    player->finished = 0;
    player->repeat = 0;
    player->userdata = 0;
    player->slide_op.enabled = 0;
    player->slide_op.x_per_tick = 0;
    player->slide_op.x_rem = 0;
    player->slide_op.y_per_tick = 0;
    player->slide_op.y_rem = 0;
    player->add_player = NULL;
    if(sd_stringparser_set_string(player->parser, animation->sdani->anim_string)) {
        sd_stringparser_delete(player->parser);
        PERROR("Unable to initialize stringparser w/ '%s'", animation->sdani->anim_string);
        return 1;
    }
    sd_stringparser_peek(player->parser, 0, &param);
    if(isset(&param, "x=")) {
        player->x = get(&param, "x=");
        DEBUG("Set player->x to %d", player->x);
    }
    if(isset(&param, "y=")) {
        player->y = get(&param, "y=");
        DEBUG("Set player->y to %d", player->y);
    }

    DEBUG("P: '%s'", animation->sdani->anim_string);
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
        int flipmode = s->flipmode;
        if (player->direction == -1) {
            flipmode ^= FLIP_HORIZONTAL;
        }
        video_render_sprite_flip(s->tex, player->x + s->x, player->y + s->y, s->blendmode, flipmode);
    }
}

void animationplayer_run(animationplayer *player) {
    if(player && player->finished) return;

    // Handle slide operation
    if(player->slide_op.enabled) {
        int x = player->slide_op.x_rem + player->slide_op.x_per_tick;
        int y = player->slide_op.y_rem + player->slide_op.y_per_tick;
        // find how many full pixels we can move
        player->x += x / 10;
        player->y += y / 10;
        if(player->obj) {
            player->obj->x += x / 10;
            player->obj->y += y / 10;
        }
        // stash the remaining partial pixels for the next tick
        player->slide_op.x_rem = x % 10;
        player->slide_op.y_rem = y % 10;
    }
    
    // Handle frame switch
    sd_stringparser_frame param;
    sd_stringparser_frame n_param;
    sd_stringparser_frame *f = &param;
    sd_stringparser_frame *n = &n_param;
    int real_frame;
    if(sd_stringparser_run(player->parser, player->ticks-1, &param) == 0) {
        real_frame = param.frame - 65;
        
        // Disable stuff from previous frame
        player->slide_op.enabled = 0;
        
        // Do something if animation is finished!
        if(param.is_animation_end || player->finished) {
            player->finished = 1;
            if(player->repeat) {
                player->ticks = 1;
                player->finished = 0;
                sd_stringparser_run(player->parser, player->ticks-1, &param);
                real_frame = param.frame - 65;
            } else {
                return;
            }
        }
        
        // Kill old sprite, if defined
        incinerate_obj(player);
        
        // Tick management
        if(isset(f, "d"))   { cmd_tickjump(player, get(f, "d")); }
    
        // Animation management
        if(isset(f, "m") && player->add_player != NULL) {
            int mx = isset(f, "mx") ? get(f, "mx") : 0;
            int my = isset(f, "my") ? get(f, "my") : 0;
            player->add_player(player->userdata, get(f, "m"), mx, my);
        }
        if(isset(f, "md") && player->del_player != NULL) { 
            player->del_player(player->userdata, get(f, "md"));
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
            player->slide_op.x_rem = 0;
            player->slide_op.y_per_tick = 0;
            player->slide_op.y_rem = 0;
            int slide = 0;
            if(isset(n, "x=")) {
                slide = get(n, "x=");
                if (slide != player->x) {
                    DEBUG("%d player->x was %d, interpolating to %d", player->id, player->x, slide);
                    // scale distance by 10 so we can handle uneven interpolation (eg. 160/100)
                    player->slide_op.x_per_tick = (dist(player->x, slide) * 10) / param.duration;
                    DEBUG("%d moving %d per tick over %d ticks", player->id, player->slide_op.x_per_tick, param.duration);
                    player->slide_op.enabled = 1;
                }
            }
            if(isset(n, "y=")) { 
                slide = get(n, "y=");
                if (slide != player->y) {
                    DEBUG("%d player->y was %d, interpolating to %d", player->id, player->y, slide);
                    // scale distance by 10 so we can handle uneven interpolation (eg. 160/100)
                    player->slide_op.y_per_tick = (dist(player->y, slide) * 10) / param.duration;
                    DEBUG("%d moving %d per tick over %d ticks", player->id, player->slide_op.y_per_tick, param.duration);
                    player->slide_op.enabled = 1;
                }
            }
        }
        
        // Draw frame sprite
        if(real_frame < 25) {
            sd_sprite *sprite = player->ani->sdani->sprites[real_frame];
            texture *tex = array_get(&player->ani->sprites, real_frame);
            if(tex) {
                aniplayer_sprite *anisprite = malloc(sizeof(aniplayer_sprite));
                if (player->direction == -1) {
                    anisprite->x = (sprite->pos_x * player->direction) - sprite->img->w;
                } else {
                    anisprite->x = sprite->pos_x;
                }
                anisprite->y = sprite->pos_y;
                anisprite->flipmode = FLIP_NONE;
                anisprite->blendmode = isset(f, "br") ? BLEND_ADDITIVE : BLEND_ALPHA;
                if(isset(f, "r")) {
                    anisprite->flipmode |= FLIP_HORIZONTAL;
                }
                if (isset(f, "f")) {
                    anisprite->flipmode |= FLIP_VERTICAL;
                }
                anisprite->tex = tex;
                player->obj = anisprite;
            } else {
                PERROR("No texture @ %u", real_frame);
            }
        } 
    }
    player->ticks++;
    
    // All done.
    return;
}

void animationplayer_set_repeat(animationplayer *player, unsigned int repeat) {
    player->repeat = repeat;
}

void animationplayer_set_direction(animationplayer *player, int direction) {
    player->direction = direction;
}

int animationplayer_get_frame(animationplayer *player) {
    return sd_stringparser_get_current_frame_id(player->parser);
}

void animationplayer_next_frame(animationplayer *player) {
    sd_stringparser_frame param;
    // right now, this can only skip the first frame...
    if(sd_stringparser_run(player->parser, 0, &param) == 0) {
        DEBUG("setting ticks %d -> %d", player->ticks, param.duration);
        player->ticks = param.duration+1;
    }
}

void animationplayer_reset(animationplayer *player) {
    player->ticks = 1;
    player->finished = 0;
}
