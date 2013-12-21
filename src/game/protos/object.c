#include <stdlib.h>
#include <string.h>
#include <shadowdive/vga_image.h>
#include <shadowdive/sprite_image.h>
#include <shadowdive/sprite.h>
#include "game/protos/object.h"
#include "game/protos/object_specializer.h"
#include "game/game_state_type.h"
#include "video/video.h"
#include "utils/log.h"

#define UNUSED(x) (void)(x)

void object_create(object *obj, game_state *gs, vec2i pos, vec2f vel) {
    // State
    obj->gs = gs;

    // Position related
    obj->pos = vec2i_to_f(pos);
    obj->vel = vel;
    object_reset_vstate(obj);
    object_reset_hstate(obj);
    obj->direction = OBJECT_FACE_RIGHT;
    obj->y_percent = 1.0;

    // Physics
    obj->is_static = 0;
    obj->layers = OBJECT_DEFAULT_LAYER;
    obj->group = OBJECT_NO_GROUP;
    obj->gravity = 0.0f;

    // Animation playback related
    obj->cur_animation_own = OWNER_EXTERNAL;
    obj->texture_refresh = 0;
    obj->cur_palette = NULL;
    obj->cur_animation = NULL;
    obj->cur_sprite = NULL;
    obj->sound_translation_table = NULL;
    obj->cur_texture = NULL;
    obj->cur_remap = 0;
    obj->halt = 0;
    obj->stride = 1;
    obj->cast_shadow = 0;
    player_create(obj);

    // Callbacks & userdata
    obj->userdata = NULL;
    obj->tick = NULL;
    obj->free = NULL;
    obj->act = NULL;
    obj->collide = NULL;
    obj->finish = NULL;
    obj->move = NULL;
    obj->serialize = NULL;
    obj->unserialize = NULL;
    obj->debug = NULL;
}

/*
 * Serializes the object to a buffer. Should return 1 on error, 0 on success
 * This will call the specialized objects, eg. har or projectile for their 
 * serialization data. 
 */
int object_serialize(object *obj, serial *ser) {
    // TODO: Write other attributes too
    serial_write_float(ser, obj->pos.x);
    serial_write_float(ser, obj->pos.y);
    serial_write_float(ser, obj->vel.x);
    serial_write_float(ser, obj->vel.y);

    // Serialize the underlying object
    if(obj->serialize != NULL) {
        obj->serialize(obj, ser);
    }

    // Return success
    return 0;
}

/* 
 * Unserializes the data from buffer to a specialized object. 
 * Should return 1 on error, 0 on success.
 * Serial reder position should be set to correct position before calling this.
 */
int object_unserialize(object *obj, serial *ser) {
    // TODO: Read object_t data
    obj->pos.x = serial_read_float(ser);
    obj->pos.y = serial_read_float(ser);
    obj->vel.x = serial_read_float(ser);
    obj->vel.y = serial_read_float(ser);

    // Read the specialization ID from ther serial "stream".
    // This should be an int.
    int specialization_id = serial_read_int(ser);

    // This should automatically bootstrap the object so that it has at least
    // unserialize function callback and local memory allocated
    object_auto_specialize(obj, specialization_id);
    
    // Now, if the object has unserialize function, call it with
    // serialization data. serial object should be pointing to the 
    // start of that data.
    if(obj->unserialize != NULL) {
        obj->unserialize(obj, ser);
    }

    // Return success
    return 0;
}

