#include <stdlib.h>
#include <math.h>
#include <SDL.h>
#include "controller/keyboard.h"
#include "controller/joystick.h"
#include "controller/rec_controller.h"
#include "utils/log.h"
#include "utils/miscmath.h"
#include "game/utils/serial.h"
#include "resources/ids.h"
#include "resources/pilots.h"
#include "console/console.h"
#include "video/video.h"
#include "video/tcache.h"
#include "formats/rec.h"
#include "formats/error.h"
#include "game/game_state.h"
#include "game/common_defines.h"
#include "game/utils/settings.h"
#include "game/utils/ticktimer.h"
#include "game/protos/scene.h"
#include "game/protos/object.h"
#include "game/protos/intersect.h"
#include "game/scenes/intro.h"
#include "game/scenes/mainmenu.h"
#include "game/scenes/credits.h"
#include "game/scenes/cutscene.h"
#include "game/scenes/arena.h"
#include "game/scenes/mechlab.h"
#include "game/scenes/newsroom.h"
#include "game/scenes/melee.h"
#include "game/scenes/vs.h"
#include "game/scenes/scoreboard.h"
#include "game/scenes/openomf.h"

#define MS_PER_OMF_TICK 10
#define MS_PER_OMF_TICK_SLOWEST 60

enum {
    TICK_DYNAMIC = 0,
    TICK_STATIC,
};

void _setup_rec_controller(game_state *gs, int player_id, sd_rec_file *rec);

// How long the scene waits after order to move to another scene
// Used for crossfades
#define FRAME_WAIT_TICKS 30

typedef struct {
    int layer; ///< Object rendering layer
    int persistent; ///< 1 if the object should keep alive across scene boundaries
    int singleton; ///< 1 if object should be the only representative of its animation ID
    object *obj;
} render_obj;

int game_state_create(game_state *gs, engine_init_flags *init_flags) {
    gs->run = 1;
    gs->paused = 0;
    gs->tick = 0;
    gs->int_tick = 0;
    gs->role = ROLE_CLIENT;
    gs->next_requires_refresh = 0;
    gs->net_mode = init_flags->net_mode;
    gs->speed = settings_get()->gameplay.speed + 5;
    gs->init_flags = init_flags;
    vector_create(&gs->objects, sizeof(render_obj));

    // For screen shake
    gs->screen_shake_horizontal = 0;
    gs->screen_shake_vertical = 0;

    // For momentary game speed switches
    gs->speed_slowdown_previous = 0;
    gs->speed_slowdown_time = -1;

    // Used for crossfades
    gs->next_wait_ticks = 0;
    gs->this_wait_ticks = 0;

    // Set up players
    gs->sc = malloc(sizeof(scene));
    for(int i = 0; i < 2; i++) {
        gs->players[i] = malloc(sizeof(game_player));
        game_player_create(gs->players[i]);
    }

    reconfigure_controller(gs);
    int nscene;
    if (strlen(init_flags->rec_file) > 0 && init_flags->record == 0) {
        sd_rec_file rec;
        sd_rec_create(&rec);
        int ret = sd_rec_load(&rec, init_flags->rec_file);
        if(ret != SD_SUCCESS) {
            PERROR("Unable to load recording %s.", init_flags->rec_file);
            goto error_0;
        }

        // hardcode arena 0 for now, although the arena ID resides in field 'L' of the REC file
        nscene = SCENE_ARENA0;
        DEBUG("playing recording file %s", init_flags->rec_file);
        if(scene_create(gs->sc, gs, nscene)) {
            PERROR("Error while loading scene %d.", nscene);
            goto error_0;
        }

        // set the HAR colors, pilot, har type
        for(int i = 0; i < 2; i++) {
            gs->players[i]->colors[0] = rec.pilots[i].info.color_3;
            gs->players[i]->colors[1] = rec.pilots[i].info.color_2;
            gs->players[i]->colors[2] = rec.pilots[i].info.color_1;
            gs->players[i]->har_id = HAR_JAGUAR + rec.pilots[i].info.har_id;
            gs->players[i]->pilot_id = rec.pilots[i].info.pilot_id;
        }

        // XXX use playback controller once it exista
        _setup_rec_controller(gs, 0, &rec);
        _setup_rec_controller(gs, 1, &rec);
        if(arena_create(gs->sc)) {
            PERROR("Error while creating arena scene.");
            goto error_1;
        }
    } else {
        // Select correct starting scene and load resources
         nscene = (init_flags->net_mode == NET_MODE_NONE ? SCENE_OPENOMF : SCENE_MENU);
        if(scene_create(gs->sc, gs, nscene)) {
            PERROR("Error while loading scene %d.", nscene);
            goto error_0;
        }
        if(init_flags->net_mode == NET_MODE_NONE) {
            if(openomf_create(gs->sc)) {
                PERROR("Error while creating intro scene.");
                goto error_1;
            }
        } else {
            // if connecting to the server or listening, jump straight to the menu
            if(mainmenu_create(gs->sc)) {
                PERROR("Error while creating menu scene.");
                goto error_1;
            }
        }
    }

    // Initialize scene
    scene_init(gs->sc);

    // All done
    gs->this_id = nscene;
    gs->next_id = nscene;
    return 0;

error_1:
    scene_free(gs->sc);
error_0:
    free(gs->sc);
    vector_free(&gs->objects);
    return 1;
}

