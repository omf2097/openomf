#include <inttypes.h>
#include <stdlib.h>

#include <shadowdive/stringparser.h>

#include "game/settings.h"
#include "video/video.h"
#include "audio/audio.h"
#include "audio/sound_state.h"
#include "audio/soundloader.h"
#include "audio/music.h"
#include "game/protos/player.h"
#include "game/protos/object.h"
#include "utils/string.h"

// ---------------- Private callbacks ---------------- 

void cmd_tickjump(object *obj, int tick) {
    obj->animation_state.ticks = tick;
}

void cmd_sound(object *obj, int sound) {
    soundloader_play(obj->sound_translation_table[sound] - 1, obj->animation_state.sound_state);
}

 // -100 to 100, 0 is the middle
void cmd_sound_pan(object *obj, int pan) {
    sound_state *snd = obj->animation_state.sound_state;
    if(pan < -100) { pan = -100; }
    if(pan > 100) { pan = 100; }
    snd->pan_start = ((char)pan)/100.0f;
    snd->pan_end = snd->pan_start;
}

 // -100 to 100, 0 is the middle
void cmd_sound_pan_end(object *obj, int pan) {
    sound_state *snd = obj->animation_state.sound_state;
    if(pan < -100) { pan = -100; }
    if(pan > 100) { pan = 100; }
    snd->pan_end = ((char)pan)/100.0f;
    snd->pan = snd->pan_start;
}

// between 0 and 100 (capped to 100)
void cmd_sound_vol(object *obj, int vol) {
    sound_state *snd = obj->animation_state.sound_state;
    if(vol < 0) { vol = 0; }
    if(vol > 100) { vol = 100; }
    snd->vol = vol/100.0f * (settings_get()->sound.sound_vol/10.0f);
}

