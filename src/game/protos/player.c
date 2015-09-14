#include <inttypes.h>
#include <stdlib.h>
#include <shadowdive/script.h>

#include "game/game_state.h"
#include "game/game_player.h"
#include "game/utils/settings.h"
#include "video/video.h"
#include "audio/sink.h"
#include "audio/sound.h"
#include "audio/music.h"
#include "resources/ids.h"
#include "game/protos/player.h"
#include "game/protos/object.h"
#include "utils/str.h"
#include "utils/miscmath.h"
#include "utils/log.h"
#include "utils/random.h"
#include "utils/vec.h"

// ---------------- Private functions ----------------

void player_clear_frame(object *obj) {
    player_sprite_state *s = &obj->sprite_state;
    s->blendmode = BLEND_ALPHA;
    s->flipmode = FLIP_NONE;
    s->method_flags = 0;
    s->timer = 0;
    s->duration = 0;

    s->o_correction = vec2i_create(0,0);

    s->disable_gravity = 0;

    s->screen_shake_horizontal = 0;
    s->screen_shake_vertical = 0;

    s->blend_start = 0xFF;
    s->blend_finish = 0xFF;

    s->pal_begin = 0;
    s->pal_end = 0;
    s->pal_ref_index = 0;
    s->pal_start_index = 0;
    s->pal_entry_count = 0;
    s->pal_tint = 0;
}

// ---------------- Public functions ----------------

void player_create(object *obj) {
    obj->animation_state.reverse = 0;
    obj->animation_state.end_frame = UINT32_MAX;
    obj->animation_state.current_tick = 0;
    obj->animation_state.previous_tick = -1;
    obj->animation_state.finished = 0;
    obj->animation_state.repeat = 0;
    obj->animation_state.entered_frame = 0;
    obj->animation_state.spawn = NULL;
    obj->animation_state.spawn_userdata = NULL;
    obj->animation_state.destroy = NULL;
    obj->animation_state.destroy_userdata = NULL;
    obj->animation_state.disable_d = 0;
    obj->animation_state.enemy = NULL;
    obj->animation_state.shadow_corner_hack = 0;
    obj->slide_state.timer = 0;
    obj->slide_state.vel = vec2f_create(0,0);
    sd_script_create(&obj->animation_state.parser);
    player_clear_frame(obj);
}

void player_free(object *obj) {
    sd_script_free(&obj->animation_state.parser);
}

void player_reload_with_str(object *obj, const char* custom_str) {
    // Free and reload parser
    sd_script_free(&obj->animation_state.parser);
    sd_script_create(&obj->animation_state.parser);
    int ret;
    int err_pos;
    ret = sd_script_decode(&obj->animation_state.parser, custom_str, &err_pos);
    if(ret != SD_SUCCESS) {
        PERROR("Decoder error %s at position %d in string \"%s\"",
            sd_get_error(ret), err_pos, custom_str);
    }

    // Set player state
    player_reset(obj);
    obj->animation_state.reverse = 0;
    obj->slide_state.timer = 0;
    obj->slide_state.vel = vec2f_create(0,0);
    obj->enemy_slide_state.timer = 0;
    obj->enemy_slide_state.dest = vec2i_create(0,0);
    obj->enemy_slide_state.duration = 0;
    obj->hit_frames = 0;
    obj->can_hit = 0;
}

void player_reload(object *obj) {
    player_reload_with_str(obj, str_c(&obj->cur_animation->animation_string));
}

void player_reset(object *obj) {
    obj->animation_state.previous_tick = -1;
    obj->animation_state.current_tick = 0;
    obj->animation_state.finished = 0;
    obj->animation_state.previous = -1;
}

int player_frame_isset(const object *obj, const char *tag) {
    const sd_script_frame *frame = sd_script_get_frame_at(&obj->animation_state.parser, obj->animation_state.current_tick);
    return sd_script_isset(frame, tag);
}

int player_frame_get(const object *obj, const char *tag) {
    const sd_script_frame *frame = sd_script_get_frame_at(&obj->animation_state.parser, obj->animation_state.current_tick);
    return sd_script_get(frame, tag);
}