/*
 * \param game_state gs Game state object
 * \param obj Object to add
 * \param layer Object layer (top, middle, bottom)
 * \param singleton Should object be the lone representative of the animation ID ?
 * \param persistent Should object keep active across scene boundaries ?
 */
int game_state_add_object(game_state *gs, object *obj, int layer, int singleton, int persistent) {
    render_obj o;
    o.obj = obj;
    o.layer = layer;
    o.singleton = singleton;
    o.persistent = persistent;
    animation *new_ani = object_get_animation(obj);
    if(singleton) {
        iterator it;
        render_obj *robj;
        vector_iter_begin(&gs->objects, &it);
        while((robj = iter_next(&it)) != NULL) {
            animation *ani = object_get_animation(robj->obj);
            if(ani != NULL && ani->id == new_ani->id && robj->singleton) {
                return 1;
            }
        }
    }
    vector_append(&gs->objects, &o);

#ifdef DEBUGMODE_STFU
    animation *ani = object_get_animation(obj);
    DEBUG("Added animation %i to game_state on layer %d.", ani->id, layer);
#endif
    return 0;
}

/*
 * Slows down the game for n ticks. Only allows slowdown if one isn't already ongoing.
 */
void game_state_slowdown(game_state *gs, int ticks, int rate) {
    if(gs->speed_slowdown_time < 0) {
        gs->speed_slowdown_previous = gs->speed;
        gs->speed_slowdown_time = ticks;
        gs->speed = max2(rate, 0);
    }
}

void game_state_set_speed(game_state *gs, int rate) {
    gs->speed = max2(rate, 0);
    DEBUG("game speed set to %d", gs->speed);
}

unsigned int game_state_get_speed(game_state *gs) {
    return gs->speed;
}

void game_state_del_animation(game_state *gs, int anim_id) {
    iterator it;
    render_obj *robj;
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        animation *ani = object_get_animation(robj->obj);
        if(ani != NULL && ani->id == anim_id) {
            object_free(robj->obj);
            free(robj->obj);
            vector_delete(&gs->objects, &it);
            DEBUG("Deleted animation %i from game_state.", anim_id);
            return;
        }
    }
    DEBUG("Attempted to delete animation %i from game_state, but no such animation was playing.", anim_id);
}

void game_state_del_object(game_state *gs, object *target) {
    iterator it;
    render_obj *robj;
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        if(target == robj->obj) {
            object_free(robj->obj);
            free(robj->obj);
            vector_delete(&gs->objects, &it);
            return;
        }
    }
}

