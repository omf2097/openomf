#include <stdlib.h>
#include <string.h>
#include <shadowdive/sprite.h>
#include "game/protos/object.h"
#include "game/protos/object_specializer.h"
#include "game/objects/arena_constraints.h"
#include "game/game_state_type.h"
#include "video/video.h"
#include "utils/log.h"
#include "utils/miscmath.h"

#define UNUSED(x) (void)(x)

/** \brief Creates a new, empty object.
  * \param obj Object handle
  * \param gs Game state handle
  * \param pos Initial position
  * \param vel Initial velocity
  */
void object_create(object *obj, game_state *gs, vec2i pos, vec2f vel) {
    // State
    obj->gs = gs;

    // Position related
    obj->pos = vec2i_to_f(pos);
    // remember the place we were spawned, the x= and y= tags are relative to that
    obj->start = vec2i_to_f(pos);
    obj->vel = vel;
    obj->direction = OBJECT_FACE_RIGHT;
    obj->y_percent = 1.0;

    // Physics
    obj->layers = OBJECT_DEFAULT_LAYER;
    obj->group = OBJECT_NO_GROUP;
    obj->gravity = 0.0f;

    // Video effect stuff
    obj->video_effects = 0;

    // Attachment stuff
    obj->attached_to = NULL;

    // Fire orb wandering
    obj->orbit = 0;
    obj->orbit_tick = MATH_PI/2.0f;
    obj->orbit_dest = obj->start;
    obj->orbit_pos = obj->start;
    obj->orbit_pos_vary = vec2f_create(0, 0);

    // Animation playback related
    obj->cur_animation_own = OWNER_EXTERNAL;
    obj->cur_animation = NULL;
    obj->cur_sprite = NULL;
    obj->sprite_override = 0;
    obj->sound_translation_table = NULL;
    obj->cur_surface = NULL;
    obj->cur_remap = -1;
    obj->pal_offset = 0;
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
    obj->serialize = NULL;
    obj->unserialize = NULL;
    obj->debug = NULL;
    obj->pal_transform = NULL;
}

/*
 * Serializes the object to a buffer. Should return 1 on error, 0 on success
 * This will call the specialized objects, eg. har or projectile for their
 * serialization data.
 */
int object_serialize(object *obj, serial *ser) {
    serial_write_float(ser, obj->pos.x);
    serial_write_float(ser, obj->pos.y);
    serial_write_float(ser, obj->vel.x);
    serial_write_float(ser, obj->vel.y);
    serial_write_float(ser, obj->gravity);
    serial_write_int8(ser, obj->direction);
    serial_write_int8(ser, obj->group);
    serial_write_int8(ser, obj->layers);
    serial_write_int8(ser, obj->stride);
    serial_write_int8(ser, object_get_repeat(obj));
    serial_write_int8(ser, obj->sprite_override);
    serial_write_int32(ser, obj->age);
    serial_write_int32(ser, random_get_seed(&obj->rand_state));
    serial_write_int8(ser, obj->cur_animation->id);
    serial_write_int8(ser, obj->pal_offset);
    serial_write_int8(ser, obj->hit_frames);
    serial_write_int8(ser, obj->can_hit);

    // Write animation state
    if (obj->custom_str) {
        serial_write_int16(ser, strlen(obj->custom_str)+1);
        serial_write(ser, obj->custom_str, strlen(obj->custom_str)+1);
    } else {
        // using regular animation string from animation
        serial_write_int16(ser, 0);
    }
    serial_write_int16(ser, (int)obj->animation_state.current_tick);
    serial_write_int16(ser, (int)obj->animation_state.previous_tick);
    serial_write_int8(ser, (int)obj->animation_state.reverse);

    /*DEBUG("Animation state: [%d] %s, ticks = %d stride = %d direction = %d pos = %f,%f vel = %f,%f gravity = %f", strlen(player_get_str(obj))+1, player_get_str(obj), obj->animation_state.ticks, obj->stride, obj->animation_state.reverse, obj->pos.x, obj->pos.y, obj->vel.x, obj->vel.y, obj->gravity);*/

    // Serialize the underlying object
    if(obj->serialize != NULL) {
        obj->serialize(obj, ser);
    } else {
        serial_write_int8(ser, SPECID_NONE);
    }

    // Return success
    return 0;
}

