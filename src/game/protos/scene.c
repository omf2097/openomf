#include <stdlib.h>
#include "game/protos/scene.h"
#include "video/video.h"
#include "resources/ids.h"
#include "resources/bk_loader.h"
#include "utils/log.h"
#include "utils/vec.h"
#include "game/game_player.h"
#include "game/game_state_type.h"
#include "resources/af_loader.h"

// Some internal functions
void cb_scene_spawn_object(object *parent, int id, vec2i pos, int g, void *userdata);
void cb_scene_destroy_object(object *parent, int id, void *userdata);

// Loads BK file etc.
int scene_create(scene *scene, game_state *gs, int scene_id) {
    // Load BK
    if(scene_id == SCENE_NONE || load_bk_file(&scene->bk_data, scene_id)) {
        PERROR("Unable to load BK file %s (%d)!", get_id_name(scene_id), scene_id);
        return 1;
    }
    scene->id = scene_id;
    scene->gs = gs;
    scene->af_data[0] = NULL;
    scene->af_data[1] = NULL;

    // Init functions
    scene->userdata = NULL;
    scene->free = NULL;
    scene->event = NULL;
    scene->render = NULL;
    scene->render_overlay = NULL;
    scene->tick = NULL;
    scene->input_poll = NULL;
    scene->startup = NULL;

    // Set palette
    video_set_base_palette(bk_get_palette(&scene->bk_data, 0));

    // All done.
    DEBUG("Loaded BK file %s (%d).", get_id_name(scene_id), scene_id);
    return 0;
}

void har_fix_sprite_coords(animation *ani, int fix_x, int fix_y) {
    iterator it;
    sprite *s;
    // Fix sprite positions
    vector_iter_begin(&ani->sprites, &it);
    while((s = iter_next(&it)) != NULL) {
        s->pos.x += fix_x;
        s->pos.y += fix_y;
    }
    // Fix collisions coordinates
    collision_coord *c;
    vector_iter_begin(&ani->collision_coords, &it);
    while((c = iter_next(&it)) != NULL) {
        c->pos.x += fix_x;
        c->pos.y += fix_y;
    }
}

int scene_load_har(scene *scene, int player_id, int har_id) {
    if (scene->af_data[player_id]) {
        af_free(scene->af_data[player_id]);
        free(scene->af_data[player_id]);
    }

    scene->af_data[player_id] = malloc(sizeof(af));

    if(load_af_file(scene->af_data[player_id], har_id)) {
        PERROR("Unable to load HAR %s (%d)!", get_id_name(har_id), har_id);
        return 1;
    }

    // Fix some coordinates on jump sprites
    har_fix_sprite_coords(&af_get_move(scene->af_data[player_id], ANIM_JUMPING)->ani, 0, -50);

    return 0;
}

void scene_init(scene *scene) {
    // Init background sprite with palette
    object_create(&scene->background, scene->gs, vec2i_create(0,0), vec2f_create(0,0));
    animation *bgani = create_animation_from_single(sprite_copy(&scene->bk_data.background), vec2i_create(0,0));
    object_set_animation(&scene->background, bgani);
    object_set_animation_owner(&scene->background, OWNER_OBJECT);
    object_select_sprite(&scene->background, 0);
    object_set_palette(&scene->background, bk_get_palette(&scene->bk_data, 0), 0);

    // init shadow buffer
    image_create(&scene->shadow_buffer_img, 320, 200);

    // Bootstrap animations
    iterator it;
    hashmap_iter_begin(&scene->bk_data.infos, &it);
    hashmap_pair *pair = NULL;
    while((pair = iter_next(&it)) != NULL) {
        bk_info *info = (bk_info*)pair->val;
        if(info->load_on_start == 255 || info->probability == 1 || scene_startup(scene, info->ani.id)) {
            object *obj = malloc(sizeof(object));
            object_create(obj, scene->gs, info->ani.start_pos, vec2f_create(0,0));
            object_set_stl(obj, scene->bk_data.sound_translation_table);
            object_set_palette(obj, bk_get_palette(&scene->bk_data, 0), 0);
            object_set_animation(obj, &info->ani);
            if(info->probability == 1) {
                object_set_repeat(obj, 1);
            }
            object_set_spawn_cb(obj, cb_scene_spawn_object, (void*)scene);
            object_set_destroy_cb(obj, cb_scene_destroy_object, (void*)scene);
            game_state_add_object(scene->gs, obj, RENDER_LAYER_BOTTOM);
            DEBUG("Scene bootstrap: Animation %d started.", info->ani.id);
        }
    }

    // init tick timer
    ticktimer_init(&scene->tick_timer);
}

/*
 * Serializes the scene to a buffer. Should return 1 on error, 0 on success
 * This will call the specialized scenes, (eg. arena) for their 
 * serialization data. 
 */
int scene_serialize(scene *s, serial *ser) {
    game_state_serialize(s->gs, ser);

    // Return success
    return 0;
}

