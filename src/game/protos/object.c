#include "game/protos/object.h"
#include "formats/sprite.h"
#include "game/game_state.h"
#include "game/objects/arena_constraints.h"
#include "utils/allocator.h"
#include "utils/compat.h"
#include "utils/log.h"
#include "utils/miscmath.h"
#include "video/vga_state.h"
#include "video/video.h"
#include <stdlib.h>
#include <string.h>

#define UNUSED(x) (void)(x)

static uint32_t object_id = 1;

/** \brief Creates a new, empty object.
 * \param obj Object handle
 * \param gs Game state handle
 * \param pos Initial position
 * \param vel Initial velocity
 */
void object_create(object *obj, game_state *gs, vec2i pos, vec2f vel) {
    // State
    obj->gs = gs;
    obj->id = object_id++;

    // Position related
    obj->pos = vec2i_to_f(pos);
    // remember the place we were spawned, the x= and y= tags are relative to that
    obj->start = vec2i_to_f(pos);
    obj->vel = vel;
    obj->horizontal_velocity_modifier = obj->vertical_velocity_modifier = 1.0f;
    obj->direction = OBJECT_FACE_RIGHT;
    obj->y_percent = 1.0;
    obj->x_percent = 1.0;

    // Physics
    obj->layers = OBJECT_DEFAULT_LAYER;
    obj->group = OBJECT_NO_GROUP;
    obj->gravity = 0.0f;

    // Video effect stuff
    obj->animation_video_effects = 0;
    obj->frame_video_effects = 0;

    // Attachment stuff
    obj->attached_to_id = 0;

    // Fire orb wandering
    obj->orbit = 0;
    obj->orbit_tick = MATH_PI / 2.0f;
    obj->orbit_dest = obj->start;
    obj->orbit_pos = obj->start;
    obj->orbit_pos_vary = vec2f_create(0, 0);

    // Animation playback related
    obj->cur_animation_own = OWNER_EXTERNAL;
    obj->cur_animation = NULL;
    obj->cur_sprite_id = -1;
    obj->sprite_override = 0;
    obj->sound_translation_table = NULL;
    obj->cur_surface = NULL;
    obj->cur_remap = -1;
    obj->pal_offset = 0;
    obj->pal_limit = 255;
    obj->halt = 0;
    obj->halt_ticks = 0;
    obj->stride = 1;
    obj->cast_shadow = 0;
    obj->age = 0;
    player_create(obj);

    obj->custom_str = NULL;

    random_seed(&obj->rand_state, rand_intmax());

    // For enabling hit on the current and the next n-1 frames
    obj->hit_frames = 0;
    obj->can_hit = 0;

    // Callbacks & userdata
    obj->userdata = NULL;
    obj->dynamic_tick = NULL;
    obj->static_tick = NULL;
    obj->free = NULL;
    obj->act = NULL;
    obj->collide = NULL;
    obj->finish = NULL;
    obj->move = NULL;
    obj->debug = NULL;
    obj->clone = NULL;
    obj->clone_free = NULL;
}

int object_clone(object *src, object *dst, game_state *gs) {
    memcpy(dst, src, sizeof(object));
    dst->gs = gs;
    player_clone(src, dst);
    if(src->custom_str) {
        dst->custom_str = strdup(src->custom_str);
    }

    if(src->cur_animation_own == OWNER_OBJECT) {
        dst->cur_animation = omf_calloc(1, sizeof(animation));
        animation_clone(src->cur_animation, dst->cur_animation);
    }

    if(src->clone) {
        src->clone(src, dst);
    }

    return 0;
}

// FIXME: This was removed in HEAD, not sure why or what is the replacement
// TODO: GET RID
void object_create_static(object *obj, game_state *gs) {
    object_create(obj, gs, vec2i_create(0, 0), vec2f_create(0, 0));
}

void object_set_stride(object *obj, int stride) {
    if(stride < 1) {
        stride = 1;
    }
    obj->stride = stride;
}

void object_set_delay(object *obj, int delay) {
    player_set_delay(obj, delay);
}

void object_set_playback_direction(object *obj, int dir) {
    if(dir != PLAY_FORWARDS && dir != PLAY_BACKWARDS) {
        dir = PLAY_FORWARDS;
    }
    if(dir == PLAY_BACKWARDS) {
        obj->animation_state.reverse = 1;
    } else {
        obj->animation_state.reverse = 0;
    }
}