/*
 * Unserializes the data from buffer to a specialized object.
 * Should return 1 on error, 0 on success.
 * Serial reder position should be set to correct position before calling this.
 */
int object_unserialize(object *obj, serial *ser, game_state *gs) {
    obj->pos.x = serial_read_float(ser);
    obj->pos.y = serial_read_float(ser);
    obj->vel.x = serial_read_float(ser);
    obj->vel.y = serial_read_float(ser);
    float gravity = serial_read_float(ser);
    obj->direction = serial_read_int8(ser);
    obj->group = serial_read_int8(ser);
    obj->layers = serial_read_int8(ser);
    uint8_t stride = serial_read_int8(ser);
    uint8_t repeat = serial_read_int8(ser);
    obj->sprite_override = serial_read_int8(ser);
    obj->age = serial_read_int32(ser);
    random_seed(&obj->rand_state, serial_read_int32(ser));
    uint8_t animation_id = serial_read_int8(ser);
    uint8_t pal_offset = serial_read_int8(ser);
    int8_t hit_frames = serial_read_int8(ser);
    int8_t can_hit = serial_read_int8(ser);

    // Other stuff not included in serialization
    obj->y_percent = 1.0;
    obj->cur_animation_own = OWNER_EXTERNAL;
    obj->cur_animation = NULL;
    obj->cur_sprite = NULL;
    obj->sound_translation_table = NULL;
    obj->cur_surface = NULL;
    obj->cur_remap = 0;
    obj->halt = 0;
    obj->halt_ticks = 0;
    obj->cast_shadow = 0;
    player_create(obj);

    // Read animation state
    uint16_t anim_str_len = serial_read_int16(ser);
    char anim_str[anim_str_len+1];
    if(anim_str_len > 0) {
        serial_read(ser, anim_str, anim_str_len);
    }
    obj->animation_state.current_tick = (uint16_t)serial_read_int16(ser);
    obj->animation_state.previous_tick = (uint16_t)serial_read_int16(ser);
    uint8_t reverse = serial_read_int8(ser);

    // Read the specialization ID from ther serial "stream".
    // This should be an int.
    int specialization_id = serial_read_int8(ser);

    // This should automatically bootstrap the object so that it has at least
    // unserialize function callback and local memory allocated
    object_auto_specialize(obj, specialization_id);

    // Now, if the object has unserialize function, call it with
    // serialization data. serial object should be pointing to the
    // start of that data.
    if(obj->unserialize != NULL) {
        obj->unserialize(obj, ser, animation_id, gs);
    } else {
        DEBUG("object has no special unserializer");
    }

    // Init animation with correct string and tick
    if(anim_str_len > 0) {
        // server is using a custom string
        DEBUG("serialized object has custom animation string %s", anim_str);
        player_reload_with_str(obj, anim_str);
    }
    if(reverse) {
        object_set_playback_direction(obj, PLAY_BACKWARDS);
    }

    // deserializing hars can reset these, so we have to set this late
    obj->stride = stride;
    object_set_gravity(obj, gravity);
    object_set_repeat(obj, repeat);
    object_set_pal_offset(obj, pal_offset);
    obj->hit_frames = hit_frames;
    obj->can_hit = can_hit;

    /*DEBUG("Animation state: [%d] %s, ticks = %d stride = %d direction = %d pos = %f,%f vel = %f,%f gravity = %f", strlen(player_get_str(obj))+1, player_get_str(obj), obj->animation_state.ticks, obj->stride, obj->animation_state.reverse, obj->pos.x, obj->pos.y, obj->vel.x, obj->vel.y, obj->gravity);*/

    // Return success
    return 0;
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

void object_dynamic_tick(object *obj) {
    obj->age++;

    if(obj->attached_to != NULL) {
        object_set_pos(obj, object_get_pos(obj->attached_to));
        object_set_direction(obj, object_get_direction(obj->attached_to));
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
        obj->collide(obj,b);
    }
}

void object_set_effects(object *obj, int effects) {
    obj->video_effects = effects;
}

int object_get_effects(const object *obj) {
    return obj->video_effects;
}

void object_add_effects(object *obj, int effects) {
    obj->video_effects |= effects;
}

void object_del_effects(object *obj, int effects) {
    obj->video_effects &= ~effects;
}

void object_render(object *obj) {
    // Stop here if cur_sprite is NULL
    if(obj->cur_sprite == NULL) return;

    // Set current surface
    obj->cur_surface = obj->cur_sprite->data;

    // Something to ease the pain ...
    player_sprite_state *rstate = &obj->sprite_state;

    // Position
    int y = obj->pos.y + obj->cur_sprite->pos.y + rstate->o_correction.y;
    int x = obj->pos.x + obj->cur_sprite->pos.x + rstate->o_correction.x;
    if(object_get_direction(obj) == OBJECT_FACE_LEFT) {
        x = obj->pos.x - obj->cur_sprite->pos.x  + rstate->o_correction.x - object_get_size(obj).x;
    }

    // Flip to face the right direction
    int flipmode = rstate->flipmode;
    if(obj->direction == OBJECT_FACE_LEFT) {
        flipmode ^= FLIP_HORIZONTAL;
    }

    // Blend start / blend finish
    uint8_t opacity = rstate->blend_finish;
    if(rstate->duration > 0) {
        float moment = (float)rstate->timer / (float)rstate->duration;
        float d = ((float)rstate->blend_finish - (float)rstate->blend_start) * moment;
        opacity = rstate->blend_start + d;
    }

    // Default Tint color
    color tint = color_create(0xFF, 0xFF, 0xFF, 0xFF);

    // These two force set the sprite color, so handle them first
    if(obj->video_effects & EFFECT_SHADOW) {
        tint = color_create(0x20, 0x20, 0x20, 0xFF);
    }
    if(obj->video_effects & EFFECT_DARK_TINT) {
        tint = color_create(0x60, 0x60, 0x60, 0xFF);
    }
    if(obj->video_effects & EFFECT_STASIS) {
        opacity = 128;
    }

    // This changes the tint depending on position, so handle next
    if(obj->video_effects & EFFECT_POSITIONAL_LIGHTING) {
        float p = (x > 160) ? 320 - x : x;
        float shade = 0.65f + p / 320;
        if(shade > 1.0f) shade = 1.0f;
        tint.r *= shade;
        tint.g *= shade;
        tint.b *= shade;
    }

    // Render
    video_render_sprite_flip_scale_opacity_tint(
        obj->cur_surface,
        x, y,
        rstate->blendmode,
        obj->pal_offset,
        flipmode,
        obj->y_percent,
        opacity,
        tint);
}

void object_render_shadow(object *obj) {
    if(obj->cur_sprite == NULL || !obj->cast_shadow) {
        return;
    }

    // Scale of the sprite on Y axis should be less than the
    // height of the sprite because of light position
    float scale_y = 0.25f;

    // Determine X
    int flipmode = obj->sprite_state.flipmode;
    int x = obj->pos.x + obj->cur_sprite->pos.x + obj->sprite_state.o_correction.x;
    if(object_get_direction(obj) == OBJECT_FACE_LEFT) {
        x = (obj->pos.x + obj->sprite_state.o_correction.x) - obj->cur_sprite->pos.x - object_get_size(obj).x;
        flipmode ^= FLIP_HORIZONTAL;
    }

    // Determine Y
    float temp = object_h(obj) * scale_y;
    int y = 190 - temp - (object_h(obj) - temp) / 2;

    // Render shadow object twice with different offsets, so that
    // the shadows seem a bit blobbier and shadow-y
    for(int i = 0; i < 2; i++) {
        video_render_sprite_flip_scale_opacity_tint(
            obj->cur_sprite->data,
            x+i, y+i,
            BLEND_ALPHA,
            obj->pal_offset,
            flipmode,
            scale_y,
            50,
            color_create(0,0,0,255));
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
        object_set_vel(obj, vec2f_create(0,0));
    }
    if(obj->move != NULL) {
        obj->move(obj);
    }
}

// This does palette transformations to the WHOLE screen palette
// and affects all objects!
int object_scenewide_palette_transform(object *obj, screen_palette *pal) {
    player_sprite_state *rstate = &obj->sprite_state;
    if(rstate->pal_entry_count > 0 && rstate->duration > 0) {
        float bp = ((float)rstate->pal_begin) +
            ((float)rstate->pal_end - (float)rstate->pal_begin) *
            ((float)rstate->timer / (float)rstate->duration);

        color b;
        b.r = pal->data[rstate->pal_ref_index][0];
        b.g = pal->data[rstate->pal_ref_index][1];
        b.b = pal->data[rstate->pal_ref_index][2];

        uint8_t m;
        float u;
        float k = bp / 255.0f;
        for(int i = rstate->pal_start_index; i < rstate->pal_start_index + rstate->pal_entry_count; i++) {
            if(rstate->pal_tint) {
                m = max3(pal->data[i][0], pal->data[i][1], pal->data[i][2]);
                u = m / 255.0f;
                pal->data[i][0] = max2(0, min2(255, pal->data[i][0] + u * k * (b.r - pal->data[i][0])));
                pal->data[i][1] = max2(0, min2(255, pal->data[i][1] + u * k * (b.g - pal->data[i][1])));
                pal->data[i][2] = max2(0, min2(255, pal->data[i][2] + u * k * (b.b - pal->data[i][2])));
            } else {
                pal->data[i][0] = max2(0, min2(255, pal->data[i][0] * (1 - k) + (b.r * k)));
                pal->data[i][1] = max2(0, min2(255, pal->data[i][1] * (1 - k) + (b.g * k)));
                pal->data[i][2] = max2(0, min2(255, pal->data[i][2] * (1 - k) + (b.b * k)));
            }
        }
        return 1;
    }
    return 0;
}

// This does palette transformations to the WHOLE screen palette
// and affects all objects!
int object_palette_transform(object *obj, screen_palette *pal) {
    int transform_done = 0;
    transform_done |= object_scenewide_palette_transform(obj, pal);
    if(obj->pal_transform != NULL) {
        transform_done |= obj->pal_transform(obj, pal);
    }
    return transform_done;
}

/** Frees the object and all resources attached to it (even the animation, if it is owned by the object)
  * \param obj Object handle
  */
void object_free(object *obj) {
    if(obj->free != NULL) {
        obj->free(obj);
    }
    player_free(obj);
    if(obj->cur_animation_own == OWNER_OBJECT) {
        animation_free(obj->cur_animation);
        free(obj->cur_animation);
    }
    if (obj->custom_str) {
        free(obj->custom_str);
    }
    obj->cur_surface = NULL;
    obj->cur_animation = NULL;
}

/** Sets a pointer to a sound translation table. Note! Does NOT copy!
  * \param obj Object handle
  * \param ptr Pointer to the STL (30 byte char array)
  */
void object_set_stl(object *obj, char *ptr) {
    obj->sound_translation_table = ptr;
}

/** Returns a pointer to the sound translation table (30 byte char array)
  * \param obj Object handle
  * \return Pointer to the sound translation table
  */
char* object_get_stl(const object *obj) {
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
        free(obj->cur_animation);
    }
    if (obj->custom_str != NULL) {
        free(obj->custom_str);
    }
    obj->custom_str = NULL;
    obj->cur_animation = ani;
    obj->cur_animation_own = OWNER_EXTERNAL;
    player_reload(obj);

    // Debug texts
    if(obj->cur_animation->id == -1) {
        DEBUG("Custom object set to (x,y) = (%f,%f).",
            obj->pos.x, obj->pos.y);
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
    obj->custom_str = strcpy(malloc(strlen(str)+1), str);
    player_reload_with_str(obj, str);
    DEBUG("Set animation string to %s", str);
}

/** Returns a pointer to the currently playing animation
  * \param obj Object handle
  * \return animation* Pointer to current animation
  */
animation* object_get_animation(object *obj) {
    return obj->cur_animation;
}

/** Selects sprite to show. Note! Animation string will override this!
  * \param obj Object handle
  * \param id Sprite ID (starting from 0). Negative values will set sprite to nonexistent (NULL).
  */
void object_select_sprite(object *obj, int id) {
    if(obj == NULL) return;
    if(!obj->sprite_override) {
        if(id < 0) {
            obj->cur_sprite = NULL;
        } else {
            obj->cur_sprite = animation_get_sprite(obj->cur_animation, id);
            obj->sprite_state.blendmode = BLEND_ALPHA;
            obj->sprite_state.flipmode = FLIP_NONE;
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

void object_set_userdata(object *obj, void *ptr) { obj->userdata = ptr; }
void* object_get_userdata(const object *obj) { return obj->userdata; }
void object_set_free_cb(object *obj, object_free_cb cbfunc) { obj->free = cbfunc; }
void object_set_act_cb(object *obj, object_act_cb cbfunc) { obj->act = cbfunc; }
void object_set_static_tick_cb(object *obj, object_tick_cb cbfunc) { obj->static_tick = cbfunc; }
void object_set_dynamic_tick_cb(object *obj, object_tick_cb cbfunc) { obj->dynamic_tick = cbfunc; }
void object_set_collide_cb(object *obj, object_collide_cb cbfunc) { obj->collide = cbfunc; }
void object_set_finish_cb(object *obj, object_finish_cb cbfunc) { obj->finish = cbfunc; }
void object_set_move_cb(object *obj, object_move_cb cbfunc) { obj->move = cbfunc; }
void object_set_debug_cb(object *obj, object_debug_cb cbfunc) { obj->debug = cbfunc; }
void object_set_serialize_cb(object *obj, object_serialize_cb cbfunc) { obj->serialize = cbfunc; }
void object_set_unserialize_cb(object *obj, object_unserialize_cb cbfunc) { obj->unserialize = cbfunc; }
void object_set_pal_transform_cb(object *obj, object_palette_transform_cb cbfunc) { obj->pal_transform = cbfunc; }

void object_set_layers(object *obj, int layers) { obj->layers = layers; }
void object_set_group(object *obj, int group) { obj->group = group; }
void object_set_gravity(object *obj, float gravity) { obj->gravity = gravity; }

float object_get_gravity(const object *obj) { return obj->gravity; }
int object_get_group(const object *obj) { return obj->group; }
int object_get_layers(const object *obj) { return obj->layers; }

void object_set_pal_offset(object *obj, int offset) { obj->pal_offset = offset; }
int object_get_pal_offset(const object *obj) { return obj->pal_offset; }

void object_set_halt_ticks(object *obj, int ticks) { obj->halt = (ticks > 0); obj->halt_ticks = ticks; }
int object_get_halt_ticks(object *obj) { return obj->halt_ticks; }

void object_set_halt(object *obj, int halt) { obj->halt = halt; obj->halt_ticks = (halt == 0 ? 0 : obj->halt_ticks); }
int object_get_halt(const object *obj) { return obj->halt; }

void object_set_repeat(object *obj, int repeat) { player_set_repeat(obj, repeat); }
int object_get_repeat(const object *obj) { return player_get_repeat(obj); }
int object_finished(object *obj) { return obj->animation_state.finished; }
void object_set_direction(object *obj, int dir) { obj->direction = dir; }
int object_get_direction(const object *obj) { return obj->direction; }

void object_set_shadow(object *obj, int enable) { obj->cast_shadow = enable; }
int object_get_shadow(const object *obj) { return obj->cast_shadow; }

int object_w(const object *obj) { return object_get_size(obj).x; }
int object_h(const object *obj) { return object_get_size(obj).y; }
int object_px(const object *obj) { return vec2f_to_i(obj->pos).x; }
int object_py(const object *obj) { return vec2f_to_i(obj->pos).y; }
float object_vx(const object *obj) { return obj->vel.x; }
float object_vy(const object *obj) { return obj->vel.y; }

void object_set_px(object *obj, int val) { obj->pos.x = val; }
void object_set_py(object *obj, int val) { obj->pos.y = val; }
void object_set_vx(object *obj, float val) { obj->vel.x = val; }
void object_set_vy(object *obj, float val) { obj->vel.y = val; }

vec2i object_get_pos(const object *obj) { return vec2f_to_i(obj->pos); }
vec2f object_get_vel(const object *obj) { return obj->vel; }
void object_set_pos(object *obj, vec2i pos) { obj->pos = vec2i_to_f(pos); }
void object_set_vel(object *obj, vec2f vel) { obj->vel = vel; }

vec2i object_get_size(const object *obj) {
    if(obj->cur_sprite != NULL) {
        return sprite_get_size(obj->cur_sprite);
    }
    return vec2i_create(0,0);
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
    obj->attached_to = attach_to;
}