// between -16 and 239
void cmd_sound_freq(object *obj, int f) {
    sound_state *snd = obj->animation_state.sound_state;
    if(f < -16) { f = -16; }
    if(f > 239) { f = 239; }
    float nf = (f/239.0f)*3.0f + 1.0f; // 3x freq multiplier seems to be close to omf
    if(nf < 0.5f) { nf = 0.5f; }
    if(nf > 2.0f) { nf = 2.0f; }
    snd->freq = nf;
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

// ---------------- Private functions ---------------- 

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

// ---------------- Public functions ---------------- 

void player_create(object *obj) {
    obj->animation_state.reverse = 0;
    obj->animation_state.end_frame = UINT32_MAX;
    obj->animation_state.ticks = 1;
    obj->animation_state.finished = 0;
    obj->animation_state.repeat = 0;
    obj->animation_state.enemy_x = 0;
    obj->animation_state.enemy_y = 0;
    obj->animation_state.add_player = NULL;
    obj->animation_state.parser = NULL;
}

void player_free(object *obj) {
    if(obj->animation_state.parser != NULL) {
        sd_stringparser_delete(obj->animation_state.parser);
    }
}

void player_reload(object *obj) {
    // Unload old parser, if any
    if(obj->animation_state.parser != NULL) {
        sd_stringparser_delete(obj->animation_state.parser);
        obj->animation_state.parser = NULL;
    }

    // Load new parser
    obj->animation_state.parser = sd_stringparser_create();
    sd_stringparser_set_string(
        obj->animation_state.parser, 
        str_c(&obj->cur_animation->animation_string));

    // Peek parameters
    sd_stringparser_frame param;
    sd_stringparser_peek(obj->animation_state.parser, 0, &param);
    
    // Set initial position for sprite
    if(isset(&param, "x=")) {
        object_set_px(obj, get(&param, "y="));
    }
    if(isset(&param, "y=")) {
        object_set_py(obj, get(&param, "y="));
    }

    // Set player state
    obj->animation_state.ticks = 1;
    obj->animation_state.finished = 0;
}

void player_reset(object *obj) {
    obj->animation_state.ticks = 1;
    obj->animation_state.finished = 0;
    sd_stringparser_reset(obj->animation_state.parser);
}

void player_run(object *obj) {
    // Some vars for easier life
    player_animation_state *state = &obj->animation_state;
    if(state->finished) return;

    // Not sure what this does
    int run_ret;
    if(state->end_frame == UINT32_MAX) {
        run_ret = sd_stringparser_run(state->parser, state->ticks - 1);
    } else {
        run_ret = sd_stringparser_run_frames(state->parser, state->ticks-1, state->end_frame);
    }

    // Handle frame
    if(run_ret == 0) {
        // Handle frame switch
        sd_stringparser_frame *param = &state->parser->current_frame;
        //sd_stringparser_frame n_param;
        sd_stringparser_frame *f = param;
        //sd_stringparser_frame *n = &n_param;
        int real_frame;

        real_frame = param->letter - 65;
        
        // Do something if animation is finished!
        if(param->is_animation_end || state->finished) {
            state->finished = 1;
            if(state->repeat) {
                player_reset(obj);
                sd_stringparser_run(state->parser, state->ticks - 1);
                real_frame = param->letter - 65;
            } else {
                return;
            }
        }
        
        // Tick management
        if(isset(f, "d"))   {
            cmd_tickjump(obj, get(f, "d"));
            sd_stringparser_reset(state->parser);
        }
    
        // Animation management
        if(isset(f, "m") && state->add_player != NULL) {
            int mx = isset(f, "mx") ? get(f, "mx") : 0;
            int my = isset(f, "my") ? get(f, "my") : 0;
            int mg = isset(f, "mg") ? get(f, "mg") : 0;
            state->add_player(obj, get(f, "m"), mx, my, mg);
        }
        if(isset(f, "md") && state->del_player != NULL) { 
            state->del_player(obj, get(f, "md"));
        }
    
        // Handle music and sounds
        state->sound_state->freq = 1.0f;
        state->sound_state->pan = 0.0f;
        state->sound_state->pan_start = 0.0f;
        state->sound_state->pan_end = 0.0f;
        if(isset(f, "sf"))  { cmd_sound_freq(obj, get(f, "sf"));      }
        if(isset(f, "l"))   { cmd_sound_vol(obj, get(f, "l"));        }
        if(isset(f, "sb"))  { cmd_sound_pan(obj, get(f, "sb"));       }
        if(isset(f, "sl"))  { cmd_sound_pan_end(obj, get(f, "sl"));   }
        if(isset(f, "se"))  { cmd_sound_pan_end(obj, get(f, "se")+1); }
        if(isset(f, "smo")) { cmd_music_on(get(f, "smo"));            }
        if(isset(f, "smf")) { cmd_music_off();                        }
        if(isset(f, "s"))   { cmd_sound(obj, get(f, "s"));            }
        if (isset(f, "v")) {
            int x = 0, y = 0;
            if(isset(f, "y-")) {
                y = get(f, "y-") * -1;
            } else if(isset(f, "y+")) {
                y = get(f, "y+");
            }
            if(isset(f, "x-")) {
                x = get(f, "x-") * -1 * object_get_direction(obj);
            } else if(isset(f, "x+")) {
                x = get(f, "x+") * object_get_direction(obj);
            }

            if (x || y) {
                object_add_vel(obj, x, y);
            }
        }
        /*if (isset(f, "e")) {
            // x,y relative to *enemy's* position
            int x = 0, y = 0;
            if(isset(f, "y-")) {
                y = get(f, "y-") * -1;
            } else if(isset(f, "y+")) {
                y = get(f, "y+");
            }
            if(isset(f, "x-")) {
                x = get(f, "x-") * -1 * object_get_direction(obj);
            } else if(isset(f, "x+")) {
                x = get(f, "x+") * object_get_direction(obj);
            }

            int xpos, ypos;
            object_get_pos(obj, &xpos, &ypos);
            int x_dist = dist(xpos, state->enemy_x + x);
            int y_dist = dist(ypos, state->enemy_y + y);
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
                x = get(f, "x-") * -1 * object_get_direction(obj);
            } else if(isset(f, "x+")) {
                x = get(f, "x+") * object_get_direction(obj);
            }

            player->slide_op.enabled = 1;
            player->slide_op.x_per_tick = x / (float)param->duration;
            player->slide_op.y_per_tick = y / (float)param->duration;
        }*/


        // Check if next frame contains X=nnn or Y=nnn 
        /*
        if(!param->is_final_frame) {
            sd_stringparser_peek(state->parser, param->id + 1, &n_param);
            int slide = 0;
            int xpos, ypos;
            if(isset(n, "x=")) {
                slide = get(n, "x=");
                if(object_get_direction(obj) == OBJECT_FACE_LEFT) {
                    // if the sprite is flipped horizontally, adjust the X coordinates
                    slide = 320 - slide;
                }
                object_get_pos(obj, &xpos, &ypos);
            }
            if(isset(n, "y=")) { 
                slide = get(n, "y=");
                object_get_pos(obj, &xpos, &ypos);
            }
        }*/
        
        // Set render settings
        if(real_frame < 25) {
            sprite *sprite = animation_get_sprite(obj->cur_animation, real_frame);
            obj->cur_sprite = sprite;
            if(sprite != NULL) {
                obj->sprite_state.flipmode = FLIP_NONE;
                obj->sprite_state.blendmode = isset(f, "br") ? BLEND_ADDITIVE : BLEND_ALPHA;
                if(isset(f, "r")) {
                    obj->sprite_state.flipmode |= FLIP_HORIZONTAL;
                }
                if (isset(f, "f")) {
                    obj->sprite_state.flipmode |= FLIP_VERTICAL;
                }
            }
        } 
    }

    if(state->reverse) {
        state->ticks--;
    } else {
        state->ticks++;
    }
    
    // All done.
    return;
}

void player_set_repeat(object *obj, int repeat) {
	obj->animation_state.repeat = repeat;
}

int player_get_repeat(object *obj) {
    return obj->animation_state.repeat;
}

void player_set_end_frame(object *obj, int end_frame) {
	obj->animation_state.end_frame = end_frame;
}

void player_next_frame(object *obj) {
    // right now, this can only skip the first frame...
    if(sd_stringparser_run(obj->animation_state.parser, 0) == 0) {
        obj->animation_state.ticks = obj->animation_state.parser->current_frame.duration + 1;
    }
}

void player_goto_frame(object *obj, int frame_id) {
    sd_stringparser_goto_frame(obj->animation_state.parser, frame_id, &obj->animation_state.ticks);
    obj->animation_state.ticks++;
}

int player_get_frame(object *obj) {
	return sd_stringparser_get_current_frame_id(obj->animation_state.parser);
}

char player_get_frame_letter(object *obj) {
	return sd_stringparser_get_current_frame_letter(obj->animation_state.parser);
}