void object_scenewide_palette_transform(damage_tracker *damage, vga_palette *pal, void *userdata) {
    float u, k, step, bp;
    uint8_t m;
    vga_color ref;
    vga_index start, end;
    player_sprite_state *state = userdata;

    // Make sure stuff seems legit.
    assert(state->pal_start_index < 256);
    assert(state->pal_start_index + state->pal_entry_count <= 256);

    step = state->timer / (float)state->duration;
    bp = (float)state->pal_begin + (state->pal_end - state->pal_begin) * step;
    k = bp / 255.0f;
    ref = pal->colors[state->pal_ref_index];
    start = state->pal_start_index;
    end = state->pal_start_index + state->pal_entry_count;

    if(state->pal_tint) {
        for(vga_index i = start; i < end; i++) {
            m = max3(pal->colors[i].r, pal->colors[i].g, pal->colors[i].b);
            u = m / 255.0f;
            pal->colors[i].r = clamp(pal->colors[i].r + u * k * (ref.r - pal->colors[i].r), 0, 255);
            pal->colors[i].g = clamp(pal->colors[i].g + u * k * (ref.g - pal->colors[i].g), 0, 255);
            pal->colors[i].b = clamp(pal->colors[i].b + u * k * (ref.b - pal->colors[i].b), 0, 255);
        }
    } else {
        for(vga_index i = start; i < end; i++) {
            pal->colors[i].r = clamp(pal->colors[i].r * (1 - k) + (ref.r * k), 0, 255);
            pal->colors[i].g = clamp(pal->colors[i].g * (1 - k) + (ref.g * k), 0, 255);
            pal->colors[i].b = clamp(pal->colors[i].b * (1 - k) + (ref.b * k), 0, 255);
        }
    }

    // Mark the palette as damaged
    damage_set_range(damage, start, end);
}

void object_palette_copy_transform(damage_tracker *damage, vga_palette *pal, void *userdata) {
    player_sprite_state *state = userdata;
    int src_start = state->pal_copy_start;
    int src_end = state->pal_copy_entries + src_start;
    float step = state->timer / (float)state->duration;
    float bpp = (state->pal_begin + (state->pal_end - state->pal_begin) * step) / 255.0;

    int pos = src_end;
    for(int i = 0; i < state->pal_copy_count; i++) {
        for(int w = src_start; w < src_end; w++) {
            pal->colors[pos].r = clamp(pal->colors[w].r * bpp, 0, 255);
            pal->colors[pos].g = clamp(pal->colors[w].g * bpp, 0, 255);
            pal->colors[pos].b = clamp(pal->colors[w].b * bpp, 0, 255);
            pos++;
        }
    }

    damage_set_range(damage, src_end, pos);
}

void object_dynamic_tick(object *obj) {
    obj->age++;

    if(obj->attached_to_id != 0) {
        object *attached_to = game_state_find_object(obj->gs, obj->attached_to_id);
        object_set_pos(obj, object_get_pos(attached_to));
        object_set_direction(obj, object_get_direction(attached_to));
    }

    // Check if object still needs to be halted
    if(obj->halt_ticks > 0) {
        obj->halt_ticks--;
        obj->halt = (obj->halt_ticks > 0);
    }

    // Run animation player
    if(obj->cur_animation != NULL && obj->halt == 0) {
        for(int i = 0; i < obj->stride; i++)
            player_run(obj);
    }

    // Tick object implementation
    if(obj->dynamic_tick != NULL) {
        obj->dynamic_tick(obj);
    }

    // Handle screen shakes, V & H
    if(obj->sprite_state.screen_shake_vertical > 0) {
        obj->gs->screen_shake_vertical = obj->sprite_state.screen_shake_vertical * 4;
        obj->sprite_state.screen_shake_vertical = 0;
    }
    if(obj->sprite_state.screen_shake_horizontal > 0) {
        obj->gs->screen_shake_horizontal = obj->sprite_state.screen_shake_horizontal * 4;
        obj->sprite_state.screen_shake_horizontal = 0;
    }

    if(obj->sprite_state.pal_tricks_off && obj->sprite_state.pal_copy_count > 0) { // BPO tag is on
        vga_state_use_palette_transform(object_palette_copy_transform, &obj->sprite_state);
    } else if(obj->sprite_state.pal_entry_count > 0 && obj->sprite_state.duration > 0) { // BPO tag is off
        vga_state_use_palette_transform(object_scenewide_palette_transform, &obj->sprite_state);
    }
}