void game_state_get_projectiles(game_state *gs, vector *obj_proj) {
    iterator it;
    render_obj *robj;
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        if(object_get_layers(robj->obj) & LAYER_PROJECTILE) {
            vector_append(obj_proj, &robj->obj);
        }
    }
}

void game_state_clear_hazards_projectiles(game_state *gs) {
    iterator it;
    render_obj *robj;
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        if(object_get_group(robj->obj) == GROUP_PROJECTILE) {
            object_free(robj->obj);
            free(robj->obj);
            vector_delete(&gs->objects, &it);
        }
    }
}

void game_state_set_next(game_state *gs, unsigned int next_scene_id) {
    if(gs->next_wait_ticks <= 0) {
        gs->next_wait_ticks = FRAME_WAIT_TICKS;
        gs->next_next_id = SCENE_MENU;
        gs->next_id = next_scene_id;
    }
}

scene* game_state_get_scene(game_state *gs) {
    return gs->sc;
}

unsigned int game_state_is_running(game_state *gs) {
    return gs->run;
}

unsigned int game_state_is_paused(game_state *gs) {
    return gs->paused;
}

void game_state_set_paused(game_state *gs, unsigned int paused) {
    gs->paused = paused;
}

// Return 0 if event was handled here
int game_state_handle_event(game_state *gs, SDL_Event *event) {
    if(scene_event(gs->sc, event) == 0) {
        return 0;
    }
    return 1;
}

void game_state_render(game_state *gs) {
    iterator it;
    render_obj *robj;

    // Do palette transformations
    screen_palette *scr_pal = video_get_pal_ref();
    int pal_changed = 0;
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        if(object_palette_transform(robj->obj, scr_pal) == 1) {
            pal_changed = 1;
            gs->next_requires_refresh = 1;
        }
    }

    // If changes were made to palette, then
    // all resources that depend on it must be redrawn.
    // This will take care of it.
    if(pal_changed) {
        scr_pal->version++;
    } else if(gs->next_requires_refresh) {
        // Because of caching, we might sometimes get stuck to
        // a bad/old frame. This will fix it.
        scr_pal->version++;
        gs->next_requires_refresh = 0;
    }

    // Render scene background
    scene_render(gs->sc);

    // Get har objects
    object *har[2];
    har[0] = game_state_get_player(gs, 0)->har;
    har[1] = game_state_get_player(gs, 1)->har;

    // Render BOTTOM layer
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        if(robj->layer == RENDER_LAYER_BOTTOM) {
            if(robj->obj == har[0] || robj->obj == har[1])
                continue;
            object_render(robj->obj);
        }
    }

    // cast object shadows (scrap, projectiles, etc)
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        object_render_shadow(robj->obj);
    }

    // Render passive HARs here
    for(int i = 0; i < 2; i++) {
        if(har[i] != NULL && !har_is_active(har[i])) {
            object_render(har[i]);
        }
    }

    // Render MIDDLE layer
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        if(robj->layer == RENDER_LAYER_MIDDLE) {
            if(robj->obj == har[0] || robj->obj == har[1])
                continue;
            object_render(robj->obj);
        }
    }

    // Render active HARs here
    for(int i = 0; i < 2; i++) {
        if(har[i] != NULL && har_is_active(har[i])) {
            object_render(har[i]);
        }
    }

    // Render TOP layer
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        if(robj->layer == RENDER_LAYER_TOP) {
            if(robj->obj == har[0] || robj->obj == har[1])
                continue;
            object_render(robj->obj);
        }
    }

    // Render scene overlay (menus, etc.)
    scene_render_overlay(gs->sc);
}

void game_state_debug(game_state *gs) {
    // If we are in debug mode, handle HAR debug layers
#ifdef DEBUGMODE
    for(int i = 0; i < 2; i++) {
        object *h = game_state_get_player(gs, i)->har;
        if(h != NULL) {
            object_debug(h);
        }
    }
#endif
}

