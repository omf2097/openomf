#include <stdlib.h>
#include <string.h>
#include <shadowdive/shadowdive.h>
#include "utils/log.h"
#include "audio/stream.h"
#include "audio/audio.h"
#include "audio/sound.h"
#include "audio/music.h"
#include "audio/soundloader.h"
#include "audio/sound_state.h"
#include "video/texture.h"
#include "video/video.h"
#include "game/animation.h"
#include "game/animationplayer.h"
#include "game/settings.h"


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
    player->snd->vol = vol/100.0f * (settings_get()->sound.sound_vol/10.0f);
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
    audio_set_volume(TYPE_MUSIC, settings_get()->sound.music_vol/10.0f);
}

void incinerate_obj(animationplayer *player) {
    if(player->sprite_obj) {
        free(player->sprite_obj);
        player->sprite_obj = 0;
    }
}

sound_state *sound_state_create() {
    sound_state *s = malloc(sizeof(sound_state));
    memset(s, 0, sizeof(sound_state));
    s->vol = 1.0f * (settings_get()->sound.sound_vol/10.0f);
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

int animationplayer_create(animationplayer *player, unsigned int id, animation *animation, object *pobj) {
    player->pobj = pobj;
    player->snd = sound_state_create();
    player->ani = animation;
    player->id = id;
    player->direction = 1;
    player->reverse = 0;
    player->end_frame = UINT32_MAX;
    player->ticks = 1;
    player->sprite_obj = NULL;
    player->finished = 0;
    player->repeat = 0;
    player->userdata = NULL;
    player->slide_op.enabled = 0;
    player->slide_op.x_per_tick = 0;
    player->slide_op.x_rem = 0;
    player->slide_op.y_per_tick = 0;
    player->slide_op.y_rem = 0;
    player->add_player = NULL;
    player->parser = NULL;
    return animationplayer_set_string(player, animation->sdani->anim_string);
}

int animationplayer_set_string(animationplayer *player, const char *string) {
    sd_stringparser_frame param;
    if (player->parser) {
        sd_stringparser_delete(player->parser);
    }
    player->parser = sd_stringparser_create();
    if(sd_stringparser_set_string(player->parser, string)) {
        sd_stringparser_delete(player->parser);
        PERROR("Unable to initialize stringparser w/ '%s'", string);
        return 1;
    }
    sd_stringparser_peek(player->parser, 0, &param);
    
    // Set pos
    int opx, opy;
    if(isset(&param, "x=")) {
        object_get_pos(player->pobj, &opx, &opy);
        object_set_pos(player->pobj, get(&param, "y="), opy);
    }
    if(isset(&param, "y=")) {
        object_get_pos(player->pobj, &opx, &opy);
        object_set_pos(player->pobj, opx, get(&param, "y="));
    }
    DEBUG("P: '%s'", string);
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
    if(player->finished) return;

    // Render self
    if(player->sprite_obj) {
        int x, y;
        aniplayer_sprite *s = player->sprite_obj;
        int flipmode = s->flipmode;
        if (player->direction == -1) {
            flipmode ^= FLIP_HORIZONTAL;
        }
        object_get_pos(player->pobj, &x, &y);
        video_render_sprite_flip(s->tex, x + s->x, y + s->y, s->blendmode, flipmode);
    }
}

void animationplayer_run(animationplayer *player) {
    if(player->finished) return;

    // Handle slide operation
    // TODO make the physics object handle this, or something
    if(player->slide_op.enabled) {
        int x = player->slide_op.x_rem + player->slide_op.x_per_tick;
        int y = player->slide_op.y_rem + player->slide_op.y_per_tick;
        // find how many full pixels we can move
        object_add_pos(player->pobj, x / 100, y / 100);
        // stash the remaining partial pixels for the next tick
        player->slide_op.x_rem = x % 100;
        player->slide_op.y_rem = y % 100;
    }

    int run_ret;
    if(player->end_frame == UINT32_MAX) {
        run_ret = sd_stringparser_run(player->parser, player->ticks-1);
    } else {
        run_ret = sd_stringparser_run_frames(player->parser, player->ticks-1, player->end_frame);
    }
    if(run_ret == 0) {
        // Handle frame switch
        sd_stringparser_frame *param = &player->parser->current_frame;
        sd_stringparser_frame n_param;
        sd_stringparser_frame *f = param;
        sd_stringparser_frame *n = &n_param;
        int real_frame;

        real_frame = param->letter - 65;
        
        // Disable stuff from previous frame
        player->slide_op.enabled = 0;
        
        // Do something if animation is finished!
        if(param->is_animation_end || player->finished) {
            player->finished = 1;
            if(player->repeat) {
                animationplayer_reset(player);
                sd_stringparser_run(player->parser, player->ticks-1);
                real_frame = param->letter - 65;
            } else {
                return;
            }
        }
        
        // Kill old sprite, if defined
        incinerate_obj(player);
        
        // Tick management
        if(isset(f, "d"))   {
            cmd_tickjump(player, get(f, "d"));
            sd_stringparser_reset(player->parser);
        }
    
        // Animation management
        if(isset(f, "m") && player->add_player != NULL) {
            int mx = isset(f, "mx") ? get(f, "mx") : 0;
            int my = isset(f, "my") ? get(f, "my") : 0;
            int mg = isset(f, "mg") ? get(f, "mg") : 0;
            player->add_player(player->userdata, get(f, "m"), mx, my, mg);
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
        if (isset(f, "v")) {
            int x = 0, y = 0;
            if(isset(f, "y-")) {
                y = get(f, "y-") * -1;
            } else if(isset(f, "y+")) {
                y = get(f, "y+");
            }
            if(isset(f, "x-")) {
                x = get(f, "x-") * -1 * player->direction;
            } else if(isset(f, "x+")) {
                x = get(f, "x+") * player->direction;
            }

            if (x || y) {
                object_add_vel(player->pobj, x, y);
            }
        }
        if (isset(f, "e")) {
            // x,y relative to *enemy's* position
            int x = 0, y = 0;
            if(isset(f, "y-")) {
                y = get(f, "y-") * -1;
            } else if(isset(f, "y+")) {
                y = get(f, "y+");
            }
            if(isset(f, "x-")) {
                x = get(f, "x-") * -1 * player->direction;
            } else if(isset(f, "x+")) {
                x = get(f, "x+") * player->direction;
            }

            int xpos, ypos;
            object_get_pos(player->pobj, &xpos, &ypos);
            int x_dist = dist(xpos, player->enemy_x + x);
            int y_dist = dist(ypos, player->enemy_y + y);
            /*DEBUG("xdist %d %d + %d -> %d, ydist %d %d + %d -> %d", player->phy->pos.x, player->enemy_x, x, x_dist, player->phy->pos.y, player->enemy_y, y, y_dist);*/
            player->slide_op.enabled = 1;
            player->slide_op.x_per_tick = x_dist / (float)param->duration;
            player->slide_op.y_per_tick = y_dist / (float)param->duration;
        }
        if (isset(f, "v") == 0 && isset(f, "e") == 0 && (isset(f, "x+") || isset(f, "y+") || isset(f, "x-") || isset(f, "y-"))) {
            // check for relative X interleaving
            int x = 0, y = 0;
            if(isset(f, "y-")) {
                y = get(f, "y-") * -1;
            } else if(isset(f, "y+")) {
                y = get(f, "y+");
            }
            if(isset(f, "x-")) {
                x = get(f, "x-") * -1 * player->direction;
            } else if(isset(f, "x+")) {
                x = get(f, "x+") * player->direction;
            }

            /*DEBUG("x %d, y %d => x %d, y %d", player->phy->pos.x, player->phy->pos.y, x, y);*/
            player->slide_op.enabled = 1;
            player->slide_op.x_per_tick = x / (float)param->duration;
            player->slide_op.y_per_tick = y / (float)param->duration;
        }


        // Check if next frame contains X=nnn or Y=nnn 
        if(!param->is_final_frame) {
            sd_stringparser_peek(player->parser, param->id + 1, &n_param);
            player->slide_op.x_per_tick = 0;
            player->slide_op.x_rem = 0;
            player->slide_op.y_per_tick = 0;
            player->slide_op.y_rem = 0;
            int slide = 0;
            int xpos, ypos;
            if(isset(n, "x=")) {
                slide = get(n, "x=");
                if (player->direction == -1) {
                    // if the sprite is flipped horizontally, adjust the X coordinates
                    slide = 320 - slide;
                }
                object_get_pos(player->pobj, &xpos, &ypos);
                if (slide != xpos) {
                    DEBUG("%d player->x was %d, interpolating to %d", player->id, xpos, slide);
                    // scale distance by 101 so we can handle uneven interpolation (eg. 160/100)
                    player->slide_op.x_per_tick = (dist(xpos, slide) * 100) / param->duration;
                    DEBUG("%d moving %d per tick over %d ticks for total %d", player->id, player->slide_op.x_per_tick, param->duration, dist(xpos, slide));
                    player->slide_op.enabled = 1;
                }
            }
            if(isset(n, "y=")) { 
                slide = get(n, "y=");
                object_get_pos(player->pobj, &xpos, &ypos);
                if (slide != ypos) {
                    DEBUG("%d player->y was %d, interpolating to %d", player->id, ypos, slide);
                    // scale distance by 100 so we can handle uneven interpolation (eg. 160/100)
                    player->slide_op.y_per_tick = (dist(ypos, slide) * 100) / param->duration;
                    DEBUG("%d moving %d per tick over %d ticks", player->id, player->slide_op.y_per_tick, param->duration);
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
                player->sprite_obj = anisprite;
                /*object_set_collision_box(player->pobj, anisprite->tex->w, anisprite->tex->h);*/
            } else {
                PERROR("No texture @ %u", real_frame);
            }
        } 
    }

    if(player->reverse) {
        player->ticks--;
    } else {
        player->ticks++;
    }
    
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

char animationplayer_get_frame_letter(animationplayer *player) {
    return sd_stringparser_get_current_frame_letter(player->parser);
}

void animationplayer_next_frame(animationplayer *player) {
    // right now, this can only skip the first frame...
    if(sd_stringparser_run(player->parser, 0) == 0) {
        DEBUG("setting ticks %d -> %d", player->ticks, player->parser->current_frame.duration);
        player->ticks = player->parser->current_frame.duration+1;
    }
}

void animationplayer_goto_frame(animationplayer *player, unsigned int frame_id) {
    sd_stringparser_goto_frame(player->parser, frame_id, &player->ticks);
    player->ticks++;
}

void animationplayer_set_end_frame(animationplayer *player, unsigned int end_frame) {
    player->end_frame = end_frame;
}

void animationplayer_reset(animationplayer *player) {
    player->ticks = 1;
    player->finished = 0;
    sd_stringparser_reset(player->parser);
}