void object_set_stride(object *obj, int stride) {
    if(stride < 1) {
        stride = 1;
    }
    obj->stride = stride;
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

void object_tick(object *obj) {
    if(obj->cur_animation != NULL && obj->halt == 0) {
        for(int i = 0; i < obj->stride; i++)
            player_run(obj);
    }
    if(obj->tick != NULL) {
        obj->tick(obj);
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

void object_revalidate(object *obj) {
    obj->texture_refresh = 1;
}

int _max(int r, int g, int b) {
    int max = r;
    if(g > max) max = g;
    if(b > max) max = b;
    return max;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

int min(int a, int b) {
    return (a > b) ? b : a;
}

void object_check_texture(object *obj) {
    // (Re)generate texture if necessary
    if(obj->cur_texture == NULL) {
        obj->cur_texture = malloc(sizeof(texture));
        texture_create(obj->cur_texture);
        obj->texture_refresh = 1;
    }

    // Check if we need to do palette stuff on every tick
    player_sprite_state *rstate = &obj->sprite_state;
    if(rstate->pal_entry_count > 0 && rstate->duration > 0) {
        obj->texture_refresh = 1;
    }

    // If palette has changed, load up new texture here
    if(obj->texture_refresh) {
        // Load up sprite with defined palette
        sd_rgba_image *img = sd_vga_image_decode(
            obj->cur_sprite->raw_sprite, 
            (sd_palette*)obj->cur_palette, 
            obj->cur_remap);

        // If texture size differs, free it here
        if(obj->cur_texture->w != img->w || obj->cur_texture->h != img->h) {
            texture_free(obj->cur_texture);
        }
/*
        // Do palette tricks
        if(rstate->pal_entry_count > 0 && rstate->duration > 0) {
            float bp = rstate->pal_begin + 
                ((float)rstate->pal_end - (float)rstate->pal_begin) * 
                (float)rstate->timer / (float)rstate->duration;
            sd_vga_image *vga = obj->cur_sprite->raw_sprite;
            int pix = 0;
            float m = 0;
            int r = obj->cur_palette->data[rstate->pal_ref_index][0];
            int g = obj->cur_palette->data[rstate->pal_ref_index][1];
            int b = obj->cur_palette->data[rstate->pal_ref_index][2];
            color s;
            for(int y = 0; y < img->h; y++) {
                for(int x = 0; x < img->w; x++) {
                    pix = vga->data[y * img->w + x];
                    if(img->data[(y * img->w + x)*4 + 3] == 0) continue;
                    if(pix >= rstate->pal_start_index && 
                       pix <= (rstate->pal_start_index + rstate->pal_entry_count)) {
                        s.r = img->data[(y * img->w + x)*4 + 0];
                        s.g = img->data[(y * img->w + x)*4 + 1];
                        s.b = img->data[(y * img->w + x)*4 + 2];
                        float rr,gr,br;
                        if(rstate->pal_tint) {
                            m = _max(s.r, s.g, s.b);
                            rr = (float)s.r + m/64.0f + bp/64.0f + (float)(r - s.r);
                            gr = (float)s.g + m/64.0f + bp/64.0f + (float)(g - s.g);
                            br = (float)s.b + m/64.0f + bp/64.0f + (float)(b - s.b);
                        } else {
                            rr = (float)r * (float)bp/64.0f;
                            gr = s.g * (float)(1 - bp/64.0f) + (float)g * bp/64.0f;
                            br = s.b * (float)(1 - bp/64.0f) + (float)b * bp/64.0f;
                        }
                        img->data[(y * img->w + x)*4 + 0] = max(0, min(63, rr));
                        img->data[(y * img->w + x)*4 + 1] = max(0, min(63, gr));
                        img->data[(y * img->w + x)*4 + 2] = max(0, min(63, br));
                    }
                }
            }
        }*/

        // If texture is valid, just reupload. We checked for size similarity previously.
        // If texture is NOT valid, re-create it with a free ID etc.
        if(texture_is_valid(obj->cur_texture)) {
            if(texture_upload(obj->cur_texture, img->data)) {
                PERROR("object_render: Error while uploading to an existing texture!");
            }
        } else {
            if(texture_init(obj->cur_texture, img->data, img->w, img->h)) {
                PERROR("object_render: Error while creating texture!");
            }
        }

        // Delete resources
        sd_rgba_image_delete(img);
        obj->texture_refresh = 0;
    }
}

void object_render(object *obj) {
    // Stop here if cur_sprite is NULL
    if(obj->cur_sprite == NULL) return;

    // Something to ease the pain ...
    player_sprite_state *rstate = &obj->sprite_state;

    // Make sure texture is valid etc.
    object_check_texture(obj);

    // Render
    int y = obj->pos.y + obj->cur_sprite->pos.y;
    int x = obj->pos.x + obj->cur_sprite->pos.x;
    if(object_get_direction(obj) == OBJECT_FACE_LEFT) {
        x = obj->pos.x - obj->cur_sprite->pos.x - object_get_size(obj).x;
    }
    int flipmode = rstate->flipmode;
    if(obj->direction == OBJECT_FACE_LEFT) {
        flipmode ^= FLIP_HORIZONTAL;
    }

    // Some interesting stuff
    if(rstate->duration > 0) {
        float moment = rstate->timer / rstate->duration;
        float b = (rstate->blend_start) 
            ? (rstate->blend_start + (rstate->blend_finish - rstate->blend_start) * moment)
            : rstate->blend_finish;
        UNUSED(b);
    }

    // Render
    video_render_sprite_flip_scale(obj->cur_texture, x, y, rstate->blendmode, flipmode, obj->y_percent);
}

// Renders sprite to left top corner with no special stuff applied
void object_render_neutral(object *obj) {
    if(obj->cur_sprite == NULL) return;
    object_check_texture(obj);
    video_render_background(obj->cur_texture);
}

// Renders sprite's shadow to a shadow buffer
void object_render_shadow(object *obj, image *shadow_buffer) {
    if(obj->cur_sprite == NULL || !obj->cast_shadow) {
        return;
    }
    sd_vga_image *vga = (sd_vga_image*)obj->cur_sprite->raw_sprite;
    char *stencil = vga->stencil;
    player_sprite_state *rstate = &obj->sprite_state;
    int w = vga->w;
    int h = vga->h;
    int y = 190;
    int x = obj->pos.x + obj->cur_sprite->pos.x;
    int flipmode = rstate->flipmode;
    if(object_get_direction(obj) == OBJECT_FACE_LEFT) {
        x = obj->pos.x - obj->cur_sprite->pos.x - object_get_size(obj).x;
        flipmode ^= FLIP_HORIZONTAL;
    }

    ///TODO smarter code to make this less branchy on flipmode
    if (flipmode & FLIP_VERTICAL) {
        // only render every third line of the sprite, to emulate the shadow being cast onto the floor
        for (int i = 0; i < h; i+=3) {
            y--;
            for (int j = 0; j < w; j++) {

                if (stencil[(i*w)+j]) {
                    switch(flipmode) {
                        case FLIP_VERTICAL:
                            image_set_pixel(shadow_buffer, x+j, y, color_create(0,0,0,100));
                            break;
                        case FLIP_VERTICAL|FLIP_HORIZONTAL:
                            image_set_pixel(shadow_buffer, x+(w-j), y, color_create(0,0,0,100));
                            break;
                    }
                }
            }
        }
    } else {
        // only render every third line of the sprite, to emulate the shadow being cast onto the floor
        for (int i = h-1; i >= 0; i-=3) {
            y--;
            for (int j = 0; j < w; j++) {
                if (stencil[(i*w)+j]) {
                    switch(flipmode) {
                        case FLIP_NONE:
                            image_set_pixel(shadow_buffer, x+j, y, color_create(0,0,0,100));
                            break;
                        case FLIP_HORIZONTAL:
                            image_set_pixel(shadow_buffer, x+(w-j), y, color_create(0,0,0,100));
                            break;
                    }
                }
            }
        }
    }

}

void object_act(object *obj, int action) {
    if(obj->act != NULL) {
        obj->act(obj, action);
    }
}

void object_move(object *obj) {
    if(obj->move != NULL) {
        obj->move(obj);
    }
}

void object_free(object *obj) {
    if(obj->free != NULL) {
        obj->free(obj);
    }
    player_free(obj);
    if(obj->cur_animation_own == OWNER_OBJECT) {
        animation_free(obj->cur_animation);
        free(obj->cur_animation);
    }
    if(obj->cur_texture != NULL) {
        texture_free(obj->cur_texture);
        free(obj->cur_texture);
    }
    obj->cur_texture = NULL;
    obj->cur_animation = NULL;
}

void object_set_stl(object *obj, char *ptr) {
    obj->sound_translation_table = ptr;
}

char* object_get_stl(object *obj) {
    return obj->sound_translation_table;
}

void object_set_animation_owner(object *obj, int owner) {
    obj->cur_animation_own = owner;
}

void object_set_palette(object *obj, palette *pal, int remap) {
    obj->cur_palette = pal;
    obj->cur_remap = remap;
    obj->texture_refresh = 1;
}

palette* object_get_palette(object *obj) {
    return obj->cur_palette;
}

void object_set_animation(object *obj, animation *ani) {
    if(obj->cur_animation != NULL && obj->cur_animation_own == OWNER_OBJECT) {
        animation_free(obj->cur_animation);
        free(obj->cur_animation);
    }
    obj->cur_animation = ani;
    obj->cur_animation_own = OWNER_EXTERNAL;
    player_reload(obj);

    // Debug texts
    if(obj->cur_animation->id == -1) {
        DEBUG("Custom object set to (x,y) = (%f,%f).", 
            obj->pos.x, obj->pos.y);
    } else {
        DEBUG("Animation object %d set to (x,y) = (%f,%f) with \"%s\".", 
            obj->cur_animation->id,
            obj->pos.x, obj->pos.y,
            str_c(&obj->cur_animation->animation_string));
    }
}

void object_set_custom_string(object *obj, const char *str) {
    player_reload_with_str(obj, str);
    DEBUG("Set animation string to %s", str);
}

animation* object_get_animation(object *obj) {
    return obj->cur_animation;
}

void object_select_sprite(object *obj, int id) {
    obj->cur_sprite = animation_get_sprite(obj->cur_animation, id);
    obj->texture_refresh = 1;
    obj->sprite_state.blendmode = BLEND_ALPHA;
    obj->sprite_state.flipmode = FLIP_NONE;
}

void object_set_userdata(object *obj, void *ptr) { obj->userdata = ptr; }
void* object_get_userdata(object *obj) { return obj->userdata; }
void object_set_free_cb(object *obj, object_free_cb cbfunc) { obj->free = cbfunc; }
void object_set_act_cb(object *obj, object_act_cb cbfunc) { obj->act = cbfunc; }
void object_set_tick_cb(object *obj, object_tick_cb cbfunc) { obj->tick = cbfunc; }
void object_set_collide_cb(object *obj, object_collide_cb cbfunc) { obj->collide = cbfunc; }
void object_set_finish_cb(object *obj, object_finish_cb cbfunc) { obj->finish = cbfunc; }
void object_set_move_cb(object *obj, object_move_cb cbfunc) { obj->move = cbfunc; }
void object_set_debug_cb(object *obj, object_debug_cb cbfunc) { obj->debug = cbfunc; }
void object_set_serialize_cb(object *obj, object_serialize_cb cbfunc) { obj->serialize = cbfunc; }
void object_set_unserialize_cb(object *obj, object_unserialize_cb cbfunc) { obj->unserialize = cbfunc; }

void object_set_layers(object *obj, int layers) { obj->layers = layers; }
void object_set_group(object *obj, int group) { obj->group = group; }
void object_set_gravity(object *obj, float gravity) { obj->gravity = gravity; }
void object_set_static(object *obj, int is_static) { obj->is_static = is_static; }

int object_is_static(object *obj) { return obj->is_static; }
int object_get_gravity(object *obj) { return obj->gravity; }
int object_get_group(object *obj) { return obj->group; }
int object_get_layers(object *obj) { return obj->layers; }

void object_reset_vstate(object *obj) {
    obj->hstate = (obj->vel.x < 0.01f && obj->vel.x > -0.01f) ? OBJECT_STABLE : OBJECT_MOVING;
}
void object_reset_hstate(object *obj) {
    obj->vstate = (obj->vel.y < 0.01f && obj->vel.y > -0.01f) ? OBJECT_STABLE : OBJECT_MOVING;
}

void object_set_halt(object *obj, int halt) { obj->halt = halt; }
int object_get_halt(object *obj) { return obj->halt; }

int object_get_vstate(object *obj) { return obj->vstate; }
int object_get_hstate(object *obj) { return obj->hstate; }

void object_set_repeat(object *obj, int repeat) { player_set_repeat(obj, repeat); }
int object_get_repeat(object *obj) { return player_get_repeat(obj); }
int object_finished(object *obj) { return obj->animation_state.finished; }
void object_set_direction(object *obj, int dir) { obj->direction = dir; }
int object_get_direction(object *obj) { return obj->direction; }

vec2i object_get_size(object *obj) {
    if(obj->cur_sprite != NULL) {
        return sprite_get_size(obj->cur_sprite);
    }
    return vec2i_create(0,0);
}

vec2i object_get_pos(object *obj) {
    return vec2f_to_i(obj->pos);
}

vec2f object_get_vel(object *obj) {
    return obj->vel;
}

void object_set_pos(object *obj, vec2i pos) {
    obj->pos = vec2i_to_f(pos);
}

void object_set_vel(object *obj, vec2f vel) {
    obj->vel = vel;
    object_reset_hstate(obj);
    object_reset_vstate(obj);
}

void object_set_spawn_cb(object *obj, object_state_add_cb cbf, void *userdata) {
    obj->animation_state.spawn = cbf;
    obj->animation_state.spawn_userdata = userdata;
}

void object_set_destroy_cb(object *obj, object_state_del_cb cbf, void *userdata) {
    obj->animation_state.destroy = cbf;
    obj->animation_state.destroy_userdata = userdata;
}