int game_load_new(game_state *gs, int scene_id) {
    // Free old scene
    scene_free(gs->sc);
    free(gs->sc);

    // Clear up old video cache objects
    tcache_clear();

    // Remove old objects
    render_obj *robj;
    iterator it;
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        if(!robj->persistent) {
            object_free(robj->obj);
            free(robj->obj);
            vector_delete(&gs->objects, &it);
        }
    }

    // Initialize new scene with BK data etc.
    gs->sc = malloc(sizeof(scene));
    if(scene_create(gs->sc, gs, scene_id)) {
        PERROR("Error while loading scene %d.", scene_id);
        goto error_0;
    }

    // Load scene specifics
    switch(scene_id) {
        case SCENE_OPENOMF:
            if(openomf_create(gs->sc)) {
                PERROR("Error while creating openomf-intro scene.");
                goto error_1;
            }
            break;
        case SCENE_INTRO:
            if(intro_create(gs->sc)) {
                PERROR("Error while creating intro scene.");
                goto error_1;
            }
            break;
        case SCENE_MENU:
            if(mainmenu_create(gs->sc)) {
                PERROR("Error while creating mainmenu scene.");
                goto error_1;
            }
            break;
        case SCENE_SCOREBOARD:
            if(scoreboard_create(gs->sc)) {
                PERROR("Error while creating scoreboard scene.");
                goto error_1;
            }
            break;
        case SCENE_CREDITS:
            if(credits_create(gs->sc)) {
                PERROR("Error while creating credits scene.");
                goto error_1;
            }
            break;
        case SCENE_MELEE:
            if(melee_create(gs->sc)) {
                PERROR("Error while creating melee scene.");
                goto error_1;
            }
            break;
        case SCENE_VS:
            if(vs_create(gs->sc)) {
                PERROR("Error while creating VS scene.");
                goto error_1;
            }
            break;
        case SCENE_MECHLAB:
            if(mechlab_create(gs->sc)) {
                PERROR("Error while creating Mechlab scene.");
                goto error_1;
            }
            break;
        case SCENE_NEWSROOM:
            if(newsroom_create(gs->sc)) {
                PERROR("Error while creating Newsroom scene.");
                goto error_1;
            }
            break;
        case SCENE_ARENA0:
        case SCENE_ARENA1:
        case SCENE_ARENA2:
        case SCENE_ARENA3:
        case SCENE_ARENA4:
            if(arena_create(gs->sc)) {
                PERROR("Error while creating arena scene.");
                goto error_1;
            }
            break;
        default:
            if(cutscene_create(gs->sc)) {
                PERROR("Error while creating cut scene.");
                goto error_1;
            }
            break;
    }

    // Zap scene to produce objects & background
    scene_init(gs->sc);

    // All done.
    gs->this_id = scene_id;
    gs->next_id = scene_id;
    gs->tick = 0;
    return 0;

error_1:
    scene_free(gs->sc);
error_0:
    free(gs->sc);
    return 1;
}

void game_state_call_collide(game_state *gs) {
    object *a, *b;
    unsigned int size = vector_size(&gs->objects);
    for(int i = 0; i < size; i++) {
        a = ((render_obj*)vector_get(&gs->objects, i))->obj;
        for(int k = i+1; k < size; k++) {
            b = ((render_obj*)vector_get(&gs->objects, k))->obj;
            if(a->group != b->group || a->group == OBJECT_NO_GROUP || b->group == OBJECT_NO_GROUP) {
                if(a->layers & b->layers) {
                    object_collide(a, b);
                }
            }
        }
    }
}

void game_state_cleanup(game_state *gs) {
    render_obj *robj;
    iterator it;
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        if(object_finished(robj->obj)) {
            /*DEBUG("Animation object %d is finished, removing.", robj->obj->cur_animation->id);*/
            object_free(robj->obj);
            free(robj->obj);
            vector_delete(&gs->objects, &it);
        }
    }
}

void game_state_call_move(game_state *gs) {
    render_obj *robj;
    iterator it;
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        object_move(robj->obj);
    }
}