void object_static_tick(object *obj) {
    if(obj->static_tick != NULL) {
        obj->static_tick(obj);
    }
}

/*
 * If negative, sets position to end - ticks, otherwise start + ticks.
 */
void object_set_tick_pos(object *obj, int tick) {
    if(obj->cur_animation != NULL && obj->halt == 0) {
        if(tick < 0) {
            player_jump_to_tick(obj, player_get_len_ticks(obj) + tick);
        } else {
            player_jump_to_tick(obj, tick);
        }
    }
}

void object_debug(object *obj) {
    if(obj->debug != NULL) {
        obj->debug(obj);
    }
}

void object_collide(object *obj, object *b) {
    if(obj->collide != NULL) {
        obj->collide(obj, b);
    }
}

void object_set_frame_effects(object *obj, uint32_t effects) {
    obj->frame_video_effects = effects;
}

void object_set_animation_effects(object *obj, uint32_t effects) {
    obj->animation_video_effects = effects;
}

void object_add_animation_effects(object *obj, uint32_t effects) {
    obj->animation_video_effects |= effects;
}

void object_del_animation_effects(object *obj, uint32_t effects) {
    obj->animation_video_effects &= ~effects;
}

bool object_has_effect(const object *obj, uint32_t effect) {
    return obj->animation_video_effects & effect || obj->frame_video_effects & effect;
}

void object_add_frame_effects(object *obj, uint32_t effects) {
    obj->frame_video_effects |= effects;
}

void object_del_frame_effects(object *obj, uint32_t effects) {
    obj->frame_video_effects &= ~effects;
}

void object_render(object *obj) {
    // Stop here if cur_sprite_id is not set
    if(obj->cur_sprite_id < 0)
        return;

    const sprite *cur_sprite = animation_get_sprite(obj->cur_animation, obj->cur_sprite_id);
    if(cur_sprite == NULL)
        return;

    // Set current surface
    obj->cur_surface = cur_sprite->data;

    // Something to ease the pain ...
    player_sprite_state *rstate = &obj->sprite_state;

    // Position
    int x;
    int y;
    int w = obj->cur_surface->w * obj->x_percent;
    int h = obj->cur_surface->h * obj->y_percent;

    // Set Y coord, take into account sprite flipping
    if(rstate->flipmode & FLIP_VERTICAL) {
        y = obj->pos.y - cur_sprite->pos.y + rstate->o_correction.y - object_get_size(obj).y;

        if(obj->cur_animation->id == ANIM_JUMPING) {
            y -= 100;
        }
    } else {
        y = obj->pos.y + cur_sprite->pos.y + rstate->o_correction.y;
    }

    // Set X coord, take into account the HAR facing.
    if(object_get_direction(obj) == OBJECT_FACE_LEFT) {
        x = obj->pos.x - cur_sprite->pos.x + rstate->o_correction.x - object_get_size(obj).x;
    } else {
        x = obj->pos.x + cur_sprite->pos.x + rstate->o_correction.x;
    }

    // Centrify if scaled
    x = x + (obj->cur_surface->w - w) / 2;
    y = y + (obj->cur_surface->h - h) / 2;

    // Flip to face the right direction
    int flip_mode = rstate->flipmode;
    if(object_get_direction(obj) == OBJECT_FACE_LEFT) {
        flip_mode ^= FLIP_HORIZONTAL;
    }

    uint8_t opacity = rstate->blend_finish;
    if(rstate->duration > 0) {
        float moment = (float)rstate->timer / (float)rstate->duration;
        float d = ((float)rstate->blend_finish - (float)rstate->blend_start) * moment;
        opacity = clamp(rstate->blend_start + d, 0, 255);
    }

    int remap_offset = 0;
    int remap_rounds = 0;
    unsigned int options = 0;
    if(object_has_effect(obj, EFFECT_GLOW)) {
        remap_rounds = 1;
        remap_offset = 3;

        if(object_has_effect(obj, EFFECT_SATURATE)) {
            remap_rounds = 10;
        }
    } else if(object_has_effect(obj, EFFECT_SHADOW)) {
        remap_rounds = 1;
        remap_offset = clamp((opacity * 4) >> 8, 0, 3);
    } else if(object_has_effect(obj, EFFECT_DARK_TINT)) {
        remap_rounds = 0;
        remap_offset = 5;
        options |= REMAP_SPRITE;
    } else if(object_has_effect(obj, EFFECT_STASIS)) {
        remap_rounds = 0;
        remap_offset = 5;
        options |= REMAP_SPRITE;
    } else if(object_has_effect(obj, EFFECT_POSITIONAL_LIGHTING)) {
        int rx = x + (w >> 1);
        remap_rounds = 0;
        remap_offset = clamp(6 + floorf(((rx > 160) ? 320 - rx : rx) / 60), 6, 8);
        options |= REMAP_SPRITE;
    }

    video_draw_full(obj->cur_surface, x, y, w, h, remap_offset, remap_rounds, obj->pal_offset, obj->pal_limit,
                    flip_mode, options);
}

