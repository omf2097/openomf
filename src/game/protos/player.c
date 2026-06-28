#include <assert.h>
#include <math.h>

#include "audio/audio.h"
#include "formats/script.h"
#include "formats/tag_list_helpers.h"
#include "game/audio/music_tracker.h"
#include "game/game_player.h"
#include "game/game_state.h"
#include "game/objects/projectile.h"
#include "game/protos/object.h"
#include "game/protos/player.h"
#include "resources/ids.h"
#include "resources/script_cache.h"
#include "utils/log.h"
#include "utils/miscmath.h"
#include "utils/random.h"
#include "utils/str.h"
#include "utils/vec.h"
#include "video/enums.h"

#define COLOR_6TO8(color) ((color << 2) | ((color & 0x30) >> 4))

static void player_clear_frame(object *obj) {
    player_sprite_state *s = &obj->sprite_state;
    memset(s, 0, sizeof(player_sprite_state));
    s->flipmode = FLIP_NONE;
    s->blend_start = 0xFF;
    s->blend_finish = 0xFF;
}

void player_create(object *obj) {
    memset(&obj->animation_state, 0, sizeof(player_animation_state));
    script_reader_load(&obj->animation_state.reader, NULL);
    player_clear_frame(obj);
}

void player_reload_with_str(object *obj, const char *custom_str) {
    script_reader_load(&obj->animation_state.reader, script_cache_get(custom_str));

    // Set player state
    player_reset(obj);
    obj->animation_state.reverse = 0;
    obj->slide_state.timer = 0;
    obj->slide_state.vel = vec2f_create(0, 0);
    obj->q_counter = 0;
    obj->q_val = 0;
    obj->can_hit = 0;
}

void player_reload(object *obj) {
    player_reload_with_str(obj, str_c(&obj->cur_animation->animation_string));
}

void player_reset(object *obj) {
    script_reader_reset(&obj->animation_state.reader);
    obj->animation_state.finished = false;
    obj->animation_state.disable_d = 0;
}

int player_frame_isset(const object *obj, const script_tag tag) {
    return script_reader_isset(&obj->animation_state.reader, tag);
}

int player_frame_get(const object *obj, const script_tag tag) {
    return script_reader_get(&obj->animation_state.reader, tag);
}

#ifdef DEBUGMODE
void player_describe_frame(const script_frame *frame) {
    log_debug("Frame %c%d", 65 + frame->sprite, frame->tick_len);
    for(unsigned i = 0; i < vector_size(&frame->tags); i++) {
        const script_frame_tag *tag = vector_get(&frame->tags, i);
        if(tag->has_param) {
            log_debug("    %3s%5d   %s", script_get_frame_tag_name(tag), tag->value,
                      script_get_frame_tag_description(tag));
        } else {
            log_debug("    %3s        %s", script_get_frame_tag_name(tag), script_get_frame_tag_description(tag));
        }
    }
}

void player_describe_object(object *obj) {
    log_debug("Object:");
    log_debug("  - Start: %d, %d", obj->start.x, obj->start.y);
    log_debug("  - Position: %d, %d", obj->pos.x, obj->pos.y);
    log_debug("  - Velocity: %d, %d", obj->vel.x, obj->vel.y);
    if(obj->cur_sprite_id) {
        sprite *cur_sprite = animation_get_sprite(obj->cur_animation, obj->cur_sprite_id);
        log_debug("  - Pos: %d, %d", cur_sprite->pos.x, cur_sprite->pos.y);
        log_debug("  - Size: %d, %d", cur_sprite->data->w, cur_sprite->data->h);
        player_sprite_state *rstate = &obj->sprite_state;
        log_debug("CURRENT = %d - %d + %d - %d", obj->pos.y, cur_sprite->pos.y, rstate->o_correction.y,
                  cur_sprite->data->h);
    }
}

void player_describe_mp_flags(const script_frame *frame, int mp) {
    if(mp != 0) {
        log_debug("mp flags set for new animation %d:", script_get_tag_value_by_id(frame, TAG_M));
        if(mp & 0x1) {
            log_debug(" * 0x01: NON-HAR Sprite");
        }
        if(mp & 0x2) {
            log_debug(" * 0x02: Unknown");
        }
        if(mp & 0x4) {
            log_debug(" * 0x04: HAR 1 related");
        }
        if(mp & 0x8) {
            log_debug(" * 0x08: Something timer related is skipped ?");
        }
        if(mp & 0x10) {
            log_debug(" * 0x10: HAR 2 related");
        }
        if(mp & 0x20) {
            log_debug(" * 0x20: Flip x operations");
        }
        if(mp & 0x40) {
            log_debug(" * 0x40: Something about wall collisions ?");
        }
        if(mp & 0x80) {
            log_debug(" * 0x80: Sprite timer related ?");
        }
    }
}
#endif /* DEBUGMODE */

