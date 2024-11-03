#include <assert.h>
#include <math.h>

#include "audio/audio.h"
#include "formats/error.h"
#include "formats/script.h"
#include "game/game_player.h"
#include "game/game_state.h"
#include "game/protos/object.h"
#include "game/protos/player.h"
#include "game/utils/settings.h"
#include "resources/ids.h"
#include "utils/log.h"
#include "utils/miscmath.h"
#include "utils/random.h"
#include "utils/str.h"
#include "utils/vec.h"
#include "video/video.h"

#define COLOR_6TO8(color) ((color << 2) | ((color & 0x30) >> 4))

static void player_clear_frame(object *obj) {
    player_sprite_state *s = &obj->sprite_state;
    memset(s, 0, sizeof(player_sprite_state));
    s->flipmode = FLIP_NONE;
    s->dir_correction = 1;
    s->blend_start = 0xFF;
    s->blend_finish = 0xFF;
}

void player_create(object *obj) {
    memset(&obj->animation_state, 0, sizeof(player_animation_state));
    obj->animation_state.end_frame = UINT32_MAX;
    obj->animation_state.previous_tick = -1;
    sd_script_create(&obj->animation_state.parser);
    player_clear_frame(obj);
}

void player_clone(object *src, object *dst) {
    sd_script_clone(&src->animation_state.parser, &dst->animation_state.parser);
}

void player_free(object *obj) {
    sd_script_free(&obj->animation_state.parser);
}

void player_reload_with_str(object *obj, const char *custom_str) {
    // Free and reload parser
    sd_script_free(&obj->animation_state.parser);
    sd_script_create(&obj->animation_state.parser);
    int ret;
    int err_pos;
    ret = sd_script_decode(&obj->animation_state.parser, custom_str, &err_pos);
    if(ret != SD_SUCCESS) {
        PERROR("Decoder error %s at position %d in string \"%s\"", sd_get_error(ret), err_pos, custom_str);
    }

    // Set player state
    player_reset(obj);
    obj->animation_state.reverse = 0;
    obj->slide_state.timer = 0;
    obj->slide_state.vel = vec2f_create(0, 0);
    obj->enemy_slide_state.timer = 0;
    obj->enemy_slide_state.dest = vec2i_create(0, 0);
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
    obj->animation_state.disable_d = 0;
}

int player_frame_isset(const object *obj, const char *tag) {
    const sd_script_frame *frame =
        sd_script_get_frame_at(&obj->animation_state.parser, obj->animation_state.current_tick);
    return sd_script_isset(frame, tag);
}

int player_frame_get(const object *obj, const char *tag) {
    const sd_script_frame *frame =
        sd_script_get_frame_at(&obj->animation_state.parser, obj->animation_state.current_tick);
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
    }
}

#ifdef DEBUGMODE
void player_describe_frame(const sd_script_frame *frame) {
    DEBUG("Frame %c%d", 65 + frame->sprite, frame->tick_len);
    for(unsigned i = 0; i < vector_size(&frame->tags); i++) {
        sd_script_tag *tag = vector_get(&frame->tags, i);
        if(tag->has_param) {
            DEBUG("    %3s%5d   %s", tag->key, tag->value, tag->desc);
        } else {
            DEBUG("    %3s        %s", tag->key, tag->desc);
        }
    }
}

void player_describe_object(object *obj) {
    DEBUG("Object:");
    DEBUG("  - Start: %d, %d", obj->start.x, obj->start.y);
    DEBUG("  - Position: %d, %d", obj->pos.x, obj->pos.y);
    DEBUG("  - Velocity: %d, %d", obj->vel.x, obj->vel.y);
    if(obj->cur_sprite_id) {
        sprite *cur_sprite = animation_get_sprite(obj->cur_animation, obj->cur_sprite_id);
        DEBUG("  - Pos: %d, %d", cur_sprite->pos.x, cur_sprite->pos.y);
        DEBUG("  - Size: %d, %d", cur_sprite->data->w, cur_sprite->data->h);
        player_sprite_state *rstate = &obj->sprite_state;
        DEBUG("CURRENT = %d - %d + %d - %d", obj->pos.y, cur_sprite->pos.y, rstate->o_correction.y,
              cur_sprite->data->h);
    }
}