void object_render_shadow(object *obj) {
    if(obj->cur_sprite_id < 0 || !obj->cast_shadow) {
        return;
    }

    const sprite *cur_sprite = animation_get_sprite(obj->cur_animation, obj->cur_sprite_id);
    if(cur_sprite == NULL) {
        return;
    }

    // Scale of the sprite on Y axis should be less than the
    // height of the sprite because of light position
    float scale_y = 0.25f;
    int w = cur_sprite->data->w;
    int h = cur_sprite->data->h;
    int scaled_h = h * scale_y;

    // Determine X
    int flip_mode = obj->sprite_state.flipmode;
    int x = obj->pos.x + cur_sprite->pos.x + obj->sprite_state.o_correction.x;
    if(object_get_direction(obj) == OBJECT_FACE_LEFT) {
        x = (obj->pos.x + obj->sprite_state.o_correction.x) - cur_sprite->pos.x - object_get_size(obj).x;
        flip_mode ^= FLIP_HORIZONTAL;
    }

    // Determine Y
    int y = 190 - scaled_h;

    // Render shadow object twice with different offsets, so that
    // the shadows seem a bit blobbier and shadow-y
    for(int i = 0; i < 2; i++) {
        video_draw_full(cur_sprite->data, x + i, y + i, w, scaled_h, 2, 1, obj->pal_offset, obj->pal_limit, flip_mode,
                        SPRITE_MASK);
    }
}

int object_act(object *obj, int action) {
    if(obj->act != NULL) {
        int res = obj->act(obj, action);
        return res;
    }
    return 0;
}

void object_move(object *obj) {
    if(obj->sprite_state.disable_gravity) {
        object_set_vel(obj, vec2f_create(0, 0));
    }
    if(obj->move != NULL) {
        obj->move(obj);
    }
}

/** Frees the object and all resources attached to it (even the animation, if it is owned by the object)
 * \param obj Object handle
 */
void object_free(object *obj) {
    if(obj == NULL) {
        return;
    }
    vga_state_dontuse_palette_transform(object_scenewide_palette_transform, obj);
    if(obj->free != NULL) {
        obj->free(obj);
    }
    player_free(obj);
    if(obj->cur_animation_own == OWNER_OBJECT) {
        animation_free(obj->cur_animation);
        omf_free(obj->cur_animation);
    }
    if(obj->custom_str) {
        omf_free(obj->custom_str);
    }
    obj->cur_surface = NULL;
    obj->cur_animation = NULL;
}

int object_clone_free(object *obj) {
    if(obj->clone_free != NULL) {
        obj->clone_free(obj);
    }
    player_free(obj);
    if(obj->cur_animation_own == OWNER_OBJECT) {
        animation_free(obj->cur_animation);
        omf_free(obj->cur_animation);
    }
    if(obj->custom_str) {
        omf_free(obj->custom_str);
    }
    obj->cur_surface = NULL;
    obj->cur_animation = NULL;
    return 0;
}

/** Sets a pointer to a sound translation table. Note! Does NOT copy!
 * \param obj Object handle
 * \param ptr Pointer to the STL (30 byte char array)
 */
