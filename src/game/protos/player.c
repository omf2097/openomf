#include <inttypes.h>
#include <stdlib.h>
#include <shadowdive/stringparser.h>

#include "game/settings.h"
#include "video/video.h"
#include "audio/sink.h"
#include "audio/sound.h"
#include "audio/music.h"
#include "resources/ids.h"
#include "game/protos/player.h"
#include "game/protos/object.h"
#include "utils/string.h"
#include "utils/log.h"

// --------------------- Helpers ---------------------

static int clamp(int val, int min, int max) {
    if(val > max) return max;
    if(val < min) return min;
    return val;
}

static float clampf(float val, float min, float max) {
    if(val > max) return max;
    if(val < min) return min;
    return val;
}

// ---------------- Private callbacks ---------------- 

void cmd_tickjump(object *obj, int tick) {
    obj->animation_state.ticks = tick;
}

void cmd_music_off() {
    music_stop();
}

void cmd_music_on(int music) {
    // 0 is special case; stops playback
    if(music == 0) {
        music_stop();
        return;
    }

    // Find file we want to play
    char filename[64];
    switch(music) {
        case 0: music_stop(); break;
        case 1: get_filename_by_id(PSM_END, filename); break;
        case 2: get_filename_by_id(PSM_MENU, filename); break;
        case 3: get_filename_by_id(PSM_ARENA0, filename); break;
        case 4: get_filename_by_id(PSM_ARENA1, filename); break;
        case 5: get_filename_by_id(PSM_ARENA2, filename); break;
        case 6: get_filename_by_id(PSM_ARENA3, filename); break;
        case 7: get_filename_by_id(PSM_ARENA4, filename); break;
    }
    music_play(filename);
    music_set_volume(settings_get()->sound.music_vol/10.0f);
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

float dist(float a, float b) {
    return abs((a < b ? a : b) - (a > b ? a : b)) * (a < b ? 1 : -1);
}

void player_clear_frame(object *obj) {
    player_sprite_state *s = &obj->sprite_state;
    s->blendmode = BLEND_ALPHA;
    s->flipmode = FLIP_NONE;
    s->method_flags = 0;
    s->blend_start = 0;
    s->blend_finish = 0;
    s->timer = 0;
    s->duration = 0;

    s->pal_begin = 0;
    s->pal_end = 0;
    s->pal_ref_index = 0;
    s->pal_start_index = 0;
    s->pal_entry_count = 0;
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
    obj->animation_state.spawn = NULL;
    obj->animation_state.spawn_userdata = NULL;
    obj->animation_state.destroy = NULL;
    obj->animation_state.destroy_userdata = NULL;
    obj->animation_state.parser = NULL;
    obj->animation_state.previous = -1;
    obj->animation_state.ticks_len = 0;
    obj->animation_state.parser = sd_stringparser_create();
    obj->slide_state.timer = 0;
    obj->slide_state.vel = vec2f_create(0,0);
    player_clear_frame(obj);
}

void player_free(object *obj) {
    if(obj->animation_state.parser != NULL) {
        sd_stringparser_delete(obj->animation_state.parser);
    }
}

void player_reload_with_str(object *obj, const char* custom_str) {
    // Load new animation string
    sd_stringparser_set_string(
        obj->animation_state.parser, 
        custom_str);

    // Find string length
    sd_stringparser_frame tmp;
    obj->animation_state.ticks_len = 0;
    int frames = sd_stringparser_num_frames(obj->animation_state.parser);
    for(int i = 0; i < frames; i++) {
        sd_stringparser_peek(obj->animation_state.parser, i, &tmp);
        obj->animation_state.ticks_len += tmp.duration;
    }

    // Peek parameters
    sd_stringparser_frame param;
    sd_stringparser_peek(obj->animation_state.parser, 0, &param);
    
    // Set initial position for sprite
    if(isset(&param, "x=")) {
        obj->pos.x = get(&param, "x=");
    }
    if(isset(&param, "y=")) {
        obj->pos.y = get(&param, "y=");
    }

    // Set player state
    obj->animation_state.ticks = 1;
    obj->animation_state.finished = 0;
    obj->animation_state.previous = -1;
    obj->animation_state.reverse = 0;
}

void player_reload(object *obj) {
    player_reload_with_str(obj, str_c(&obj->cur_animation->animation_string));
}

void player_reset(object *obj) {
    obj->animation_state.ticks = 1;
    obj->animation_state.finished = 0;
    obj->animation_state.previous = -1;
    sd_stringparser_reset(obj->animation_state.parser);
}

void player_run(object *obj) {
    // Some vars for easier life
    player_animation_state *state = &obj->animation_state;
    player_sprite_state *rstate = &obj->sprite_state;
    if(state->finished) return;

    // Handle slide operation
    if(obj->slide_state.timer > 0) {
        obj->pos.x += obj->slide_state.vel.x;
        obj->pos.y += obj->slide_state.vel.y;
        obj->slide_state.timer--;
    }

    // Not sure what this does
    int run_ret;
    if(state->end_frame == UINT32_MAX) {
        run_ret = sd_stringparser_run(
            state->parser, 
            state->ticks - 1);
    } else {
        run_ret = sd_stringparser_run_frames(
            state->parser, 
            state->ticks - 1, 
            state->end_frame);
    }

    // Handle frame
    if(run_ret == 0) {
        // Handle frame switch
        sd_stringparser_frame *param = &state->parser->current_frame;
        sd_stringparser_frame n_param;
        sd_stringparser_frame *f = param;
        sd_stringparser_frame *n = &n_param;
        int real_frame = param->letter - 65;

        // Do something if animation is finished!
        if(param->is_animation_end) {
            if(state->repeat) {
                player_reset(obj);
                sd_stringparser_run(state->parser, state->ticks - 1);
                real_frame = param->letter - 65;
            } else if(obj->finish != NULL) {
                obj->cur_sprite = NULL;
                obj->finish(obj);
                return;
            } else {
                obj->cur_sprite = NULL;
                state->finished = 1;
                return;
            }
        }

        // If frame changed, do something
        if(param->id != state->previous) {
            player_clear_frame(obj);
            
            // Tick management
            if(isset(f, "d"))   {
                cmd_tickjump(obj, get(f, "d"));
                sd_stringparser_reset(state->parser);
            }
        
            // Animation management
            if(isset(f, "m") && state->spawn != NULL) {
                int mx = isset(f, "mx") ? get(f, "mx") : 0;
                int my = isset(f, "my") ? get(f, "my") : 0;
                int mg = isset(f, "mg") ? get(f, "mg") : 0;
                DEBUG("Spawning %d, with g = %d, pos = (%d,%d)", 
                    get(f, "m"), mg, mx, my);
                state->spawn(
                    obj, get(f, "m"), 
                    vec2i_create(mx, my), mg, 
                    state->spawn_userdata);
            }
            if(isset(f, "md") && state->destroy != NULL) { 
                state->destroy(obj, get(f, "md"), state->destroy_userdata);
            }

            // Music playback
            if(isset(f, "smo")) { 
                cmd_music_on(get(f, "smo"));
            }
            if(isset(f, "smf")) { 
                cmd_music_off();
            }

            // Sound playback
            if(isset(f, "s")) {
                float pitch = PITCH_DEFAULT;
                float volume = VOLUME_DEFAULT * (settings_get()->sound.sound_vol/10.0f);
                float panning = PANNING_DEFAULT;
                if(isset(f, "sf")) {
                    int p = clamp(get(f, "sf"), -16, 239);
                    pitch = clampf((p/239.0f)*3.0f + 1.0f, PITCH_MIN, PITCH_MAX);
                }
                if(isset(f, "l")) {
                    int v = clamp(get(f, "l"), 0, 100);
                    volume = (v / 100.0f) * (settings_get()->sound.sound_vol/10.0f);
                }
                if(isset(f, "sb")) {
                    panning = clamp(get(f, "sb"), -100, 100) / 100.0f;
                }
                int sound_id = obj->sound_translation_table[get(f, "s")] - 1;
                sound_play(sound_id, volume, panning, pitch);
            }

            // Blend mode stuff
            if(isset(f, "b1")) { rstate->method_flags &= 0x2000; }
            if(isset(f, "b2")) { rstate->method_flags &= 0x4000; }
            if(isset(f, "bb")) { 
                rstate->method_flags &= 0x0010; 
                rstate->blend_finish = get(f, "bb"); 
            }
            if(isset(f, "be")) { rstate->method_flags &= 0x0800; }
            if(isset(f, "bf")) { 
                rstate->method_flags &= 0x0001; 
                rstate->blend_finish = get(f, "bf"); 
            }
            if(isset(f, "bh")) { rstate->method_flags &= 0x0040; }
            if(isset(f, "bl")) { 
                rstate->method_flags &= 0x0008; 
                rstate->blend_finish = get(f, "bl"); 
            }
            if(isset(f, "bm")) { 
                rstate->method_flags &= 0x0100; 
                rstate->blend_finish = get(f, "bm"); 
            }
            if(isset(f, "bj")) { 
                rstate->method_flags &= 0x0400; 
                rstate->blend_finish = get(f, "bj"); 
            }
            if(isset(f, "bs")) { 
                rstate->blend_start = get(f, "bs"); 
            }
            if(isset(f, "bu")) { rstate->method_flags &= 0x8000; }
            if(isset(f, "bw")) { rstate->method_flags &= 0x0080; }
            if(isset(f, "bx")) { rstate->method_flags &= 0x0002; }

            // Palette tricks
            if(isset(f, "bpd")) { rstate->pal_ref_index = get(f, "bpd"); }
            if(isset(f, "bpn")) { rstate->pal_entry_count = get(f, "bpn"); }
            if(isset(f, "bps")) { rstate->pal_start_index = get(f, "bps"); }
            if(isset(f, "bpb")) { rstate->pal_begin = get(f, "bpb"); }
            if(isset(f, "bpd")) { rstate->pal_end = get(f, "bpd"); }
            if(isset(f, "bz"))  { rstate->pal_tint = get(f, "bz"); }

            // Handle movement
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
                    obj->vel.x += x;
                    obj->vel.y += y;
                }
            }
            // handle scaling on the Y axis
            if(isset(f, "y")) { 
                obj->y_percent = get(f, "y") / 100.0f; 
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
                    x = get(f, "x-") * -1 * object_get_direction(obj);
                } else if(isset(f, "x+")) {
                    x = get(f, "x+") * object_get_direction(obj);
                }

                float x_dist = dist(obj->pos.x, state->enemy_x + x);
                float y_dist = dist(obj->pos.y, state->enemy_y + y);
                obj->slide_state.timer = param->duration;
                obj->slide_state.vel.x = x_dist / (float)param->duration;
                obj->slide_state.vel.y = y_dist / (float)param->duration;
                DEBUG("Slide object %d for (x,y) = (%f,%f) for %d ticks.", 
                    obj->cur_animation->id,
                    obj->slide_state.vel.x, 
                    obj->slide_state.vel.y, 
                    param->duration);
            }
            if (isset(f, "v") == 0 && 
                isset(f, "e") == 0 && 
                (isset(f, "x+") || isset(f, "y+") || isset(f, "x-") || isset(f, "y-"))) {
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

                obj->slide_state.timer = param->duration;
                obj->slide_state.vel.x = (float)x / (float)param->duration;
                obj->slide_state.vel.y = (float)y / (float)param->duration;
                DEBUG("Slide object %d for (x,y) = (%f,%f) for %d ticks.",
                    obj->cur_animation->id,
                    obj->slide_state.vel.x, 
                    obj->slide_state.vel.y, 
                    param->duration);
            }

            if (isset(f, "k")) {
                // knockback. this might possibly take a number, but none of the footer strings that use it seem to pass one
                // examples are crouching roundhouse for electra, katana and chronos
                // XXX we don't need to knockback on the X axis here because the flinch code will do that
                obj->vel.y -= 7;
            }

            // Check if next frame contains X=nnn or Y=nnn 
            if(!param->is_final_frame) {
                sd_stringparser_peek(state->parser, param->id + 1, &n_param);
                int slide = 0;
                float xpos = obj->pos.x;
                float ypos = obj->pos.y;
                if(isset(n, "x=") || isset(n, "y=")) {
                    obj->slide_state.vel = vec2f_create(0,0);
                }
                if(isset(n, "x=")) {
                    slide = get(n, "x=");
                    if(object_get_direction(obj) == OBJECT_FACE_LEFT) {
                        // if the sprite is flipped horizontally, adjust the X coordinates
                        slide = 320 - slide;
                    }
                    if(slide != xpos) {
                        obj->slide_state.vel.x = dist(xpos, slide) / (float)param->duration;
                        obj->slide_state.timer = param->duration;
                        DEBUG("Slide object %d for X = %f for a total of %d ticks.", 
                            obj->cur_animation->id,
                            obj->slide_state.vel.x, 
                            param->duration);
                    }
                }
                if(isset(n, "y=")) { 
                    slide = get(n, "y=");
                    if(slide != ypos) {
                        obj->slide_state.vel.y = dist(ypos, slide) / (float)param->duration;
                        obj->slide_state.timer = param->duration;
                        DEBUG("Slide object %d for Y = %f for a total of %d ticks.", 
                            obj->cur_animation->id,
                            obj->slide_state.vel.y, 
                            param->duration);
                    }
                }
            }
            
            // Set render settings
            if(real_frame < 25) {
                object_select_sprite(obj, real_frame);
                if(obj->cur_sprite != NULL) {
                    rstate->duration = param->duration;
                    rstate->blendmode = isset(f, "br") ? BLEND_ADDITIVE : BLEND_ALPHA;
                    if(isset(f, "r")) {
                        rstate->flipmode ^= FLIP_HORIZONTAL;
                    }
                    if(isset(f, "f")) {
                        rstate->flipmode ^= FLIP_VERTICAL;
                    }
                }
            } else {
                obj->cur_sprite = NULL;
            }

        }
        state->previous = param->id;
    }

    // Animation ticks
    if(state->reverse) {
        state->ticks--;
    } else {
        state->ticks++;
    }

    // Sprite ticks
    rstate->timer++;
    
    // All done.
    return;
}

void player_jump_to_tick(object *obj, int tick) {
    sd_stringparser_goto_tick(obj->animation_state.parser, tick);
    obj->animation_state.ticks = tick;
}

unsigned int player_get_len_ticks(object *obj) {
    return obj->animation_state.ticks_len;
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