void game_state_tick_controllers(game_state *gs) {
    for(int i = 0; i < game_state_num_players(gs); i++) {
        game_player *gp = game_state_get_player(gs, i);
        controller *c = game_player_get_ctrl(gp);
        if(c) {
            controller_tick(c, gs->int_tick, &c->extra_events);
        }
    }
}

void game_state_dyntick_controllers(game_state *gs) {
    for(int i = 0; i < game_state_num_players(gs); i++) {
        game_player *gp = game_state_get_player(gs, i);
        controller *c = game_player_get_ctrl(gp);
        if(c) {
            controller_dyntick(c, gs->tick, &c->extra_events);
        }
    }
}


void game_state_ctrl_events_free(game_state *gs) {
    for(int i = 0; i < game_state_num_players(gs); i++) {
        game_player *gp = game_state_get_player(gs, i);
        controller *c = game_player_get_ctrl(gp);
        if(c) {
            controller_free_chain(c->extra_events);
            c->extra_events = NULL;
        }
    }
}

// This function is called with changing interval, depending on the value of game speed
void game_state_call_tick(game_state *gs, int mode) {
    render_obj *robj;
    iterator it;
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        if(mode == TICK_DYNAMIC) {
            object_dynamic_tick(robj->obj);
        } else {
            object_static_tick(robj->obj);
        }
    }

    // Speed back up
    if(gs->speed_slowdown_time == 0) {
        DEBUG("Slowdown: Speed back up from %d to %d.", gs->speed, gs->speed_slowdown_previous);
        gs->speed = gs->speed_slowdown_previous;
    }
    if(gs->speed_slowdown_time >= 0) {
        gs->speed_slowdown_time--;
    }
}

// This function is always called with the same interval, and game speed does not affect it
void game_state_static_tick(game_state *gs) {
    // Set scene crossfade values
    if(gs->next_wait_ticks > 0) {
        gs->next_wait_ticks--;
        video_set_fade((float)gs->next_wait_ticks / (float)FRAME_WAIT_TICKS);
    }
    if(gs->this_wait_ticks > 0) {
        gs->this_wait_ticks--;
        video_set_fade(1.0f - (float)gs->this_wait_ticks / (float)FRAME_WAIT_TICKS);
    }

    // Call static ticks for scene
    scene_static_tick(gs->sc, game_state_is_paused(gs));

    // Call static tick functions
    game_state_call_tick(gs, TICK_STATIC);
}

// This function is called when the game speed requires it
void game_state_dynamic_tick(game_state *gs) {
    // We want to load another scene
    if(gs->this_id != gs->next_id && (gs->next_wait_ticks <= 1 || !settings_get()->video.crossfade_on)) {
        // If this is the end, set run to 0 so that engine knows to close here
        if(gs->next_id == SCENE_NONE) {
            DEBUG("Next ID is SCENE_NONE! bailing.");
            gs->run = 0;
            return;
        }

        // Load up new scene
        if(game_load_new(gs, gs->next_id)) {
            PERROR("Error while loading new scene! bailing.");
            gs->run = 0;
            return;
        }
        if(settings_get()->video.crossfade_on) {
            gs->this_wait_ticks = FRAME_WAIT_TICKS;
        } else {
            gs->this_wait_ticks = 0;
            gs->next_wait_ticks = 0;
        }
    }

    // Change the screen shake value downwards
    if(gs->screen_shake_horizontal > 0 && !gs->paused) {
        gs->screen_shake_horizontal--;
    }

    if(gs->screen_shake_vertical > 0 && !gs->paused) {
        gs->screen_shake_vertical--;
    }

    if (gs->screen_shake_horizontal > 0 || gs->screen_shake_vertical > 0) {
        float shake_x = sin(gs->screen_shake_horizontal) * 5 * ((float)gs->screen_shake_horizontal / 15);
        float shake_y = sin(gs->screen_shake_vertical) * 5 * ((float)gs->screen_shake_vertical / 15);
        video_move_target((int)shake_x, (int)shake_y);
        for(int i = 0; i < game_state_num_players(gs); i++) {
            game_player *gp = game_state_get_player(gs, i);
            controller *c = game_player_get_ctrl(gp);
            if(c) {
                controller_rumble(c, max2(gs->screen_shake_horizontal, gs->screen_shake_vertical)/12.0f, max2(gs->screen_shake_horizontal, gs->screen_shake_vertical) * game_state_ms_per_dyntick(gs));
            }
        }
    } else {
        // XXX Ocasionally the screen does not return back to normal position
        video_move_target(0, 0);
    }

    game_state_dyntick_controllers(gs);

    // Tick scene
    scene_dynamic_tick(gs->sc, game_state_is_paused(gs));

    // Poll input. If console is opened, do not poll the controllers.
    if(!console_window_is_open()) {
        scene_input_poll(gs->sc);
    }

    if(!game_state_is_paused(gs)) {
        // Clean up objects
        game_state_cleanup(gs);

        // Call object_move for all objects
        game_state_call_move(gs);

        // Handle physics for all pairs of objects
        game_state_call_collide(gs);

        // Tick all objects
        game_state_call_tick(gs, TICK_DYNAMIC);

        // Increment tick
        gs->tick++;
        LOGTICK(gs->tick);
    }

    // Free extra controller events
    game_state_ctrl_events_free(gs);

    // int_tick is used for ping calculation so it shouldn't be touched
    gs->int_tick++;
}