/*
 * Try to spread <delay> ticks over the 'startup' frames; those that don't spawn projectiles or have hit coordinates
 */
void player_set_delay(object *obj, int delay) {
    // find the first frame that spawns a projectile, if any
    int r = sd_script_next_frame_with_tag(&obj->animation_state.parser, "m", 0);
    int frames = (r >= 0) ? r : 99;

    // find the first frame with hit coordinates
    iterator it;
    collision_coord *cc;
    vector_iter_begin(&obj->cur_animation->collision_coords, &it);
    while((cc = iter_next(&it)) != NULL) {
        r = sd_script_next_frame_with_sprite(&obj->animation_state.parser, cc->frame_index, 0);
        frames = (r >= 0 && r < frames) ? r : frames;
    }

    // No frame found, just quit now.
    if(!frames) {
        return;
    }

    DEBUG("Animation has %d initializer frames", frames);

    int delay_per_frame = delay / frames;
    int rem = delay % frames;
    for(int i = 0; i < frames; i++) {
        int duration = sd_script_get_tick_len_at_frame(&obj->animation_state.parser, i);
        int old_dur = duration;
        int new_duration = duration + delay_per_frame;
        if(rem) {
            new_duration++;
            rem--;
        }

        sd_script_set_tick_len_at_frame(&obj->animation_state.parser, i, new_duration);
        duration = sd_script_get_tick_len_at_frame(&obj->animation_state.parser, i);
        DEBUG("changed duration of frame %d from %d to %d", i, old_dur, duration);
        (void)(old_dur); // Fixes compile complaints :P
    }
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

    if(obj->enemy_slide_state.timer > 0) {
        obj->enemy_slide_state.duration++;
        obj->pos.x = state->enemy->pos.x + obj->enemy_slide_state.dest.x;
        obj->pos.y = state->enemy->pos.y + obj->enemy_slide_state.dest.y;
        obj->enemy_slide_state.timer--;
    }

    // Not sure what this does
    const sd_script_frame *frame = sd_script_get_frame_at(&state->parser, state->current_tick);

    // Animation has ended ?
    if(frame == NULL) {
        if(state->repeat) {
            player_reset(obj);
            frame = sd_script_get_frame_at(&state->parser, state->current_tick);
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

    // Handle frame
    state->entered_frame = 0;
    if(frame == NULL) {
        DEBUG("Something went wery wrong!");
        // We shouldn't really get here, unless stringparser messes something up badly
    } else {
        // If frame changed, do something
        if(sd_script_frame_changed(&state->parser, state->previous_tick, state->current_tick)) {
            state->entered_frame = 1;
            player_clear_frame(obj);

            // Tick management
            if(sd_script_isset(frame, "d")) {
                if(!obj->animation_state.disable_d) {
                    state->previous_tick = sd_script_get(frame, "d")-1;
                    state->current_tick = sd_script_get(frame, "d");
                }
            }

            // Hover flag
            if(sd_script_isset(frame, "h")) {
                rstate->disable_gravity = 1;
            } else {
                rstate->disable_gravity = 0;
            }

            if(sd_script_isset(frame, "ua")) {
                obj->animation_state.enemy->sprite_state.disable_gravity = 1;
            }

            // Animation creation command
            if(sd_script_isset(frame, "m") && state->spawn != NULL) {
                int mx = 0;
                int my = 0;
                if (obj->animation_state.shadow_corner_hack && sd_script_get(frame, "m") == 65) {
                    mx = state->enemy->pos.x;
                    my = state->enemy->pos.y;
                }
                if (sd_script_isset(frame, "mrx")) {
                    int mrx = sd_script_get(frame, "mrx");
                    int mm = sd_script_isset(frame, "mm") ? sd_script_get(frame, "mm") : mrx;
                    mx = random_int(&obj->rand_state, 320 - 2*mm) + mrx;
                    DEBUG("randomized mx as %d", mx);
                } else if(sd_script_isset(frame, "mx")) {
                    mx = obj->start.x + (sd_script_get(frame, "mx") * object_get_direction(obj));
                }

                if (sd_script_isset(frame, "mry")) {
                    int mry = sd_script_get(frame, "mry");
                    int mm = sd_script_isset(frame, "mm") ? sd_script_get(frame, "mm") : mry;
                    my = random_int(&obj->rand_state, 320 - 2*mm) + mry;
                    DEBUG("randomized my as %d", my);
                } else if(sd_script_isset(frame, "my")) {
                    my = obj->start.y + sd_script_get(frame, "my");
                }

                int mg = sd_script_isset(frame, "mg") ? sd_script_get(frame, "mg") : 0;
                state->spawn(
                    obj,
                    sd_script_get(frame, "m"),
                    vec2i_create(mx, my),
                    mg,
                    state->spawn_userdata);
            }

            // Animation deletion
            if(sd_script_isset(frame, "md") && state->destroy != NULL) {
                state->destroy(obj, sd_script_get(frame, "md"), state->destroy_userdata);
            }

            // Music playback
            if(sd_script_isset(frame, "smo")) {
                if(sd_script_get(frame, "smo") == 0) {
                    music_stop();
                    return;
                }
                music_play(PSM_END + (sd_script_get(frame, "smo") - 1));
            }
            if(sd_script_isset(frame, "smf")) {
                music_stop();
            }

            // Sound playback
            if(sd_script_isset(frame, "s")) {
                float pitch = PITCH_DEFAULT;
                float volume = VOLUME_DEFAULT * (settings_get()->sound.sound_vol/10.0f);
                float panning = PANNING_DEFAULT;
                if(sd_script_isset(frame, "sf")) {
                    int p = clamp(sd_script_get(frame, "sf"), -16, 239);
                    pitch = clampf((p/239.0f)*3.0f + 1.0f, PITCH_MIN, PITCH_MAX);
                }
                if(sd_script_isset(frame, "l")) {
                    int v = clamp(sd_script_get(frame, "l"), 0, 100);
                    volume = (v / 100.0f) * (settings_get()->sound.sound_vol/10.0f);
                }
                if(sd_script_isset(frame, "sb")) {
                    panning = clamp(sd_script_get(frame, "sb"), -100, 100) / 100.0f;
                }
                int sound_id = obj->sound_translation_table[sd_script_get(frame, "s")] - 1;
                sound_play(sound_id, volume, panning, pitch);
            }

            // Blend mode stuff
            if(sd_script_isset(frame, "b1")) { rstate->method_flags &= 0x2000; }
            if(sd_script_isset(frame, "b2")) { rstate->method_flags &= 0x4000; }
            if(sd_script_isset(frame, "bb")) {
                rstate->method_flags &= 0x0010;
                rstate->blend_finish = sd_script_get(frame, "bb");
                rstate->screen_shake_vertical = sd_script_get(frame, "bb");
            }
            if(sd_script_isset(frame, "be")) { rstate->method_flags &= 0x0800; }
            if(sd_script_isset(frame, "bf")) {
                rstate->method_flags &= 0x0001;
                rstate->blend_finish = sd_script_get(frame, "bf");
            }
            if(sd_script_isset(frame, "bh")) { rstate->method_flags &= 0x0040; }
            if(sd_script_isset(frame, "bl")) {
                rstate->method_flags &= 0x0008;
                rstate->blend_finish = sd_script_get(frame, "bl");
                rstate->screen_shake_horizontal = sd_script_get(frame, "bl");
            }
            if(sd_script_isset(frame, "bm")) {
                rstate->method_flags &= 0x0100;
                rstate->blend_finish = sd_script_get(frame, "bm");
            }
            if(sd_script_isset(frame, "bj")) {
                rstate->method_flags &= 0x0400;
                rstate->blend_finish = sd_script_get(frame, "bj");
            }
            if(sd_script_isset(frame, "bs")) {
                rstate->blend_start = sd_script_get(frame, "bs");
            }
            if(sd_script_isset(frame, "bu")) { rstate->method_flags &= 0x8000; }
            if(sd_script_isset(frame, "bw")) { rstate->method_flags &= 0x0080; }
            if(sd_script_isset(frame, "bx")) { rstate->method_flags &= 0x0002; }

            // Palette tricks
            if(sd_script_isset(frame, "bpd")) { rstate->pal_ref_index = sd_script_get(frame, "bpd"); }
            if(sd_script_isset(frame, "bpn")) { rstate->pal_entry_count = sd_script_get(frame, "bpn"); }
            if(sd_script_isset(frame, "bps")) { rstate->pal_start_index = sd_script_get(frame, "bps"); }
            if(sd_script_isset(frame, "bpf")) {
                // Exact values come from master.dat
                if(game_state_get_player(obj->gs, 0)->har == obj) {
                    rstate->pal_start_index =  1;
                    rstate->pal_entry_count = 47;
                } else {
                    rstate->pal_start_index =  48;
                    rstate->pal_entry_count = 48;
                }
            }
            if(sd_script_isset(frame, "bpp")) {
                rstate->pal_end = sd_script_get(frame, "bpp") * 4;
                rstate->pal_begin = sd_script_get(frame, "bpp") * 4;
            }
            if(sd_script_isset(frame, "bpb")) { rstate->pal_begin = sd_script_get(frame, "bpb") * 4; }
            if(sd_script_isset(frame, "bz"))  { rstate->pal_tint = 1; }

            // The following is a hack. We don't REALLY know what these tags do.
            // However, they are only used in CREDITS.BK, so we can just interpret
            // then as we see fit, as long as stuff works.
            if(sd_script_isset(frame, "bc") && frame->tick_len >= 50) {
                rstate->blend_start = 0;
            } else if(sd_script_isset(frame, "bd") && frame->tick_len >= 30) {
                rstate->blend_finish = 0;
            }

            // Handle position correction
            if(sd_script_isset(frame, "ox")) {
                DEBUG("O_CORRECTION: X = %d", sd_script_get(frame, "ox"));
                rstate->o_correction.x = sd_script_get(frame, "ox");
            } else {
                rstate->o_correction.x = 0;
            }
            if(sd_script_isset(frame, "oy")) {
                DEBUG("O_CORRECTION: Y = %d", sd_script_get(frame, "oy"));
                rstate->o_correction.y = sd_script_get(frame, "oy");
            } else {
                rstate->o_correction.y = 0;
            }

            if(sd_script_isset(frame, "ar")) {
                DEBUG("flipping direction %d -> %d", object_get_direction(obj), object_get_direction(obj) *-1);
                // reverse direction
                object_set_direction(obj, object_get_direction(obj) * -1);
                DEBUG("flipping direction now %d", object_get_direction(obj));
            }

            // See if x+/- or y+/- are set and save values
            int trans_x = 0, trans_y = 0;
            if(sd_script_isset(frame, "y-")) {
                trans_y = sd_script_get(frame, "y-") * -1;
            } else if(sd_script_isset(frame, "y+")) {
                trans_y = sd_script_get(frame, "y+");
            }
            if(sd_script_isset(frame, "x-")) {
                trans_x = sd_script_get(frame, "x-") * -1 * object_get_direction(obj);
            } else if(sd_script_isset(frame, "x+")) {
                trans_x = sd_script_get(frame, "x+") * object_get_direction(obj);
            }

            if (sd_script_isset(frame, "bm")) {
                if (sd_script_isset(frame, "am") && sd_script_isset(frame, "e")) {
                    // destination is the enemy's position
                    DEBUG("BE tag with x/y offsets: %d %d %d %d", trans_x, trans_y, object_get_direction(obj), object_get_direction(state->enemy));
                    DEBUG("enemy x %d modified trans_x: %d (%d * %d *%d)", state->enemy->pos.x, (trans_x * object_get_direction(obj) * object_get_direction(state->enemy)), object_get_direction(obj), object_get_direction(state->enemy));
                    // hack because we don't have 'walk to other HAR' implemented
                    obj->pos.x = state->enemy->pos.x + (trans_x * object_get_direction(obj) * object_get_direction(state->enemy));
                    obj->pos.y = state->enemy->pos.y + trans_y;
                } else if (sd_script_isset(frame, "cf")) {
                    // shadow's scrap, position is in the corner behind shadow
                    if (object_get_direction(obj) == OBJECT_FACE_RIGHT) {
                        obj->pos.x = 0;
                    } else {
                        obj->pos.x = 320;
                    }
                    // flip the HAR's position for this animation
                    obj->animation_state.shadow_corner_hack = 1;
                } else {
                    PERROR("unknown end position for BE tag");
                }
                player_next_frame(state->enemy);
            } else {
                // Handle vx+/-, ex+/-, vy+/-, ey+/-, x+/-. y+/-
                if(trans_x || trans_y) {
                    if(sd_script_isset(frame, "v")) {
                        obj->vel.x += trans_x;
                        obj->vel.y += trans_y;
                    } else {
                        if(sd_script_isset(frame, "e")) {
                            obj->enemy_slide_state.timer = frame->tick_len;
                            obj->enemy_slide_state.duration = 0;
                            obj->enemy_slide_state.dest.x = trans_x;
                            obj->enemy_slide_state.dest.y = trans_y;
                        } else {
                            obj->slide_state.timer = frame->tick_len;
                            obj->slide_state.vel.x = (float)trans_x;
                            obj->slide_state.vel.y = (float)trans_y;
                        }
                    }
                }
            }

            if (sd_script_isset(frame, "bu") && obj->vel.y < 0.0f) {
                float x_dist = dist(obj->pos.x, 160);
                // assume that bu is used in conjunction with 'vy-X' and that we want to land in the center of the arena
                obj->slide_state.vel.x = x_dist / (obj->vel.y*-2);
                obj->slide_state.timer = obj->vel.y*-2;
            }

            // handle scaling on the Y axis
            if(sd_script_isset(frame, "y")) {
                obj->y_percent = sd_script_get(frame, "y") / 100.0f;
            }

            // Handle slides
            if(sd_script_isset(frame, "x=") || sd_script_isset(frame, "y=")) {
                obj->slide_state.vel = vec2f_create(0,0);
            }
            if(sd_script_isset(frame, "x=")) {
                obj->pos.x = obj->start.x + (sd_script_get(frame, "x=") * object_get_direction(obj));

                // Find frame ID by tick
                int frame_id = sd_script_next_frame_with_tag(&state->parser, "x=", state->current_tick);

                // Handle it!
                if(frame_id >= 0) {
                    int mr = sd_script_get_tick_pos_at_frame(&state->parser, frame_id);
                    int r = mr - state->current_tick - frame->tick_len;
                    int next_x = sd_script_get(sd_script_get_frame(&state->parser, frame_id), "x=");
                    int slide = obj->start.x + (next_x * object_get_direction(obj));
                    if(slide != obj->pos.x) {
                        obj->slide_state.vel.x = dist(obj->pos.x, slide) / (float)(frame->tick_len + r);
                        obj->slide_state.timer = frame->tick_len + r;
                        /* DEBUG("Slide object %d for X = %f for a total of %d + %d = %d ticks.",
                                obj->cur_animation->id,
                                obj->slide_state.vel.x,
                                frame->tick_len,
                                r,
                                frame->tick_len + r);*/
                    }

                }
            }
            if(sd_script_isset(frame, "y=")) {
                obj->pos.y = obj->start.y + sd_script_get(frame, "y=");

                // Find frame ID by tick
                int frame_id = sd_script_next_frame_with_tag(&state->parser, "y=", state->current_tick);

                // handle it!
                if(frame_id >= 0) {
                    int mr = sd_script_get_tick_pos_at_frame(&state->parser, frame_id);
                    int r = mr - state->current_tick - frame->tick_len;
                    int next_y = sd_script_get(sd_script_get_frame(&state->parser, frame_id), "y=");
                    int slide = next_y + obj->start.y;
                    if(slide != obj->pos.y) {
                        obj->slide_state.vel.y = dist(obj->pos.y, slide) / (float)(frame->tick_len + r);
                        obj->slide_state.timer = frame->tick_len + r;
                        /* DEBUG("Slide object %d for Y = %f for a total of %d + %d = %d ticks.",
                                obj->cur_animation->id,
                                obj->slide_state.vel.y,
                                frame->tick_len,
                                r,
                                frame->tick_len + r);*/
                    }

                }
            }
            if(sd_script_isset(frame, "as")) {
                // make the object move around the screen in a circular motion until end of frame
                obj->orbit = 1;
            } else {
                obj->orbit = 0;
            }
            if(sd_script_isset(frame, "q")) {
                // Enable hit on the current and the next n-1 frames.
                obj->hit_frames = sd_script_get(frame, "q");
            }
            if(obj->hit_frames > 0) {
                obj->can_hit = 1;
                obj->hit_frames--;
            }

            if(sd_script_isset(frame, "at")) {
                // set the object's X position to be behind the opponent
                obj->pos.x = obj->animation_state.enemy->pos.x + (15 * object_get_direction(obj));
            }

            // Set render settings
            if(frame->sprite < 25) {
                object_select_sprite(obj, frame->sprite);
                if(obj->cur_sprite != NULL) {
                    rstate->duration = frame->tick_len;
                    rstate->blendmode = sd_script_isset(frame, "br") ? BLEND_ADDITIVE : BLEND_ALPHA;
                    if(sd_script_isset(frame, "r") || obj->animation_state.shadow_corner_hack) {
                        rstate->flipmode ^= FLIP_HORIZONTAL;
                    }
                    if(sd_script_isset(frame, "f")) {
                        rstate->flipmode ^= FLIP_VERTICAL;
                    }
                }
            } else {
                object_select_sprite(obj, -1);
            }

        }
    }

    // Animation ticks
    state->previous_tick = state->current_tick;
    if(state->reverse) {
        state->current_tick--;
    } else {
        state->current_tick++;
    }

    // Sprite ticks
    rstate->timer++;

    // All done.
    return;
}

void player_jump_to_tick(object *obj, int tick) {
    player_animation_state *state = &obj->animation_state;
    state->previous_tick = state->current_tick;
    state->current_tick = tick;
}

unsigned int player_get_len_ticks(const object *obj) {
    const player_animation_state *state = &obj->animation_state;
    return sd_script_get_total_ticks(&state->parser);
}

void player_set_repeat(object *obj, int repeat) {
    obj->animation_state.repeat = repeat;
}

int player_get_repeat(const object *obj) {
    return obj->animation_state.repeat;
}

void player_set_end_frame(object *obj, int end_frame) {
    obj->animation_state.end_frame = end_frame;
}

void player_next_frame(object *obj) {
    player_animation_state *state = &obj->animation_state;
    int current_index = sd_script_get_frame_index_at(&state->parser, state->current_tick);
    state->current_tick = sd_script_get_tick_pos_at_frame(&state->parser, current_index+1);
    state->previous_tick = state->current_tick-1;
}

void player_goto_frame(object *obj, int frame_id) {
    player_animation_state *state = &obj->animation_state;
    state->current_tick = sd_script_get_tick_pos_at_frame(&state->parser, frame_id);
    state->previous_tick = state->current_tick-1;
}

int player_get_current_tick(const object *obj) {
    return obj->animation_state.current_tick;
}

int player_get_frame(const object *obj) {
    const player_animation_state *state = &obj->animation_state;
    return sd_script_get_frame_index_at(&state->parser, state->current_tick);
}

char player_get_frame_letter(const object *obj) {
    return sd_script_frame_to_letter(player_get_frame(obj));
}

int player_is_last_frame(const object *obj) {
    const player_animation_state *state = &obj->animation_state;
    return sd_script_is_last_frame_at(&state->parser, state->current_tick);
}