void player_run(object *obj) {
    // Some vars for easier life
    player_animation_state *state = &obj->animation_state;
    player_sprite_state *rstate = &obj->sprite_state;
    object *enemy = game_state_find_object(obj->gs, state->enemy_obj_id);
    if(state->finished) {
        return;
    }

    const script *script = script_reader_get_script(&state->reader);
    const script_frame *frame = script_reader_frame(&state->reader);

    // Animation has ended ?
    if(frame == NULL) {
        if(state->repeat) {
            player_reset(obj);
            frame = script_reader_frame(&state->reader);
        } else {
            state->finished = true;
            if(obj->finish != NULL) {
                obj->finish(obj);
            }
            // let har_finish hold last sprite of victory animation indefinitely
            // Note that the finished flag can be modified by the obj->finish() call.
            if(state->finished) {
                obj->cur_sprite_id = -1;
            }
            return;
        }
    }

    // This should really never happen, but just make sure anyway.
    assert(frame != NULL);

    // Get MP flag content, set to 0 if not set.
    uint8_t mp = script_is_tag_set_by_id(frame, TAG_MP) ? script_get_tag_value_by_id(frame, TAG_MP) & 0xFF : 0;

    if(script_is_tag_set_by_id(frame, TAG_E) && enemy && !script_is_tag_set_by_id(frame, TAG_AM)) {

        // Set speed to 0, since we're being controlled by animation tag system
        obj->vel.x = 0;
        obj->vel.y = 0;

        // Reset position to enemy coordinates and make sure facing is set correctly
        obj->pos.x = enemy->pos.x;
        obj->pos.y = enemy->pos.y;
        object_set_direction(obj, object_get_direction(enemy) * -1);
        // log_debug("E: pos.x = %f, pos.y = %f", obj->pos.x, obj->pos.y);
    }

    if(enemy) {
        enemy->crossup_protection = script_is_tag_set_by_id(frame, TAG_AG);
    }

    if(script_is_tag_set_by_id(frame, TAG_AR)) {
        object_set_direction(obj, object_get_direction(obj) * -1);
    }

    // Reset the wall collision condition
    obj->wall_collision = false;
    // See if x+/- or y+/- are set and save values
    int trans_x = 0, trans_y = 0;
    if(script_is_tag_set_by_id(frame, TAG_Y_MINUS)) {
        trans_y = script_get_tag_value_by_id(frame, TAG_Y_MINUS) * -1;
    } else if(script_is_tag_set_by_id(frame, TAG_Y_PLUS)) {
        trans_y = script_get_tag_value_by_id(frame, TAG_Y_PLUS);
    }
    if(script_is_tag_set_by_id(frame, TAG_X_MINUS)) {
        trans_x = script_get_tag_value_by_id(frame, TAG_X_MINUS) * -1 * object_get_direction(obj);
    } else if(script_is_tag_set_by_id(frame, TAG_X_PLUS)) {
        trans_x = script_get_tag_value_by_id(frame, TAG_X_PLUS) * object_get_direction(obj);
    }

    // Check if frame changed from the previous tick
    state->entered_frame = script_reader_frame_changed(&state->reader);
    if(state->entered_frame) {
#ifdef DEBUGMODE
        // player_describe_frame(frame);
        // player_describe_object(obj);
        // player_describe_mp_flags(frame, mp);
#endif
        player_clear_frame(obj);

        if(script_is_tag_set_by_id(frame, TAG_AC)) {
            // force the har to face the center of the arena
            if(obj->pos.x > 160) {
                object_set_direction(obj, OBJECT_FACE_LEFT);
            } else {
                object_set_direction(obj, OBJECT_FACE_RIGHT);
            }
        }

        // TODO this needs to somehow be delayed for 1 tick
        if(script_is_tag_set_by_id(frame, TAG_N)) {
            obj->hit_pixels_disabled = true;
        } else {
            obj->hit_pixels_disabled = false;
        }

        // BJ sets new animation for our HAR
        // TODO this is still wrong somehow, there's some kind of conditional
        // but it fixes gargoyle's scrap looping and some other stuff
        if(script_is_tag_set_by_id(frame, TAG_BJ)) {
            int new_ani = script_get_tag_value_by_id(frame, TAG_BJ);
            har_set_ani(obj, new_ani, 0);
            if(new_ani == ANIM_STANDUP) {
                har_face_enemy(obj, enemy);
            }
            object_dynamic_tick(obj);
            return;
        }

        if(script_is_tag_set_by_id(frame, TAG_MC)) {
            // if UC is also set, when UC proceeds to the next animation, set the object gravity to the HAR's gravity
            obj->object_flags |= OBJECT_FLAGS_MC;
        }

        if(script_is_tag_set_by_id(frame, TAG_UD)) {
            // object moves to next animation when owning HAR is hit
            obj->object_flags |= OBJECT_FLAGS_NEXT_ANIM_ON_OWNER_HIT;
        }

        if(script_is_tag_set_by_id(frame, TAG_UZ)) {
            // object moves to next animation when enemy HAR is hit
            obj->object_flags |= OBJECT_FLAGS_NEXT_ANIM_ON_ENEMY_HIT;
        }

        if(script_is_tag_set_by_id(frame, TAG_CP) && obj->should_hitpause) {
            obj->should_hitpause = false;
            game_state_hit_pause(obj->gs);
        }

        if(script_is_tag_set_by_id(frame, TAG_MU)) {
            // mu tags depend on a previous mm tag, so we need to iterate all of then tags, keeping track of the last mm
            // value we spawn
            iterator it;
            script_frame_tag *tag;
            vector_iter_begin(&frame->tags, &it);
            int mm = 0;
            foreach(it, tag) {
                if(tag->key == TAG_MM) {
                    mm = tag->value;
                } else if(tag->key == TAG_MU && mm) {
                    if(obj->cur_animation->id == 9) {
                        // I know this might be hard to believe, but the mu tag applies to the ENEMY if you're in
                        // animation 9. This is so the shadow grab lockout lives as long as the grabbed har is
                        // immobilized.
                        object_disable_animation(enemy, mm, tag->value);
                    } else {
                        object_disable_animation(obj, mm, tag->value);
                    }
                }
            }
        }

        if(script_is_tag_set_by_id(frame, TAG_BM) && enemy) {
            int destination = 160;
            if(script_is_tag_set_by_id(frame, TAG_AM) && script_is_tag_set_by_id(frame, TAG_E)) {
                // destination is the enemy's position
                destination = enemy->pos.x - trans_x;
                if(obj->pos.x > enemy->pos.x) {
                    object_set_direction(obj, OBJECT_FACE_LEFT);
                } else {
                    object_set_direction(obj, OBJECT_FACE_RIGHT);
                }
                destination = max2(ARENA_LEFT_WALL, min2(ARENA_RIGHT_WALL, destination));
            } else if(script_is_tag_set_by_id(frame, TAG_CF)) {
                // shadow's scrap, position is in the corner behind shadow
                if(object_get_direction(enemy) == OBJECT_FACE_RIGHT) {
                    destination = ARENA_RIGHT_WALL;
                } else {
                    destination = ARENA_LEFT_WALL;
                }
                destination += trans_x;
                object_set_direction(obj, object_get_direction(enemy));
                // flip the HAR's position for this animation
                obj->animation_state.shadow_corner_hack = 1;
            } else {
                destination = -1;
            }
            // clear this
            trans_x = 0;
            if(script_get_tag_value_by_id(frame, TAG_BM) == 10 && destination > 0 &&
               fabsf(obj->pos.x - destination) > 5.0) {
                log_debug("HAR walk to %d from %d", destination, obj->pos.x);
                har_walk_to(obj, destination);
                return;
            }
        }
    }

    if(script_is_tag_set_by_id(frame, TAG_H)) {
        // Hover, reset all velocities to 0 on every frame
        obj->vel.x = 0;
        obj->vel.y = 0;
        obj->cvel.x = 0;
        obj->cvel.y = 0;
    }

    int ab_flag = script_is_tag_set_by_id(frame, TAG_AB); // Pass through walls

    // Set to ground
    if(script_is_tag_set_by_id(frame, TAG_G)) {
        obj->vel.y = 0;
        obj->pos.y = ARENA_FLOOR;
    }

    if(script_is_tag_set_by_id(frame, TAG_AD)) {
        har *har = object_get_userdata(obj);
        int new_facing = obj->direction;
        switch(har->inputs[0]) {
            case '4':
            case '7':
            case '1':
                new_facing = OBJECT_FACE_LEFT;
                break;
            case '6':
            case '9':
            case '3':
                new_facing = OBJECT_FACE_RIGHT;
                break;
            default:
                break;
        }
        if(obj->direction != new_facing) {
            object_set_direction(obj, new_facing);
            obj->vel.x *= -1;
            trans_x *= -1;
        }
    }

    if(script_is_tag_set_by_id(frame, TAG_AT) && enemy) {
        har *har = object_get_userdata(obj);
        switch(har->inputs[0]) {
            case '6':
                obj->pos.x = ARENA_RIGHT_WALL - random_int(&obj->gs->rand, 30);
                break;
            case '4':
                obj->pos.x = ARENA_LEFT_WALL + random_int(&obj->gs->rand, 30);
                break;
            default:
                if(obj->pos.x > enemy->pos.x) { // From right to left
                    obj->pos.x = enemy->pos.x - 40;
                } else { // From left to right
                    obj->pos.x = enemy->pos.x + 40;
                }
        }
    }

    // Handle vx+/-, vy+/-, x+/-. y+/-
    if(trans_x || trans_y) {
        if(script_is_tag_set_by_id(frame, TAG_V)) {
            obj->vel.x = (trans_x * (mp & 0x20 ? -1 : 1)) * obj->horizontal_velocity_modifier;
            obj->vel.y = trans_y * obj->horizontal_velocity_modifier;
            // log_debug("vel x+%d, y+%d to x=%f, y=%f", trans_x * (mp & 0x20 ? -1 : 1), trans_y, obj->vel.x,
            // obj->vel.y);
        } else {
            obj->pos.x += trans_x * (mp & 0x20 ? -1 : 1);
            if(!ab_flag) {
                if(obj->pos.x < ARENA_LEFT_WALL && obj->group == GROUP_HAR) {
                    if(script_is_tag_set_by_id(frame, TAG_E) && enemy) {
                        enemy->pos.x += ARENA_LEFT_WALL - obj->pos.x;
                    }
                    obj->pos.x = ARENA_LEFT_WALL;
                    obj->wall_collision = true;
                } else if(obj->pos.x > ARENA_RIGHT_WALL && obj->group == GROUP_HAR) {
                    if(script_is_tag_set_by_id(frame, TAG_E) && enemy) {
                        enemy->pos.x -= obj->pos.x - ARENA_RIGHT_WALL;
                    }
                    obj->pos.x = ARENA_RIGHT_WALL;
                    obj->wall_collision = true;
                }
            }
            obj->pos.y += trans_y;
            // log_debug("pos x+%d, y+%d to x=%f, y=%f", trans_x * (mp & 0x20 ? -1 : 1), trans_y, obj->pos.x,
            // obj->pos.y);
        }
    }

    // Handle slide operations on self
    if(obj->slide_state.timer > 0) {
        obj->pos.x += obj->slide_state.vel.x;
        obj->pos.y += obj->slide_state.vel.y;
        obj->slide_state.timer--;
    }

    if(obj->group == GROUP_HAR && enemy && !ab_flag) {
        obj->pos.x = clampf(obj->pos.x, ARENA_LEFT_WALL, ARENA_RIGHT_WALL);
    }

    // If frame changed, do something
    if(state->entered_frame) {
        // Animation creation command
        if(script_is_tag_set_by_id(frame, TAG_M) && state->spawn != NULL) {
            int mx = 0;
            int my = 0;
            float vx = 0;
            float vy = 0;

            // Instance count
            int instances = 1;
            if(script_is_tag_set_by_id(frame, TAG_MI)) {
                instances = script_get_tag_value_by_id(frame, TAG_MI);
                log_debug("spawning %d instances", instances);
            }

            // Staring X coordinate for new animation
            if(script_is_tag_set_by_id(frame, TAG_MX)) {
                mx = obj->start.x + (script_get_tag_value_by_id(frame, TAG_MX) * object_get_direction(obj));
            }

            // Staring Y coordinate for new animation
            if(script_is_tag_set_by_id(frame, TAG_MY)) {
                my = obj->start.y + script_get_tag_value_by_id(frame, TAG_MY);
            }

            // Angle/speed for new animation
            if(script_is_tag_set_by_id(frame, TAG_MA)) {
                int ma = script_get_tag_value_by_id(frame, TAG_MA);
                vx = cosf(ma);
                vy = sinf(ma);
                log_debug("MA is set! angle = %d, vx = %f, vy = %f", ma, vx, vy);
            }

            // Special positioning for certain desert arena sprites
            int ms = script_is_tag_set_by_id(frame, TAG_MS);

            // Gravity for new object
            int mg = script_is_tag_set_by_id(frame, TAG_MG) ? script_get_tag_value_by_id(frame, TAG_MG) : 0;

            for(int i = 0; i < instances; i++) {
                // random starting coordinates
                if(script_is_tag_set_by_id(frame, TAG_MRX)) {
                    int mrx = script_get_tag_value_by_id(frame, TAG_MRX);
                    int mm = script_is_tag_set_by_id(frame, TAG_MM) ? script_get_tag_value_by_id(frame, TAG_MM) : mrx;
                    mx = random_int(&obj->gs->rand, 320 - 2 * mm) + mrx;
                    log_debug("randomized mx as %d", mx);
                }
                if(script_is_tag_set_by_id(frame, TAG_MRY)) {
                    int mry = script_get_tag_value_by_id(frame, TAG_MRY);
                    int mm = script_is_tag_set_by_id(frame, TAG_MM) ? script_get_tag_value_by_id(frame, TAG_MM) : mry;
                    my = random_int(&obj->gs->rand, 320 - 2 * mm) + mry;
                    log_debug("randomized my as %d", my);
                }

                state->spawn(obj, script_get_tag_value_by_id(frame, TAG_M), vec2i_create(mx, my), vec2f_create(vx, vy),
                             mp, ms, mg, state->spawn_userdata);
            }
        }

        // Animation deletion
        if(script_is_tag_set_by_id(frame, TAG_MD) && state->destroy != NULL) {
            state->destroy(obj, script_get_tag_value_by_id(frame, TAG_MD), state->destroy_userdata);
        }

        // Music playback
        if(script_is_tag_set_by_id(frame, TAG_SMO)) {
            if(script_get_tag_value_by_id(frame, TAG_SMO) == 0) {
                music_tracker_stop();
                return;
            }
            music_tracker_play(PSM_END + (script_get_tag_value_by_id(frame, TAG_SMO) - 1));
        }
        if(script_is_tag_set_by_id(frame, TAG_SMF)) {
            music_tracker_stop();
        }

        // Sound playback
        if(script_is_tag_set_by_id(frame, TAG_S) && obj->sound_translation_table) {
            int sound_id = obj->sound_translation_table[script_get_tag_value_by_id(frame, TAG_S)] - 1;

            bool allow_sound_playback = true;
            if(script_is_tag_set_by_id(frame, TAG_T)) {
                const int enemy_anim = (enemy != NULL && enemy->cur_animation != NULL) ? enemy->cur_animation->id : -1;
                allow_sound_playback = (enemy_anim == ANIM_DAMAGE || enemy_anim == ANIM_STANDING_BLOCK ||
                                        enemy_anim == ANIM_CROUCHING_BLOCK || obj->gs->hit_pause > 0);
                if(!allow_sound_playback) {
                    log_warn("Sound playback blocked!");
                }
            }

            sound_opts opts;
            sound_opts_init(&opts);
            if(script_is_tag_set_by_id(frame, TAG_L)) {
                // Original game: `l` is 0..63 and the driver volume is loudness*2 clamped to 127.
                opts.volume = clamp(script_get_tag_value_by_id(frame, TAG_L) * 2, 0, 127);
            }
            if(script_is_tag_set_by_id(frame, TAG_SB)) {
                opts.panning = clamp(script_get_tag_value_by_id(frame, TAG_SB), -100, 100);
            } else {
                opts.panning = clamp((obj->pos.x - 160) * 100 / 160, -100, 100);
            }
            if(script_is_tag_set_by_id(frame, TAG_SF)) {
                opts.pitch = script_get_tag_value_by_id(frame, TAG_SF);
                assert(opts.pitch >= -128 && opts.pitch <= 128);
            }
            if(script_is_tag_set_by_id(frame, TAG_SP)) {
                opts.priority = script_get_tag_value_by_id(frame, TAG_SP);
            }
            if(script_is_tag_set_by_id(frame, TAG_SC)) {
                const int sc = script_get_tag_value_by_id(frame, TAG_SC);
                if(sc == 0) {
                    opts.stop_duplicate = true;
                } else {
                    opts.channel = clamp(sc - 1, 0, SOUND_CHANNEL_COUNT - 1);
                }
            }
            if(script_is_tag_set_by_id(frame, TAG_SD)) {
                opts.skip_duplicate = true;
            }
            if(script_is_tag_set_by_id(frame, TAG_SA) || (sound_id >= 0 && sound_id < 10)) {
                opts.follow_object_id = obj->id;
            }
            if(script_is_tag_set_by_id(frame, TAG_SL)) {
                opts.panning_end = clamp(script_get_tag_value_by_id(frame, TAG_SL), -100, 100);
                opts.has_panning_sweep = true;
            } else if(script_is_tag_set_by_id(frame, TAG_SE)) {
                opts.panning_end = clamp(script_get_tag_value_by_id(frame, TAG_SE), -100, 100);
                opts.has_panning_sweep = true;
            }
            if(allow_sound_playback) {
                game_state_play_sound(obj->gs, sound_id, &opts);
            }
        }

        // Blend mode stuff
        if(script_is_tag_set_by_id(frame, TAG_BB)) {
            rstate->screen_shake_vertical = script_get_tag_value_by_id(frame, TAG_BB);
        }
        if(script_is_tag_set_by_id(frame, TAG_BF)) {
            rstate->blend_finish = script_get_tag_value_by_id(frame, TAG_BF);
        }
        if(script_is_tag_set_by_id(frame, TAG_BL)) {
            rstate->screen_shake_horizontal = script_get_tag_value_by_id(frame, TAG_BL);
        }
        if(script_is_tag_set_by_id(frame, TAG_BS)) {
            rstate->blend_start = script_get_tag_value_by_id(frame, TAG_BS);
        }

        // Palette tricks
        if(script_is_tag_set_by_id(frame, TAG_BPD)) {
            rstate->pal_ref_index = script_get_tag_value_by_id(frame, TAG_BPD);
        }
        if(script_is_tag_set_by_id(frame, TAG_BPN)) {
            rstate->pal_entry_count = script_get_tag_value_by_id(frame, TAG_BPN);
        }
        if(script_is_tag_set_by_id(frame, TAG_BPS)) {
            rstate->pal_start_index = script_get_tag_value_by_id(frame, TAG_BPS);
        }
        if(script_is_tag_set_by_id(frame, TAG_BPF)) {
            // Exact values come from master.dat
            if(game_state_get_player(obj->gs, 0)->har_obj_id == obj->id) {
                rstate->pal_start_index = 1;
                rstate->pal_entry_count = 47;
            } else {
                rstate->pal_start_index = 48;
                rstate->pal_entry_count = 48;
            }
        }
        if(script_is_tag_set_by_id(frame, TAG_BPP)) {
            rstate->pal_end = COLOR_6TO8(script_get_tag_value_by_id(frame, TAG_BPP));
            rstate->pal_begin = COLOR_6TO8(script_get_tag_value_by_id(frame, TAG_BPP));
        }
        if(script_is_tag_set_by_id(frame, TAG_BPB)) {
            rstate->pal_begin = COLOR_6TO8(script_get_tag_value_by_id(frame, TAG_BPB));
        }
        if(script_is_tag_set_by_id(frame, TAG_BZ)) {
            rstate->pal_tint = 1;
        }

        // CREDITS palette copy tricks
        rstate->pal_tricks_off = script_is_tag_set_by_id(frame, TAG_BPO) ? 1 : 0; // Disable the standard palette tricks
        rstate->bd_flag = script_is_tag_set_by_id(
            frame, TAG_BD); // Read palette from the last frame of animation (we emulate this internally)

        // These are animation-global instead of per-frame.
        if(script_is_tag_set_by_id(frame, TAG_BA)) {
            state->pal_copy_count = script_get_tag_value_by_id(frame, TAG_BA); // Number of copies to make after bi + bc
            state->pal_copy_start = script_get_tag_value_by_id(frame, TAG_BI); // Start offset for copying
            state->pal_copy_entries = script_get_tag_value_by_id(frame, TAG_BC); // Number of indexes to copy
        }

        if(script_is_tag_set_by_id(frame, TAG_BY)) {
            object_set_shadow(obj, 0);
        }

        if(script_is_tag_set_by_id(frame, TAG_BW)) {
            object_set_shadow(obj, 1);
        }

        // Handle position correction
        if(script_is_tag_set_by_id(frame, TAG_OX)) {
            log_debug("O_CORRECTION: X = %d", script_get_tag_value_by_id(frame, TAG_OX));
            rstate->o_correction.x = script_get_tag_value_by_id(frame, TAG_OX);
        } else {
            rstate->o_correction.x = 0;
        }
        if(script_is_tag_set_by_id(frame, TAG_OY)) {
            log_debug("O_CORRECTION: Y = %d", script_get_tag_value_by_id(frame, TAG_OY));
            rstate->o_correction.y = script_get_tag_value_by_id(frame, TAG_OY);
        } else {
            rstate->o_correction.y = 0;
        }

        if(script_is_tag_set_by_id(frame, TAG_BO)) {
            player_set_shadow_correction_y(obj, script_get_tag_value_by_id(frame, TAG_BO));
        }

        // If UA is set, force other HAR to damage animation
        if(script_is_tag_set_by_id(frame, TAG_UA) && enemy) {
            har *h = object_get_userdata(obj);
            if(enemy->cur_animation->id != ANIM_DAMAGE) {
                har *eh = object_get_userdata(enemy);
                object_set_animation(enemy, &af_get_move(eh->af_data, ANIM_DAMAGE)->ani);
                eh->state = STATE_RECOIL;
            }
            af_move *move = af_get_move(h->af_data, obj->cur_animation->id);
            object_set_custom_string(enemy, str_c(&move->footer_string));
            object_set_repeat(enemy, 0);
            object_set_stride(enemy, 1);
            script_reader_seek(&enemy->animation_state.reader, script_reader_tick(&obj->animation_state.reader));
        }

        // handle scaling on the Y axis
        if(script_is_tag_set_by_id(frame, TAG_Y)) {
            obj->y_percent = script_get_tag_value_by_id(frame, TAG_Y) / 100.0f;
        }

        // Handle slides
        if(script_is_tag_set_by_id(frame, TAG_X_EQ) || script_is_tag_set_by_id(frame, TAG_Y_EQ)) {
            obj->vel = vec2f_create(0, 0);
        }
        if(script_is_tag_set_by_id(frame, TAG_X_EQ)) {
            obj->pos.x = obj->start.x + (script_get_tag_value_by_id(frame, TAG_X_EQ) * object_get_direction(obj));

            // Find frame ID by tick
            int frame_id = script_get_next_frame_with_tag_id(script, TAG_X_EQ, script_reader_tick(&state->reader));

            // Handle it!
            if(frame_id >= 0) {
                int mr = script_get_tick_pos_at_frame(script, frame_id);
                int r = mr - script_reader_tick(&state->reader) - frame->tick_len;
                int next_x = script_get_tag_value_by_id(script_get_frame(script, frame_id), TAG_X_EQ);
                int slide = obj->start.x + (next_x * object_get_direction(obj));
                if(slide != obj->pos.x) {
                    obj->slide_state.vel.x = dist(obj->pos.x, slide) / (float)(frame->tick_len + r);
                    obj->slide_state.timer = frame->tick_len + r;
                    /* log_debug("Slide object %d for X = %f for a total of %d + %d = %d ticks.",
                            obj->cur_animation->id,
                            obj->slide_state.vel.x,
                            frame->tick_len,
                            r,
                            frame->tick_len + r);*/
                }
            }
        }
        if(script_is_tag_set_by_id(frame, TAG_Y_EQ)) {
            obj->pos.y = obj->start.y + script_get_tag_value_by_id(frame, TAG_Y_EQ);

            // Find frame ID by tick
            int frame_id = script_get_next_frame_with_tag_id(script, TAG_Y_EQ, script_reader_tick(&state->reader));

            // handle it!
            if(frame_id >= 0) {
                int mr = script_get_tick_pos_at_frame(script, frame_id);
                int r = mr - script_reader_tick(&state->reader) - frame->tick_len;
                int next_y = script_get_tag_value_by_id(script_get_frame(script, frame_id), TAG_Y_EQ);
                int slide = next_y + obj->start.y;
                if(slide != obj->pos.y) {
                    obj->slide_state.vel.y = dist(obj->pos.y, slide) / (float)(frame->tick_len + r);
                    obj->slide_state.timer = frame->tick_len + r;
                    /* log_debug("Slide object %d for Y = %f for a total of %d + %d = %d ticks.",
                            obj->cur_animation->id,
                            obj->slide_state.vel.y,
                            frame->tick_len,
                            r,
                            frame->tick_len + r);*/
                }
            }
        }

        if(script_is_tag_set_by_id(frame, TAG_Q)) {
            obj->q_val = script_get_tag_value_by_id(frame, TAG_Q);
            // Enable hit if the q value is higher than the hit count for this animation
            if(obj->q_val > obj->q_counter) {
                obj->can_hit = 1;
            }
        }

        // Set video effects now.
        int effects = EFFECT_NONE;
        if(script_is_tag_set_by_id(frame, TAG_BT)) {
            effects |= EFFECT_DARK_TINT;
        }
        if(script_is_tag_set_by_id(frame, TAG_BR)) {
            effects |= EFFECT_GLOW;
        }
        if(script_is_tag_set_by_id(frame, TAG_UB)) {
            effects |= EFFECT_TRAIL;
        }
        if(script_is_tag_set_by_id(frame, TAG_BG)) {
            effects |= EFFECT_ADD;
        }
        object_set_frame_effects(obj, effects);

        // Set render settings
        object_select_sprite(obj, frame->sprite);
        if(obj->cur_sprite_id >= 0) {
            rstate->duration = frame->tick_len;
            if(script_is_tag_set_by_id(frame, TAG_R)) { // || obj->animation_state.shadow_corner_hack) {
                rstate->flipmode ^= FLIP_HORIZONTAL;
            }
            if(script_is_tag_set_by_id(frame, TAG_F)) {
                rstate->flipmode ^= FLIP_VERTICAL;
            }
        }
    }

    // Orb meandering.  Only the fire pit orb should have this tag set.
    if(script_is_tag_set_by_id(frame, TAG_AS)) {
        double base_val = (obj->orb_val & 7) + 8.f;
        double delta_1 = abs(obj->orb_val * 4) + obj->gs->tick;
        double delta_2 = abs(obj->orb_val * 2) + obj->gs->tick;
        double t = sinf(base_val * (delta_1 * 0.003574533) + obj->orb_val * 3.0) * 65.0 + 160.0;
        double t2 = cosf(base_val * (delta_2 * 0.004974533) + obj->orb_val * 4.0) * 65.0;
        obj->pos.x = (t + t2);
        t = cosf(base_val * (delta_2 * 0.005874533) + obj->orb_val * 6.0) * 30.0 + 60.0;
        t2 = sinf(base_val * (delta_1 * 0.004174533) + obj->orb_val * 3.0) * 30.0;
        obj->pos.y = (t + t2);
        obj->vel.x = 0;
        obj->vel.y = 0;
    }

    if(script_is_tag_set_by_id(frame, TAG_BU)) {
        if(obj->vel.y < 0.0f) {
            float x_dist = dist(obj->pos.x, 160);
            // assume that bu is used in conjunction with 'vy-X' and that we want to land in the center of the arena
            obj->vel.x = x_dist / (obj->vel.y * -2);
        } else {
            // teleport offscreen (Thorn's scrap)
            obj->pos.x = 160;
        }
    }

    if(script_is_tag_set_by_id(frame, TAG_CG)) {
        obj->animation_state.disable_d = obj->pos.y < ARENA_FLOOR ? 1 : 0;

        if(obj->pos.y >= ARENA_FLOOR) {
            har *h = object_get_userdata(obj);
            obj->pos.y = ARENA_FLOOR;
            obj->vel.y = 0;
            h->state = STATE_STANDING;
        }
    }

    // Tick management
    if(script_is_tag_set_by_id(frame, TAG_D) && !obj->animation_state.disable_d) {
        script_reader_mark_previous(&state->reader);
        int tick_value = script_get_tag_value_by_id(frame, TAG_D);
        if(tick_value >= 0) {
            script_reader_seek(&state->reader, tick_value + 1);
            state->looping = true;
        } else {
            script_reader_seek(&state->reader, script_get_total_ticks(script) + tick_value);
        }
        return;
    }

    // Animation ticks
    script_reader_mark_previous(&state->reader);
    script_reader_advance(&state->reader, state->reverse ? -1 : 1);

    // Sprite ticks
    rstate->timer++;

    // All done.
    return;
}