void object_set_stl(object *obj, const char *ptr) {
    obj->sound_translation_table = ptr;
}

/** Returns a pointer to the sound translation table (30 byte char array)
 * \param obj Object handle
 * \return Pointer to the sound translation table
 */
const char *object_get_stl(const object *obj) {
    return obj->sound_translation_table;
}

/** Sets the owner of the animation. If OWNER_OBJECT, the object will free animation on object_free().
 * \param obj Object handle
 * \param owner Owner of the object (OWNER_EXTERNAL, OWNER_OBJECT)
 */
void object_set_animation_owner(object *obj, int owner) {
    obj->cur_animation_own = owner;
}

/** Sets an animation for object. It will automatically start playing on first tick.
 * \param obj Object handle
 * \param ani Animation to attach
 */
void object_set_animation(object *obj, animation *ani) {
    if(obj->cur_animation != NULL && obj->cur_animation_own == OWNER_OBJECT) {
        animation_free(obj->cur_animation);
        omf_free(obj->cur_animation);
    }
    omf_free(obj->custom_str);

    obj->cur_animation = ani;
    obj->cur_animation_own = OWNER_EXTERNAL;
    player_reload(obj);

    // Debug texts
    if(obj->cur_animation->id == -1) {
        DEBUG("Custom object set to (x,y) = (%f,%f).", obj->pos.x, obj->pos.y);
    } else {
        /*DEBUG("Animation object %d set to (x,y) = (%f,%f) with \"%s\".", */
        /*obj->cur_animation->id,*/
        /*obj->pos.x, obj->pos.y,*/
        /*str_c(&obj->cur_animation->animation_string));*/
    }
}

/** Sets a new animation string to currently playing animation
 * \param obj Object handle
 * \param str New animation string
 */
void object_set_custom_string(object *obj, const char *str) {
    omf_free(obj->custom_str);
    obj->custom_str = strdup(str);
    player_reload_with_str(obj, obj->custom_str);
    // DEBUG("Set animation string to %s", obj->custom_str);
}

/** Returns a pointer to the currently playing animation
 * \param obj Object handle
 * \return animation* Pointer to current animation
 */
animation *object_get_animation(object *obj) {
    return obj->cur_animation;
}

/** Selects sprite to show. Note! Animation string will override this!
 * \param obj Object handle
 * \param id Sprite ID (starting from 0). Negative values will set sprite to nonexistent (NULL).
 */
void object_select_sprite(object *obj, int id) {
    if(obj == NULL)
        return;
    if(!obj->sprite_override) {
        if(id < 0) {
            obj->cur_sprite_id = -1;
        } else {
            if(animation_get_sprite(obj->cur_animation, id)) {
                obj->cur_sprite_id = id;
                obj->sprite_state.flipmode = FLIP_NONE;
            } else {
                obj->cur_sprite_id = -1;
            }
        }
    }
}

/** Tell object to NOT change currently selected sprite, even if animation string tells it to.
 * \param obj Object handle
 * \param override Set override (1|0)
 */
void object_set_sprite_override(object *obj, int override) {
    obj->sprite_override = override;
}

void object_set_userdata(object *obj, void *ptr) {
    obj->userdata = ptr;
}
void *object_get_userdata(const object *obj) {
    return obj->userdata;
}
void object_set_free_cb(object *obj, object_free_cb cbfunc) {
    obj->free = cbfunc;
}
void object_set_act_cb(object *obj, object_act_cb cbfunc) {
    obj->act = cbfunc;
}
void object_set_static_tick_cb(object *obj, object_tick_cb cbfunc) {
    obj->static_tick = cbfunc;
}
void object_set_dynamic_tick_cb(object *obj, object_tick_cb cbfunc) {
    obj->dynamic_tick = cbfunc;
}
void object_set_collide_cb(object *obj, object_collide_cb cbfunc) {
    obj->collide = cbfunc;
}
void object_set_finish_cb(object *obj, object_finish_cb cbfunc) {
    obj->finish = cbfunc;
}
void object_set_move_cb(object *obj, object_move_cb cbfunc) {
    obj->move = cbfunc;
}
void object_set_debug_cb(object *obj, object_debug_cb cbfunc) {
    obj->debug = cbfunc;
}