unsigned int game_state_get_tick(game_state *gs) {
    return gs->tick;
}

game_player* game_state_get_player(game_state *gs, int player_id) {
    return gs->players[player_id];
}

int game_state_num_players(game_state *gs) {
    return sizeof(gs->players)/sizeof(game_player*);
}

void _setup_keyboard(game_state *gs, int player_id) {
    settings_keyboard *k = &settings_get()->keys;
    // Set up controller
    controller *ctrl = malloc(sizeof(controller));
    game_player *player = game_state_get_player(gs, player_id);
    controller_init(ctrl);

    // Set up keyboards
    keyboard_keys *keys = malloc(sizeof(keyboard_keys));
    if(player_id == 0) {
        keys->jump_up = SDL_GetScancodeFromName(k->key1_jump_up);
        keys->jump_right = SDL_GetScancodeFromName(k->key1_jump_right);
        keys->walk_right = SDL_GetScancodeFromName(k->key1_walk_right);
        keys->duck_forward = SDL_GetScancodeFromName(k->key1_duck_forward);
        keys->duck = SDL_GetScancodeFromName(k->key1_duck);
        keys->duck_back = SDL_GetScancodeFromName(k->key1_duck_back);
        keys->walk_back = SDL_GetScancodeFromName(k->key1_walk_back);
        keys->jump_left = SDL_GetScancodeFromName(k->key1_jump_left);
        keys->punch = SDL_GetScancodeFromName(k->key1_punch);
        keys->kick = SDL_GetScancodeFromName(k->key1_kick);
        keys->escape = SDL_GetScancodeFromName(k->key1_escape);
    } else {
        keys->jump_up = SDL_GetScancodeFromName(k->key2_jump_up);
        keys->jump_right = SDL_GetScancodeFromName(k->key2_jump_right);
        keys->walk_right = SDL_GetScancodeFromName(k->key2_walk_right);
        keys->duck_forward = SDL_GetScancodeFromName(k->key2_duck_forward);
        keys->duck = SDL_GetScancodeFromName(k->key2_duck);
        keys->duck_back = SDL_GetScancodeFromName(k->key2_duck_back);
        keys->walk_back = SDL_GetScancodeFromName(k->key2_walk_back);
        keys->jump_left = SDL_GetScancodeFromName(k->key2_jump_left);
        keys->punch = SDL_GetScancodeFromName(k->key2_punch);
        keys->kick = SDL_GetScancodeFromName(k->key2_kick);
        keys->escape = SDL_GetScancodeFromName(k->key2_escape);
    }

    keyboard_create(ctrl, keys, 0);

    // Set up player controller
    game_player_set_ctrl(player, ctrl);
    game_player_set_selectable(player, 1);
}