/* 
 * Unserializes the data from buffer to a specialized object. 
 * Should return 1 on error, 0 on success.
 * Serial reder position should be set to correct position before calling this.
 */
int scene_unserialize(scene *s, serial *ser) {
    // TODO: Read attributes
    s->id = serial_read_int8(ser);

    // Return success
    return 0;
}

void scene_set_userdata(scene *scene, void *userdata) {
    scene->userdata = userdata;
}

void* scene_get_userdata(scene *scene) {
    return scene->userdata;
}

int scene_startup(scene *scene, int id) {
    if(scene->startup != NULL) {
        return scene->startup(scene, id);
    }
    return 0;
}

// Return 0 if event was handled here
int scene_event(scene *scene, SDL_Event *event) {
    if(scene->event != NULL) {
        return scene->event(scene, event);
    }
    return 1;
}

void scene_render_overlay(scene *scene) {
    if(scene->render_overlay != NULL) {
        scene->render_overlay(scene);
    }
}

void scene_render(scene *scene) {
    object_render_neutral(&scene->background);

    if(scene->render != NULL) {
        scene->render(scene);
    }
}

void scene_render_shadows(scene *scene) {
    // draw shadows
    surface_create_from_image(&scene->shadow_buffer_surface, &scene->shadow_buffer_img);
    video_render_sprite(&scene->shadow_buffer_surface, 0, 0, BLEND_ALPHA_FULL);
    surface_free(&scene->shadow_buffer_surface);
    image_clear(&scene->shadow_buffer_img, color_create(0,0,0,0));
    
}

void scene_tick(scene *scene) {
    // Tick timers
    ticktimer_run(&scene->tick_timer);

    if(scene->tick != NULL) {
        scene->tick(scene);
    }
}

void scene_input_poll(scene *scene) {
    if(scene->input_poll != NULL) {
        scene->input_poll(scene);
    }
}

void scene_free(scene *scene) {
    if(scene->free != NULL) {
        scene->free(scene);
    }
    bk_free(&scene->bk_data);
    if (scene->af_data[0]) {
        af_free(scene->af_data[0]);
        free(scene->af_data[0]);
    }
    if (scene->af_data[1]) {
        af_free(scene->af_data[1]);
        free(scene->af_data[1]);
    }
    object_free(&scene->background);
    image_free(&scene->shadow_buffer_img);
    ticktimer_close(&scene->tick_timer);
}

void scene_set_free_cb(scene *scene, scene_free_cb cbfunc) {
    scene->free = cbfunc;
}

void scene_set_event_cb(scene *scene, scene_event_cb cbfunc) {
    scene->event = cbfunc;
}

void scene_set_render_cb(scene *scene, scene_render_cb cbfunc) {
    scene->render = cbfunc;
}

void scene_set_render_overlay_cb(scene *scene, scene_render_overlay_cb cbfunc) {
    scene->render_overlay = cbfunc;
}

void scene_set_startup_cb(scene *scene, scene_startup_cb cbfunc) {
    scene->startup = cbfunc;
}

void scene_set_tick_cb(scene *scene, scene_tick_cb cbfunc) {
    scene->tick = cbfunc;
}

void scene_set_input_poll_cb(scene *scene, scene_tick_cb cbfunc) {
    scene->input_poll = cbfunc;
}

void cb_scene_spawn_object(object *parent, int id, vec2i pos, int g, void *userdata) {
    scene *s = (scene*)userdata;

    // Get next animation
    bk_info *info = bk_get_info(&s->bk_data, id);
    if(info != NULL) {
        object *obj = malloc(sizeof(object));
        object_create(obj, parent->gs, vec2i_add(pos, info->ani.start_pos), vec2f_create(0,0));
        object_set_stl(obj, object_get_stl(parent));
        object_set_palette(obj, object_get_palette(parent), 0);
        object_set_animation(obj, &info->ani);
        object_set_spawn_cb(obj, cb_scene_spawn_object, userdata);
        object_set_destroy_cb(obj, cb_scene_destroy_object, userdata);
        if(info->probability == 1) {
            object_set_repeat(obj, 1);
        }

        if(object_get_layers(parent) & LAYER_HAZARD) {
            DEBUG("spawning hazard child");
            object_set_layers(obj, LAYER_HAZARD|LAYER_HAR);
            object_set_group(obj, GROUP_PROJECTILE);
            object_set_userdata(obj, object_get_userdata(parent));
            if (s->bk_data.file_id == 128 && id == 14) {
                // XXX hack because we don't understand the ms and md tags
                // without this, the 'bullet damage' sprite in the desert spawns at 0,0
                obj->pos = parent->pos;
            }
        }
        game_state_add_object(parent->gs, obj, RENDER_LAYER_BOTTOM);
    }
}

void cb_scene_destroy_object(object *parent, int id, void *userdata) {
    game_state_del_animation(parent->gs, id);
}