void player_describe_mp_flags(const sd_script_frame *frame, int mp) {
    if(mp != 0) {
        DEBUG("mp flags set for new animation %d:", sd_script_get(frame, "m"));
        if(mp & 0x1)
            DEBUG(" * 0x01: NON-HAR Sprite");
        if(mp & 0x2)
            DEBUG(" * 0x02: Unknown");
        if(mp & 0x4)
            DEBUG(" * 0x04: HAR 1 related");
        if(mp & 0x8)
            DEBUG(" * 0x08: Something timer related is skipped ?");
        if(mp & 0x10)
            DEBUG(" * 0x10: HAR 2 related");
        if(mp & 0x20)
            DEBUG(" * 0x20: Flip x operations");
        if(mp & 0x40)
            DEBUG(" * 0x40: Something about wall collisions ?");
        if(mp & 0x80)
            DEBUG(" * 0x80: Sprite timer related ?");
    }
}
#endif /* DEBUGMODE */

void player_run(object *obj) {
    // Some vars for easier life
    player_animation_state *state = &obj->animation_state;
    player_sprite_state *rstate = &obj->sprite_state;
    object *enemy = game_state_find_object(obj->gs, state->enemy_obj_id);
    // DEBUG("i am %d, enemy is %d", obj->id, state->enemy_obj_id);
    if(state->finished)
        return;

    const sd_script_frame *frame = sd_script_get_frame_at(&state->parser, state->current_tick);

    // Animation has ended ?
    if(frame == NULL) {
        if(state->repeat) {
            player_reset(obj);
            frame = sd_script_get_frame_at(&state->parser, state->current_tick);
        } else if(obj->finish != NULL) {
            obj->cur_sprite_id = -1;
            state->finished = 1;
            obj->finish(obj);
            return;
        } else {
            obj->cur_sprite_id = -1;
            state->finished = 1;
            return;
        }
    }

    // This should really never happen, but just make sure anyway.
    assert(frame != NULL);

    // Get MP flag content, set to 0 if not set.
    uint8_t mp = sd_script_isset(frame, "mp") ? sd_script_get(frame, "mp") & 0xFF : 0;

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

    // Check if frame changed from the previous tick
    state->entered_frame = sd_script_frame_changed(&state->parser, state->previous_tick, state->current_tick);
    if(state->entered_frame) {
#ifdef DEBUGMODE
        // player_describe_frame(frame);
        // player_describe_object(obj);
        // player_describe_mp_flags(frame, mp);
#endif
        player_clear_frame(obj);

        if(sd_script_isset(frame, "ar")) {
            rstate->dir_correction = -1;
        }

        if(sd_script_isset(frame, "cf")) {
            // shadow's scrap, position is in the corner behind shadow
            if(object_get_direction(obj) == OBJECT_FACE_RIGHT) {
                obj->pos.x = 0;
            } else {
                obj->pos.x = 320;
            }
            // flip the HAR's position for this animation
            obj->animation_state.shadow_corner_hack = 1;
        }

        if(sd_script_isset(frame, "ac")) {
            // force the har to face the center of the arena
            if(obj->pos.x > 160) {
                object_set_direction(obj, OBJECT_FACE_LEFT);
            } else {
                object_set_direction(obj, OBJECT_FACE_RIGHT);
            }
        }

        /*if (sd_script_isset(frame, "bm")) {
            if (sd_script_isset(frame, "am") && sd_script_isset(frame, "e")) {
                // destination is the enemy's position
                DEBUG("BE tag with x/y offsets: %d %d %d %d", trans_x, trans_y, object_get_direction(obj),
        object_get_direction(state->enemy)); DEBUG("enemy x %d modified trans_x: %d (%d * %d * %d)",
                    state->enemy->pos.x,
                    (trans_x * object_get_direction(obj) * object_get_direction(state->enemy)),
                    trans_x,
                    object_get_direction(obj),
                    object_get_direction(state->enemy));
                // hack because we don't have 'walk to other HAR' implemented
                obj->pos.x = state->enemy->pos.x + (trans_x * object_get_direction(obj) *
        object_get_direction(state->enemy)); obj->pos.y = state->enemy->pos.y + trans_y; } else if
        (sd_script_isset(frame, "cf")) {
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
        }*/
    }

    if(sd_script_isset(frame, "e") && enemy) {

        DEBUG("my position %f, %f, their position %f %f", obj->pos.x, obj->pos.y, enemy->pos.x, enemy->pos.y);
        // Set speed to 0, since we're being controlled by animation tag system
        obj->vel.x = 0;
        obj->vel.y = 0;

        // Reset position to enemy coordinates and make sure facing is set correctly
        obj->pos.x = enemy->pos.x;
        obj->pos.y = enemy->pos.y;
        object_set_direction(obj, object_get_direction(enemy) * -1);
        // DEBUG("E: pos.x = %f, pos.y = %f", obj->pos.x, obj->pos.y);
    }

    // Set to ground
    if(sd_script_isset(frame, "g")) {
        obj->vel.y = 0;
        obj->pos.y = ARENA_FLOOR;
    }

    if(sd_script_isset(frame, "h")) {
        // Hover, reset all velocities to 0 on every frame
        obj->vel.x = 0;
        obj->vel.y = 0;
    }

    if(sd_script_isset(frame, "at") && enemy) {

        DEBUG("my position %f, %f, their position %f %f", obj->pos.x, obj->pos.y, enemy->pos.x, enemy->pos.y);
        // set the object's X position to be behind the opponent

        if(obj->pos.x > enemy->pos.x) { // From right to left
            obj->pos.x = enemy->pos.x - object_get_size(obj).x / 2;
        } else { // From left to right
            obj->pos.x = enemy->pos.x + object_get_size(enemy).x / 2;
        }
        object_set_direction(obj, object_get_direction(obj) * -1);
    }

    // Handle vx+/-, vy+/-, x+/-. y+/-
    if(trans_x || trans_y) {
        if(sd_script_isset(frame, "v")) {
            obj->vel.x = (trans_x * (mp & 0x20 ? -1 : 1)) * obj->horizontal_velocity_modifier;
            obj->vel.y = trans_y * obj->vertical_velocity_modifier;
            // DEBUG("vel x+%d, y+%d to x=%f, y=%f", trans_x * (mp & 0x20 ? -1 : 1), trans_y, obj->vel.x, obj->vel.y);
        } else {
            obj->pos.x += trans_x * (mp & 0x20 ? -1 : 1);
            obj->pos.y += trans_y;
            // DEBUG("pos x+%d, y+%d to x=%f, y=%f", trans_x * (mp & 0x20 ? -1 : 1), trans_y, obj->pos.x, obj->pos.y);
        }
    }

    // Handle slide operations on self
    if(obj->slide_state.timer > 0) {
        obj->pos.x += obj->slide_state.vel.x;
        obj->pos.y += obj->slide_state.vel.y;
        obj->slide_state.timer--;
    }

    // Handle slide in relation to enemy
    if(obj->enemy_slide_state.timer > 0 && enemy) {

        DEBUG("my position %f, %f, their position %f %f", obj->pos.x, obj->pos.y, enemy->pos.x, enemy->pos.y);
        obj->enemy_slide_state.duration++;
        obj->pos.x = enemy->pos.x + obj->enemy_slide_state.dest.x;
        obj->pos.y = enemy->pos.y + obj->enemy_slide_state.dest.y;
        obj->enemy_slide_state.timer--;
    }

    // If frame changed, do something
    if(state->entered_frame) {
        // Animation creation command
        if(sd_script_isset(frame, "m") && state->spawn != NULL) {
            int mx = 0;
            int my = 0;
            float vx = 0;
            float vy = 0;

            if(obj->animation_state.shadow_corner_hack && sd_script_get(frame, "m") == 65 && enemy) {

                DEBUG("my position %f, %f, their position %f %f", obj->pos.x, obj->pos.y, enemy->pos.x, enemy->pos.y);
                mx = enemy->pos.x;
                my = enemy->pos.y;
            }

            // Staring X coordinate for new animation
            if(sd_script_isset(frame, "mrx")) {
                int mrx = sd_script_get(frame, "mrx");
                int mm = sd_script_isset(frame, "mm") ? sd_script_get(frame, "mm") : mrx;
                mx = random_int(&obj->rand_state, 320 - 2 * mm) + mrx;
                DEBUG("randomized mx as %d", mx);
            } else if(sd_script_isset(frame, "mx")) {
                mx = obj->start.x + (sd_script_get(frame, "mx") * object_get_direction(obj));
            }

            // Staring Y coordinate for new animation
            if(sd_script_isset(frame, "mry")) {
                int mry = sd_script_get(frame, "mry");
                int mm = sd_script_isset(frame, "mm") ? sd_script_get(frame, "mm") : mry;
                my = random_int(&obj->rand_state, 320 - 2 * mm) + mry;
                DEBUG("randomized my as %d", my);
            } else if(sd_script_isset(frame, "my")) {
                my = obj->start.y + sd_script_get(frame, "my");
            }

            // Angle/speed for new animation
            if(sd_script_isset(frame, "ma")) {
                int ma = sd_script_get(frame, "ma");
                vx = cosf(ma);
                vy = sinf(ma);
                DEBUG("MA is set! angle = %d, vx = %f, vy = %f", ma, vx, vy);
            }

            // Special positioning for certain desert arena sprites
            int ms = sd_script_isset(frame, "ms");

            // Gravity for new object
            int mg = sd_script_isset(frame, "mg") ? sd_script_get(frame, "mg") : 0;

            state->spawn(obj, sd_script_get(frame, "m"), vec2i_create(mx, my), vec2f_create(vx, vy), mp, ms, mg,
                         state->spawn_userdata);
        }

        // Animation deletion
        if(sd_script_isset(frame, "md") && state->destroy != NULL) {
            state->destroy(obj, sd_script_get(frame, "md"), state->destroy_userdata);
        }

        // Music playback
        if(sd_script_isset(frame, "smo")) {
            if(sd_script_get(frame, "smo") == 0) {
                audio_stop_music();
                return;
            }
            audio_play_music(PSM_END + (sd_script_get(frame, "smo") - 1));
        }
        if(sd_script_isset(frame, "smf")) {
            audio_stop_music();
        }

        // Sound playback
        if(sd_script_isset(frame, "s")) {
            float pitch = PITCH_DEFAULT;
            float volume = VOLUME_DEFAULT * (settings_get()->sound.sound_vol / 10.0f);
            float panning = PANNING_DEFAULT;
            if(sd_script_isset(frame, "sf")) {
                int p = clamp(sd_script_get(frame, "sf"), -16, 239);
                pitch = clampf((p / 239.0f) * 3.0f + 1.0f, PITCH_MIN, PITCH_MAX);
            }
            if(sd_script_isset(frame, "l")) {
                int v = clamp(sd_script_get(frame, "l"), 0, 100);
                volume = (v / 100.0f) * (settings_get()->sound.sound_vol / 10.0f);
            }
            if(sd_script_isset(frame, "sb")) {
                panning = clamp(sd_script_get(frame, "sb"), -100, 100) / 100.0f;
            }
            if(obj->sound_translation_table) {
                int sound_id = obj->sound_translation_table[sd_script_get(frame, "s")] - 1;
                audio_play_sound(sound_id, volume, panning, pitch);
            }
        }

        // Blend mode stuff
        if(sd_script_isset(frame, "bb")) {
            rstate->blend_finish = sd_script_get(frame, "bb");
            rstate->screen_shake_vertical = sd_script_get(frame, "bb");
        }
        if(sd_script_isset(frame, "bf")) {
            rstate->blend_finish = sd_script_get(frame, "bf");
        }
        if(sd_script_isset(frame, "bl")) {
            rstate->blend_finish = sd_script_get(frame, "bl");
            rstate->screen_shake_horizontal = sd_script_get(frame, "bl");
        }
        if(sd_script_isset(frame, "bm")) {
            rstate->blend_finish = sd_script_get(frame, "bm");
        }
        if(sd_script_isset(frame, "bj")) {
            rstate->blend_finish = sd_script_get(frame, "bj");
        }
        if(sd_script_isset(frame, "bs")) {
            rstate->blend_start = sd_script_get(frame, "bs");
        }

        // Palette tricks
        if(sd_script_isset(frame, "bpd")) {
            rstate->pal_ref_index = sd_script_get(frame, "bpd");
        }
        if(sd_script_isset(frame, "bpn")) {
            rstate->pal_entry_count = sd_script_get(frame, "bpn");
        }
        if(sd_script_isset(frame, "bps")) {
            rstate->pal_start_index = sd_script_get(frame, "bps");
        }
        if(sd_script_isset(frame, "bpf")) {
            // Exact values come from master.dat
            if(game_state_get_player(obj->gs, 0)->har_obj_id == obj->id) {
                rstate->pal_start_index = 1;
                rstate->pal_entry_count = 47;
            } else {
                rstate->pal_start_index = 48;
                rstate->pal_entry_count = 48;
            }
        }
        if(sd_script_isset(frame, "bpp")) {
            rstate->pal_end = COLOR_6TO8(sd_script_get(frame, "bpp"));
            rstate->pal_begin = COLOR_6TO8(sd_script_get(frame, "bpp"));
        }
        if(sd_script_isset(frame, "bpb")) {
            rstate->pal_begin = COLOR_6TO8(sd_script_get(frame, "bpb"));
        }
        if(sd_script_isset(frame, "bz")) {
            rstate->pal_tint = 1;
        }

        // CREDITS palette copy tricks
        rstate->pal_tricks_off = sd_script_isset(frame, "bpo") ? 1 : 0; // Disable the standard palette tricks
        rstate->bd_flag =
            sd_script_isset(frame, "bd"); // Read palette from the last frame of animation (we emulate this internally)

        // These are animation-global instead of per-frame.
        if(sd_script_isset(frame, "ba")) {
            state->pal_copy_count = sd_script_get(frame, "ba");   // Number of copies to make after bi + bc
            state->pal_copy_start = sd_script_get(frame, "bi");   // Start offset for copying
            state->pal_copy_entries = sd_script_get(frame, "bc"); // Number of indexes to copy
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

        // If UA is set, force other HAR to damage animation
        if(sd_script_isset(frame, "ua") && enemy && enemy->cur_animation->id != 9) {

            DEBUG("my position %f, %f, their position %f %f", obj->pos.x, obj->pos.y, enemy->pos.x, enemy->pos.y);
            har_set_ani(enemy, 9, 0);
        }

        // XXX BJ tag invalidates frame, and probably doesn't do what it's supposed to.
#if 0
        // BJ sets new animation for our HAR
        if(sd_script_isset(frame, "bj")) {
            int new_ani = sd_script_get(frame, "bj");
            har_set_ani(obj, new_ani, 0);
        }
#endif

        if(sd_script_isset(frame, "bu") && obj->vel.y < 0.0f) {
            float x_dist = dist(obj->pos.x, 160);
            // assume that bu is used in conjunction with 'vy-X' and that we want to land in the center of the arena
            obj->slide_state.vel.x = x_dist / (obj->vel.y * -2);
            obj->slide_state.timer = obj->vel.y * -2;
        }

        // handle scaling on the Y axis
        if(sd_script_isset(frame, "y")) {
            obj->y_percent = sd_script_get(frame, "y") / 100.0f;
        }

        // Handle slides
        if(sd_script_isset(frame, "x=") || sd_script_isset(frame, "y=")) {
            obj->slide_state.vel = vec2f_create(0, 0);
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

        // Set video effects now.
        int effects = EFFECT_NONE;
        if(player_frame_isset(obj, "bt"))
            effects |= EFFECT_DARK_TINT;
        if(player_frame_isset(obj, "br"))
            effects |= EFFECT_GLOW;
        if(player_frame_isset(obj, "ub"))
            effects |= EFFECT_TRAIL;
        if(player_frame_isset(obj, "bg"))
            effects |= EFFECT_ADD;
        object_set_frame_effects(obj, effects);

        // Set render settings
        object_select_sprite(obj, frame->sprite);
        if(obj->cur_sprite_id >= 0) {
            rstate->duration = frame->tick_len;
            if(sd_script_isset(frame, "r") || obj->animation_state.shadow_corner_hack) {
                rstate->flipmode ^= FLIP_HORIZONTAL;
            }
            if(sd_script_isset(frame, "f")) {
                rstate->flipmode ^= FLIP_VERTICAL;
            }
        }
    }

    // Tick management
    if(sd_script_isset(frame, "d") && !obj->animation_state.disable_d) {
        state->previous_tick = state->current_tick;
        state->current_tick = sd_script_get(frame, "d") + 1;
        return;
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
    state->current_tick = sd_script_get_tick_pos_at_frame(&state->parser, current_index + 1);
    state->previous_tick = state->current_tick - 1;
}

void player_goto_frame(object *obj, int frame_id) {
    player_animation_state *state = &obj->animation_state;
    state->current_tick = sd_script_get_tick_pos_at_frame(&state->parser, frame_id);
    state->previous_tick = state->current_tick - 1;
}

uint32_t player_get_current_tick(const object *obj) {
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
