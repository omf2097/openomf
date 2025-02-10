#include "game/protos/scene.h"
#include "game/game_player.h"
#include "game/game_state_type.h"
#include "resources/af_loader.h"
#include "resources/bk_loader.h"
#include "resources/ids.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/vec.h"
#include "video/vga_state.h"
#include "video/video.h"
#include <stdlib.h>

// Some internal functions
void cb_scene_spawn_object(object *parent, int id, vec2i pos, vec2f vel, uint8_t mp_flags, int s, int g,
                           void *userdata);
void cb_scene_destroy_object(object *parent, int id, void *userdata);

// Loads BK file etc.
int scene_create(scene *scene, game_state *gs, int scene_id) {
    if(scene_id == SCENE_NONE) {
        return 1;
    }

    // Load BK
    int resource_id = scene_to_resource(scene_id);
    scene->bk_data = omf_calloc(1, sizeof(bk));
    if(load_bk_file(scene->bk_data, resource_id)) {
        log_error("Unable to load scene %s (%s)!", scene_get_name(scene_id), get_resource_name(resource_id));
        return 1;
    }
    scene->id = scene_id;
    scene->gs = gs;
    scene->af_data[0] = NULL;
    scene->af_data[1] = NULL;
    scene->static_ticks_since_start = 0;

    // Init functions
    scene->userdata = NULL;
    scene->free = NULL;
    scene->event = NULL;
    scene->render = NULL;
    scene->render_overlay = NULL;
    scene->dynamic_tick = NULL;
    scene->static_tick = NULL;
    scene->input_poll = NULL;
    scene->startup = NULL;
    scene->prio_override = NULL;

    // Set base palette
    vga_state_set_base_palette_from(bk_get_palette(scene->bk_data, 0));
    vga_state_set_remaps_from(bk_get_remaps(scene->bk_data, 0));

    // Set menu colors to the correct position
    palette_set_menu_colors();

    // Index 0 is always black.
    vga_color c = {0, 0, 0};
    vga_state_set_base_palette_index(0, &c);

    // All done.
    log_debug("Loaded scene %s (%s).", scene_get_name(scene_id), get_resource_name(resource_id));
    return 0;
}

int scene_load_har(scene *scene, int player_id) {
    game_player *player = game_state_get_player(scene->gs, player_id);
    if(scene->af_data[player_id]) {
        af_free(scene->af_data[player_id]);
        omf_free(scene->af_data[player_id]);
    }
    scene->af_data[player_id] = omf_calloc(1, sizeof(af));

    int resource_id = har_to_resource(player->pilot->har_id);
    if(load_af_file(scene->af_data[player_id], resource_id)) {
        log_error("Unable to load HAR %s (%s)!", har_get_name(player->pilot->har_id), get_resource_name(resource_id));
        return 1;
    }

    log_debug("Loaded HAR %s (%s).", har_get_name(player->pilot->har_id), get_resource_name(resource_id));
    return 0;
}

void scene_init(scene *scene) {
    int m_load;
    int m_repeat;

    // Bootstrap animations
    iterator it;
    hashmap_iter_begin(&scene->bk_data->infos, &it);
    hashmap_pair *pair = NULL;
    foreach(it, pair) {
        bk_info *info = (bk_info *)pair->value;

        // Ask scene if this animation should be played on start
        scene_startup(scene, info->ani.id, &m_load, &m_repeat);

        // Start up animations
        if(m_load) {
            object *obj = omf_calloc(1, sizeof(object));
            object_create(obj, scene->gs, info->ani.start_pos, vec2f_create(0, 0));
            object_set_stl(obj, scene->bk_data->sound_translation_table);
            object_set_animation(obj, &info->ani);
            object_set_repeat(obj, m_repeat);
            object_set_spawn_cb(obj, cb_scene_spawn_object, (void *)scene);
            object_set_destroy_cb(obj, cb_scene_destroy_object, (void *)scene);
            int o_prio = scene_anim_prio_override(scene, info->ani.id);
            o_prio = (o_prio != -1) ? o_prio : RENDER_LAYER_BOTTOM;
            game_state_add_object(scene->gs, obj, o_prio, 0, 0);
            log_debug("Scene bootstrap: Animation %d started.", info->ani.id);
        }
    }

    // init tick timer
    ticktimer_init(&scene->tick_timer);
}

int scene_clone(scene *src, scene *dst, game_state *gs) {
    memcpy(dst, src, sizeof(scene));
    ticktimer_clone(&src->tick_timer, &dst->tick_timer);
    dst->gs = gs;
    if(src->clone) {
        src->clone(src, dst);
    }
    return 0;
}