void _setup_ai(game_state *gs, int player_id) {
    controller *ctrl = malloc(sizeof(controller));
    game_player *player = game_state_get_player(gs, player_id);
    controller_init(ctrl);

    ai_controller_create(ctrl, settings_get()->gameplay.difficulty);

    game_player_set_ctrl(player, ctrl);
    game_player_set_selectable(player, 0);
}

int _setup_joystick(game_state *gs, int player_id, const char *joyname, int offset) {
    controller *ctrl = malloc(sizeof(controller));
    game_player *player = game_state_get_player(gs, player_id);
    controller_init(ctrl);

    int res = joystick_create(ctrl, joystick_name_to_id(joyname, offset));
    game_player_set_ctrl(player, ctrl);
    game_player_set_selectable(player, 1);
    return res;
}

void _setup_rec_controller(game_state *gs, int player_id, sd_rec_file *rec) {
    controller *ctrl = malloc(sizeof(controller));
    game_player *player = game_state_get_player(gs, player_id);
    controller_init(ctrl);

    rec_controller_create(ctrl, player_id, rec);
    game_player_set_ctrl(player, ctrl);
}

void reconfigure_controller(game_state *gs) {
    settings_keyboard *k = &settings_get()->keys;
    if (k->ctrl_type1 == CTRL_TYPE_KEYBOARD) {
        _setup_keyboard(gs, 0);
    } else if (k->ctrl_type1 == CTRL_TYPE_GAMEPAD) {
        if (!_setup_joystick(gs, 0, k->joy_name1, k->joy_offset1)) {
            // fallback on the good old keyboard
            k->ctrl_type1 = CTRL_TYPE_KEYBOARD;
            reconfigure_controller(gs);
        }
    }

    // Set up second player keyboard to be available in menu
    _setup_keyboard(gs, 1);
}

void game_state_init_demo(game_state *gs) {
    // Set up player controller
    for(int i = 0;i < game_state_num_players(gs);i++) {
        game_player *player = game_state_get_player(gs, i);
        controller *ctrl = malloc(sizeof(controller));
        controller_init(ctrl);
        ai_controller_create(ctrl, 4);
        game_player_set_ctrl(player, ctrl);
        game_player_set_selectable(player, 1);

        // select random pilot and har
        player->pilot_id = rand_int(10);
        player->har_id = rand_int(11);
        chr_score_reset(&player->score, 1);

        // set proper color
        pilot pilot_info;
        pilot_get_info(&pilot_info, player->pilot_id);
        player->colors[0] = pilot_info.colors[0];
        player->colors[1] = pilot_info.colors[1];
        player->colors[2] = pilot_info.colors[2];
    }
}

void game_state_free(game_state *gs) {
    // Free objects
    render_obj *robj;
    iterator it;
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        object_free(robj->obj);
        free(robj->obj);
        vector_delete(&gs->objects, &it);
    }
    vector_free(&gs->objects);

    // Free scene
    scene_free(gs->sc);
    free(gs->sc);

    // Free players
    for(int i = 0; i < 2; i++) {
        game_player_set_ctrl(gs->players[i], NULL);
        game_player_free(gs->players[i]);
        free(gs->players[i]);
    }
}

int game_state_ms_per_dyntick(game_state *gs) {
    float tmp;
    switch(gs->this_id) {
        case SCENE_ARENA0:
        case SCENE_ARENA1:
        case SCENE_ARENA2:
        case SCENE_ARENA3:
        case SCENE_ARENA4:
            tmp = 8.0f + MS_PER_OMF_TICK_SLOWEST - ((float)gs->speed / 15.0f) * MS_PER_OMF_TICK_SLOWEST;
            return (int)tmp;
    }
    return MS_PER_OMF_TICK;
}