void player_jump_to_tick(object *obj, int tick) {
    player_animation_state *state = &obj->animation_state;
    script_reader_mark_previous(&state->reader);
    script_reader_seek(&state->reader, tick);
}

unsigned int player_get_len_ticks(const object *obj) {
    return script_get_total_ticks(script_reader_get_script(&obj->animation_state.reader));
}

void player_set_repeat(object *obj, int repeat) {
    obj->animation_state.repeat = repeat;
}

int player_get_repeat(const object *obj) {
    return obj->animation_state.repeat;
}

void player_next_frame(object *obj) {
    player_animation_state *state = &obj->animation_state;
    int current_index =
        script_get_frame_index_at(script_reader_get_script(&state->reader), script_reader_tick(&state->reader));
    script_reader_seek(&state->reader,
                       script_get_tick_pos_at_frame(script_reader_get_script(&state->reader), current_index + 1));
    script_reader_mark_entered(&state->reader);
}

void player_goto_frame(object *obj, int frame_id) {
    player_animation_state *state = &obj->animation_state;
    script_reader_seek(&state->reader,
                       script_get_tick_pos_at_frame(script_reader_get_script(&state->reader), frame_id));
    script_reader_mark_entered(&state->reader);
}

uint32_t player_get_current_tick(const object *obj) {
    return script_reader_tick(&obj->animation_state.reader);
}

int player_get_last_frame(const object *obj) {
    const script *s = script_reader_get_script(&obj->animation_state.reader);
    return script_get_frame_at(s, script_get_total_ticks(s) - 1)->sprite;
}

char player_get_last_frame_letter(const object *obj) {
    return script_frame_to_letter(player_get_last_frame(obj));
}

bool player_is_looping(const object *obj) {
    const player_animation_state *state = &obj->animation_state;
    return state->looping;
}

void player_set_shadow_correction_y(object *obj, int value) {
    obj->o_shadow_correction = value;
    if(value == 0) {
        obj->cast_shadow = 0;
    }
}
