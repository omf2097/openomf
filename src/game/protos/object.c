#include <stdlib.h>
#include <shadowdive/vga_image.h>
#include <shadowdive/sprite_image.h>
#include <shadowdive/sprite.h>
#include "game/protos/object.h"
#include "video/video.h"
#include "utils/log.h"

void object_create(object *obj, vec2i pos, vec2f vel) {
    // Position related
    obj->pos = vec2i_to_f(pos);
    obj->vel = vel;
    object_reset_vstate(obj);
    object_reset_hstate(obj);
    obj->direction = OBJECT_FACE_RIGHT;

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
    player_create(obj);

    // Callbacks & userdata
    obj->userdata = NULL;
    obj->tick = NULL;
    obj->free = NULL;
    obj->act = NULL;
    obj->collide = NULL;
    obj->finish = NULL;
    obj->move = NULL;
}

void object_tick(object *obj) {
    if(obj->cur_animation != NULL) {
        player_run(obj);
    }
    if(obj->tick != NULL) {
        obj->tick(obj);
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

void object_check_texture(object *obj) {
    // (Re)generate texture if necessary
    if(obj->cur_texture == NULL) {
        obj->cur_texture = malloc(sizeof(texture));
        texture_create(obj->cur_texture);
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
    if(obj->cur_sprite == NULL)  return;

    object_check_texture(obj);

    // Render
    int y = obj->pos.y + obj->cur_sprite->pos.y;
    int x = obj->pos.x + obj->cur_sprite->pos.x;
    if(object_get_direction(obj) == OBJECT_FACE_LEFT) {
        x = obj->pos.x - obj->cur_sprite->pos.x - object_get_size(obj).x;
    }
    int flipmode = obj->sprite_state.flipmode;
    if(obj->direction == OBJECT_FACE_LEFT) {
        flipmode ^= FLIP_HORIZONTAL;
    }
    video_render_sprite_flip(
        obj->cur_texture, 
        x, y,
        obj->sprite_state.blendmode,
        flipmode);
}

// Renders sprite to left top corner with no special stuff applied
void object_render_neutral(object *obj) {
    if(obj->cur_sprite == NULL)  return;
    object_check_texture(obj);
    video_render_background(obj->cur_texture);
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