int scene_clone_free(scene *sc) {
    if(sc->clone_free) {
        sc->clone_free(sc);
    }
    ticktimer_close(&sc->tick_timer);
    omf_free(sc);
    return 0;
}

void scene_set_userdata(scene *scene, void *userdata) {
    scene->userdata = userdata;
}

void *scene_get_userdata(const scene *scene) {
    return scene->userdata;
}

void scene_startup(scene *scene, int id, int *m_load, int *m_repeat) {
    *m_load = 0;
    *m_repeat = 0;
    if(scene->startup != NULL) {
        scene->startup(scene, id, m_load, m_repeat);
    }
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
    video_draw(&scene->bk_data->background, 0, 0);

    if(scene->render != NULL) {
        scene->render(scene);
    }
}

int scene_anim_prio_override(scene *scene, int anim_id) {
    if(scene->prio_override != NULL) {
        return scene->prio_override(scene, anim_id);
    }
    return -1;
}

void scene_static_tick(scene *scene, int paused) {
    scene->static_ticks_since_start++;
    if(scene->static_tick != NULL) {
        scene->static_tick(scene, paused);
    }
}

void scene_dynamic_tick(scene *scene, int paused) {
    // Tick timers
    if(!paused) {
        ticktimer_run(&scene->tick_timer, scene);
    }

    if(scene->dynamic_tick != NULL) {
        scene->dynamic_tick(scene, paused);
    }
}

void scene_input_poll(scene *scene) {
    if(scene->static_ticks_since_start < 25) {
        // Wait a bit at scene startup so that inputs from previous scene
        // don't bleed to this one.
        return;
    }
    if(scene->gs->this_id != scene->gs->next_id) {
        // Ignore inputs during scene fadeout
        return;
    }
    if(scene->input_poll != NULL) {
        scene->input_poll(scene);
    }
}

void scene_free(scene *scene) {
    if(scene->free != NULL) {
        scene->free(scene);
    }
    bk_free(scene->bk_data);
    omf_free(scene->bk_data);
    if(scene->af_data[0]) {
        af_free(scene->af_data[0]);
        omf_free(scene->af_data[0]);
    }
    if(scene->af_data[1]) {
        af_free(scene->af_data[1]);
        omf_free(scene->af_data[1]);
    }
    ticktimer_close(&scene->tick_timer);
}

void scene_set_free_cb(scene *scene, scene_free_cb cbfunc) {
    scene->free = cbfunc;
}

void scene_set_clone_free_cb(scene *scene, scene_clone_free_cb cbfunc) {
    scene->clone_free = cbfunc;
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

void scene_set_dynamic_tick_cb(scene *scene, scene_tick_cb cbfunc) {
    scene->dynamic_tick = cbfunc;
}

void scene_set_static_tick_cb(scene *scene, scene_tick_cb cbfunc) {
    scene->static_tick = cbfunc;
}

void scene_set_anim_prio_override_cb(scene *scene, scene_anim_prio_override_cb cbfunc) {
    scene->prio_override = cbfunc;
}

bool scene_is_arena(scene *scene) {
    return (scene->id >= SCENE_ARENA0 && scene->id <= SCENE_ARENA4);
}

void scene_set_input_poll_cb(scene *scene, scene_input_poll_cb cbfunc) {
    scene->input_poll = cbfunc;
}

void cb_scene_spawn_object(object *parent, int id, vec2i pos, vec2f vel, uint8_t mp_flags, int s, int g,
                           void *userdata) {
    scene *sc = (scene *)userdata;

    // Get next animation
    bk_info *info = bk_get_info(sc->bk_data, id);
    if(info != NULL) {
        object *obj = omf_calloc(1, sizeof(object));
        object_create(obj, parent->gs, vec2i_add(pos, info->ani.start_pos), vel);
        object_set_stl(obj, object_get_stl(parent));
        object_set_animation(obj, &info->ani);
        object_set_spawn_cb(obj, cb_scene_spawn_object, userdata);
        object_set_destroy_cb(obj, cb_scene_destroy_object, userdata);
        if(info->probability == 1) {
            object_set_repeat(obj, 1);
        }
        if(mp_flags & 0x1) {
            object_set_animation_effects(obj, EFFECT_SATURATE);
        }
        game_state_add_object(parent->gs, obj, RENDER_LAYER_BOTTOM, 0, 0);
    }
}

void cb_scene_destroy_object(object *parent, int id, void *userdata) {
    game_state_del_animation(parent->gs, id);
}