int game_state_serialize(game_state *gs, serial *ser) {
    // serialize tick time and random seed, so client can reply state from this point
    serial_write_int32(ser, game_state_get_tick(gs));
    serial_write_int32(ser, rand_get_seed());
    serial_write_int32(ser, game_state_is_paused(gs));

    object *har[2];
    har[0] = game_state_get_player(gs, 0)->har;
    har[1] = game_state_get_player(gs, 1)->har;

    object_serialize(har[0], ser);
    object_serialize(har[1], ser);

    serial objects;
    serial_create(&objects);

    // serialize any HAZARD or PROJECTILE objects
    iterator it;
    vector_iter_begin(&gs->objects, &it);
    render_obj *robj;
    uint8_t count = 0;
    while((robj = iter_next(&it)) != NULL) {
        if (robj->obj->group == GROUP_PROJECTILE) {
            serial_write_int8(&objects, robj->layer);
            object_serialize(robj->obj, &objects);
            count++;
        }
    }
    serial_write_int8(ser, count);

    serial_write(ser, objects.data, objects.len);
    serial_free(&objects);

    chr_score_serialize(game_player_get_score(game_state_get_player(gs, 0)), ser);
    chr_score_serialize(game_player_get_score(game_state_get_player(gs, 1)), ser);

    return 0;
}

int game_state_unserialize(game_state *gs, serial *ser, int rtt) {
#ifdef DEBUGMODE
    int oldtick = gs->tick;
#endif
    gs->tick = serial_read_int32(ser);
    int endtick = gs->tick + ceil(rtt / 2.0f);
    rand_seed(serial_read_int32(ser));
    game_state_set_paused(gs, serial_read_int32(ser));

    for(int i = 0; i < 2; i++) {
        // Declare some vars
        game_player *player = game_state_get_player(gs, i);
        game_state_del_object(gs, player->har);
        object *obj = malloc(sizeof(object));

        // Create object and specialize it as HAR.
        // Errors are unlikely here, but check anyway.

        object_create(obj, gs, vec2i_create(0, 0), vec2f_create(0,0));
        object_unserialize(obj, ser, gs);

        // Set HAR to controller and game_player
        game_state_add_object(gs, obj, RENDER_LAYER_MIDDLE, 0, 0);

        // Set HAR for player
        game_player_set_har(player, obj);
        game_player_get_ctrl(player)->har = obj;
    }

    // ensure the HARs know each other's positions
    object *obj_har1,*obj_har2;
    obj_har1 = game_player_get_har(game_state_get_player(gs, 0));
    obj_har2 = game_player_get_har(game_state_get_player(gs, 1));

    obj_har1->animation_state.enemy = obj_har2;
    obj_har2->animation_state.enemy = obj_har1;

    // clean out any current projectiles/hazards
    iterator it;
    vector_iter_begin(&gs->objects, &it);
    render_obj *robj;
    while((robj = iter_next(&it)) != NULL) {
        if (robj->obj->group == GROUP_PROJECTILE) {
            object_free(robj->obj);
            free(robj->obj);
            vector_delete(&gs->objects, &it);
        }
    }

    uint8_t count = serial_read_int8(ser);

    for (int i = 0; i < count; i++) {
        object *obj = malloc(sizeof(object));
        int layer = serial_read_int8(ser);
        object_create(obj, gs, vec2i_create(0, 0), vec2f_create(0,0));
        object_unserialize(obj, ser, gs);
        DEBUG("newly added object finish status %d", object_finished(obj));

        game_state_add_object(gs, obj, layer, 0, 0);
    }

    chr_score_unserialize(game_player_get_score(game_state_get_player(gs, 0)), ser);
    chr_score_unserialize(game_player_get_score(game_state_get_player(gs, 1)), ser);

    // tick things back to the current time
    DEBUG("replaying %d ticks", endtick - gs->tick);
    DEBUG("adjusting clock from %d to %d (%d)", oldtick, endtick, ceil(rtt / 2.0f));
    while (gs->tick <= endtick) {
        game_state_cleanup(gs);
        game_state_call_move(gs);
        game_state_call_collide(gs);
        game_state_call_tick(gs, TICK_DYNAMIC);
        gs->tick++;
    }
    DEBUG("replay done");

    return 0;
}