void object_set_layers(object *obj, int layers) {
    obj->layers = layers;
}
void object_set_group(object *obj, int group) {
    obj->group = group;
}
void object_set_gravity(object *obj, float gravity) {
    obj->gravity = gravity;
}

float object_get_gravity(const object *obj) {
    return obj->gravity;
}
int object_get_group(const object *obj) {
    return obj->group;
}
int object_get_layers(const object *obj) {
    return obj->layers;
}

void object_set_pal_offset(object *obj, int offset) {
    obj->pal_offset = offset;
}
int object_get_pal_offset(const object *obj) {
    return obj->pal_offset;
}

void object_set_pal_limit(object *obj, int limit) {
    obj->pal_limit = limit;
}
int object_get_pal_limit(const object *obj) {
    return obj->pal_limit;
}

void object_set_halt_ticks(object *obj, int ticks) {
    obj->halt = (ticks > 0);
    obj->halt_ticks = ticks;
}
int object_get_halt_ticks(object *obj) {
    return obj->halt_ticks;
}

void object_set_halt(object *obj, int halt) {
    obj->halt = halt;
    obj->halt_ticks = (halt == 0 ? 0 : obj->halt_ticks);
}
int object_get_halt(const object *obj) {
    return obj->halt;
}

void object_set_repeat(object *obj, int repeat) {
    player_set_repeat(obj, repeat);
}
int object_get_repeat(const object *obj) {
    return player_get_repeat(obj);
}
int object_finished(object *obj) {
    return obj->animation_state.finished;
}

void object_set_direction(object *obj, int dir) {
    obj->direction = dir;
}
int object_get_direction(const object *obj) {
    return obj->direction * obj->sprite_state.dir_correction;
}

void object_set_shadow(object *obj, int enable) {
    obj->cast_shadow = enable;
}
int object_get_shadow(const object *obj) {
    return obj->cast_shadow;
}

int object_w(const object *obj) {
    return object_get_size(obj).x;
}
int object_h(const object *obj) {
    return object_get_size(obj).y;
}
int object_px(const object *obj) {
    return vec2f_to_i(obj->pos).x;
}
int object_py(const object *obj) {
    return vec2f_to_i(obj->pos).y;
}
float object_vx(const object *obj) {
    return obj->vel.x;
}
float object_vy(const object *obj) {
    return obj->vel.y;
}

void object_set_px(object *obj, int val) {
    obj->pos.x = val;
}
void object_set_py(object *obj, int val) {
    obj->pos.y = val;
}
void object_set_vx(object *obj, float val) {
    obj->vel.x = val;
}
void object_set_vy(object *obj, float val) {
    obj->vel.y = val;
}

vec2i object_get_pos(const object *obj) {
    return vec2f_to_i(obj->pos);
}
vec2f object_get_vel(const object *obj) {
    return obj->vel;
}
void object_set_pos(object *obj, vec2i pos) {
    obj->pos = vec2i_to_f(pos);
}
void object_set_vel(object *obj, vec2f vel) {
    obj->vel = vel;
}

vec2i object_get_size(const object *obj) {
    if(obj->cur_sprite_id >= 0) {
        sprite *cur_sprite = animation_get_sprite(obj->cur_animation, obj->cur_sprite_id);
        return sprite_get_size(cur_sprite);
    }
    return vec2i_create(0, 0);
}

void object_disable_rewind_tag(object *obj, int disable_d) {
    obj->animation_state.disable_d = disable_d;
}

int object_is_rewind_tag_disabled(const object *obj) {
    return obj->animation_state.disable_d;
}

uint32_t object_get_age(object *obj) {
    return obj->age;
}

void object_set_spawn_cb(object *obj, object_state_add_cb cbf, void *userdata) {
    obj->animation_state.spawn = cbf;
    obj->animation_state.spawn_userdata = userdata;
}

void object_set_destroy_cb(object *obj, object_state_del_cb cbf, void *userdata) {
    obj->animation_state.destroy = cbf;
    obj->animation_state.destroy_userdata = userdata;
}

int object_is_airborne(const object *obj) {
    return obj->pos.y < ARENA_FLOOR;
}

/* Attaches one object to another. Positions are synced to this from the attached. */
void object_attach_to(object *obj, const object *attach_to) {
    obj->attached_to_id = attach_to->id;
}
