#include "game/game_state.h"
#include "audio/audio.h"
#include "console/console.h"
#include "controller/joystick.h"
#include "controller/keyboard.h"
#include "controller/rec_controller.h"
#include "formats/error.h"
#include "formats/pilot.h"
#include "formats/rec.h"
#include "game/common_defines.h"
#include "game/protos/object.h"
#include "game/protos/scene.h"
#include "game/scenes/arena.h"
#include "game/scenes/credits.h"
#include "game/scenes/cutscene.h"
#include "game/scenes/intro.h"
#include "game/scenes/lobby.h"
#include "game/scenes/mainmenu.h"
#include "game/scenes/mechlab.h"
#include "game/scenes/melee.h"
#include "game/scenes/newsroom.h"
#include "game/scenes/openomf.h"
#include "game/scenes/scoreboard.h"
#include "game/scenes/vs.h"
#include "game/utils/serial.h"
#include "game/utils/settings.h"
#include "game/utils/ticktimer.h"
#include "resources/pilots.h"
#include "resources/sounds_loader.h"
#include "utils/allocator.h"
#include "utils/c_array_util.h"
#include "utils/log.h"
#include "utils/miscmath.h"
#include "video/vga_state.h"
#include "video/video.h"
#include <SDL.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define MS_PER_OMF_TICK 10
#define MS_PER_OMF_TICK_SLOWEST 60

enum
{
    TICK_DYNAMIC = 0,
    TICK_STATIC,
};

static void _setup_rec_controller(game_state *gs, int player_id, sd_rec_file *rec);

// How long the scene waits after order to move to another scene
// Used for crossfades
#define FRAME_WAIT_TICKS 30

typedef struct {
    int layer;      ///< Object rendering layer
    int persistent; ///< 1 if the object should keep alive across scene boundaries
    int singleton;  ///< 1 if object should be the only representative of its animation ID
    object *obj;
} render_obj;

typedef struct {
    int tick;
    int id;
    int length;
    int duration;
    float volume;
    float panning;
    float pitch;
    int playback_id;
} playing_sound;

int game_state_create(game_state *gs, engine_init_flags *init_flags) {
    gs->run = 1;
    gs->paused = 0;
    gs->tick = 0;
    gs->int_tick = 0;
    gs->role = ROLE_CLIENT;
    gs->net_mode = init_flags->net_mode;
    gs->speed = settings_get()->gameplay.speed + 5;
    gs->init_flags = init_flags;
    gs->new_state = NULL;
    gs->clone = false;
    vector_create(&gs->objects, sizeof(render_obj));
    vector_create(&gs->sounds, sizeof(playing_sound));

    // For screen shake
    gs->screen_shake_horizontal = 0;
    gs->screen_shake_vertical = 0;

    // For momentary game speed switches
    gs->speed_slowdown_previous = 0;
    gs->speed_slowdown_time = -1;

    // Used for crossfades
    gs->next_wait_ticks = 0;
    gs->this_wait_ticks = 0;

    // Disable warp (debug) speed by default. This can be set in console.
    gs->warp_speed = 0;

    gs->hide_ui = false;
    gs->menu_ctrl = omf_calloc(1, sizeof(controller));

    // Set up players
    gs->sc = omf_calloc(1, sizeof(scene));
    for(int i = 0; i < 2; i++) {
        gs->players[i] = omf_calloc(1, sizeof(game_player));
        game_player_create(gs->players[i]);
    }

    reconfigure_controller(gs);
    int nscene;
    if(strlen(init_flags->rec_file) > 0 && init_flags->record == 0) {
        sd_rec_file rec;
        sd_rec_create(&rec);
        int ret = sd_rec_load(&rec, init_flags->rec_file);
        if(ret != SD_SUCCESS) {
            PERROR("Unable to load recording %s.", init_flags->rec_file);
            goto error_0;
        }

        nscene = SCENE_ARENA0 + rec.arena_id;
        DEBUG("playing recording file %s", init_flags->rec_file);
        gs->this_id = nscene;
        gs->next_id = nscene;

        if(scene_create(gs->sc, gs, nscene)) {
            PERROR("Error while loading scene %d.", nscene);
            goto error_0;
        }

        // set the HAR colors, pilot, har type
        for(int i = 0; i < 2; i++) {
            sd_pilot_set_player_color(gs->players[i]->pilot, PRIMARY, rec.pilots[i].info.color_3);
            sd_pilot_set_player_color(gs->players[i]->pilot, SECONDARY, rec.pilots[i].info.color_2);
            sd_pilot_set_player_color(gs->players[i]->pilot, TERTIARY, rec.pilots[i].info.color_1);
            gs->players[i]->pilot->har_id = HAR_JAGUAR + rec.pilots[i].info.har_id;
            gs->players[i]->pilot->pilot_id = rec.pilots[i].info.pilot_id;
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
        gs->this_id = nscene;
        gs->next_id = nscene;

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

    random_seed(&gs->rand, time(NULL));

    // Initialize scene
    scene_init(gs->sc);

    // All done
    return 0;

error_1:
    scene_free(gs->sc);
error_0:
    omf_free(gs->sc);
    vector_free(&gs->objects);
    vector_free(&gs->sounds);
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
            omf_free(robj->obj);
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
            omf_free(robj->obj);
            vector_delete(&gs->objects, &it);
            return;
        }
    }
}

void game_state_del_object_by_id(game_state *gs, uint32_t target) {
    iterator it;
    render_obj *robj;
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        if(target == robj->obj->id) {
            object_free(robj->obj);
            omf_free(robj->obj);
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

void game_state_clear_objects(game_state *gs, int mask) {
    iterator it;
    render_obj *robj;
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        if(object_get_group(robj->obj) & mask) {
            object_free(robj->obj);
            omf_free(robj->obj);
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

scene *game_state_get_scene(game_state *gs) {
    return gs->sc;
}

unsigned int game_state_is_running(game_state *gs) {
    return gs->run;
}

unsigned int game_state_is_paused(game_state *gs) {
    return gs->paused;
}

void game_state_set_paused(game_state *gs, unsigned int paused) {
    // don't pause netplay games
    if(is_netplay(gs)) {
        return;
    }

    gs->paused = paused;
}

// Return 0 if event was handled here
int game_state_handle_event(game_state *gs, SDL_Event *event) {
    if(event->type == SDL_KEYDOWN && is_demoplay(gs) && event->key.keysym.sym == SDLK_ESCAPE) {
        // ESC during demo mode jumps you back to the main menu
        game_state_set_next(gs, SCENE_MENU);
        return 0;
    } else if(event->type == SDL_KEYDOWN && is_demoplay(gs) && event->key.keysym.sym == SDLK_RETURN) {
        // ENTER during demo mode skips menus
        if(gs->sc->id < SCENE_ARENA0 || gs->sc->id > SCENE_ARENA4) {
            if(gs->sc->id != SCENE_VS) {
                game_state_init_demo(gs);
            }
            game_state_set_next(gs, rand_arena());
            return 0;
        }
    }
    if(scene_event(gs->sc, event) == 0) {
        return 0;
    }
    return 1;
}

void cross_fade_transform(damage_tracker *damage, vga_palette *pal, void *userdata) {
    game_state *gs = userdata;
    float value = 1.0f;

    if(gs->this_wait_ticks > 0) {
        value = 1.0f - gs->this_wait_ticks / (float)FRAME_WAIT_TICKS;
    }
    if(gs->next_wait_ticks > 0) {
        value = gs->next_wait_ticks / (float)FRAME_WAIT_TICKS;
    }

    // Set palette darkness value.
    for(int i = 0; i < 256; i++) {
        pal->colors[i].r *= value;
        pal->colors[i].g *= value;
        pal->colors[i].b *= value;
    }

    damage_set_range(damage, 0, 255);
}

void game_state_render(game_state *gs) {
    iterator it;
    render_obj *robj;

    // Render scene background
    scene_render(gs->sc);

    // Get har objects
    object *har[2];
    har[0] = game_state_find_object(gs, game_state_get_player(gs, 0)->har_obj_id);
    har[1] = game_state_find_object(gs, game_state_get_player(gs, 1)->har_obj_id);

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

void game_state_palette_transform(game_state *gs) {
    // object transforms
    render_obj *robj;
    iterator it;
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        object_palette_transform(robj->obj);
    }

    // Cross-fade effect
    if(gs->next_wait_ticks > 0 || gs->this_wait_ticks > 0) {
        vga_state_enable_palette_transform(cross_fade_transform, gs);
    }
}

void game_state_debug(game_state *gs) {
    // If we are in debug mode, handle HAR debug layers
#ifdef DEBUGMODE
    for(int i = 0; i < 2; i++) {
        object *h = game_state_find_object(gs, game_state_get_player(gs, i)->har_obj_id);
        if(h != NULL) {
            object_debug(h);
        }
    }
#endif
}

int game_load_new(game_state *gs, int scene_id) {
    // Free old scene
    scene_free(gs->sc);
    omf_free(gs->sc);

    // Remove old objects
    render_obj *robj;
    iterator it;
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        if(!robj->persistent) {
            object_free(robj->obj);
            omf_free(robj->obj);
            vector_delete(&gs->objects, &it);
        }
    }

    // Free texture items, we are going to create new ones.
    video_signal_scene_change();

    gs->this_id = scene_id;
    gs->next_id = scene_id;

    // Initialize new scene with BK data etc.
    gs->sc = omf_calloc(1, sizeof(scene));
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
        case SCENE_LOBBY:
            if(lobby_create(gs->sc)) {
                PERROR("Error creating Lobby scene.");
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
    gs->tick = 0;
    return 0;

error_1:
    scene_free(gs->sc);
error_0:
    omf_free(gs->sc);
    return 1;
}

void game_state_call_collide(game_state *gs) {
    object *a, *b;
    unsigned int size = vector_size(&gs->objects);
    for(unsigned i = 0; i < size; i++) {
        a = ((render_obj *)vector_get(&gs->objects, i))->obj;
        for(unsigned k = i + 1; k < size; k++) {
            b = ((render_obj *)vector_get(&gs->objects, k))->obj;
            if(a->group != b->group || a->group == GROUP_UNKNOWN || b->group == GROUP_UNKNOWN) {
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
            omf_free(robj->obj);
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
    controller_tick(gs->menu_ctrl, gs->int_tick, &gs->menu_ctrl->extra_events);
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

    playing_sound *s;
    vector_iter_begin(&gs->sounds, &it);
    while((s = iter_next(&it)) != NULL) {
        if(mode == TICK_DYNAMIC) {
            s->duration -= game_state_ms_per_dyntick(gs);
        } else {
            s->duration -= 10; // static ticks are 10ms??
        }
        if(s->duration <= 0) {
            DEBUG("removing finished sound %d of length %d started on tick %d", s->playback_id, s->length, s->tick);
            vector_delete(&gs->sounds, &it);
        }
    }
}

void game_state_merge_sounds(game_state *old, game_state *new) {
    // We need to do several things here:
    // * Leave any sounds that are playing in both states alone
    // * Fade out any sounds only playing in the old state
    // * Fade in any new sounds, and start playing them at the appropriate offset

    playing_sound *s, *s2;
    iterator it, it2;
    vector_iter_begin(&old->sounds, &it);
    while((s = iter_next(&it)) != NULL) {
        bool found = false;
        vector_iter_begin(&new->sounds, &it2);
        while((s2 = iter_next(&it2)) != NULL) {
            if(s->id == s2->id && s->tick == s2->tick) {
                // same sound, same frame
                found = true;
                break;
            }
        }

        if(!found) {
            // this sound no longer exists after a rollback, so we need to fade it out
            audio_fade_out(s->id, 500);
            // don't bother adding it to the new sound vector though
        }
    }

    vector_iter_begin(&new->sounds, &it);
    while((s = iter_next(&it)) != NULL) {
        bool found = false;
        vector_iter_begin(&old->sounds, &it2);
        while((s2 = iter_next(&it2)) != NULL) {
            if(s->id == s2->id && s->tick == s2->tick) {
                // same sound, same frame
                found = true;
                break;
            }
        }

        if(!found) {
            // this sound was added during the rollback, so we need to start playing it
            // but we need to determine the playback offset AND fade it in
            //
            // this sound should NOT have been played already!
            assert(s->playback_id == -1);

            // calculate the offset into the buffer we need

            // TODO factor in pitch here
            int total_duration = (int)((s->length / 8000.0f) * 1000.0f);
            int elapsed_ms = total_duration - s->duration;
            // TODO with 8khz mono, we can just multiply ms by 8, I think?
            int offset = elapsed_ms * 8;

            // Load sample (8000Hz, mono, 8bit)
            char *src_buf;
            int src_len;
            if(!sounds_loader_get(s->id, &src_buf, &src_len)) {
                PERROR("Requested sound sample %d not found", s->id);
                return;
            }
            if(src_len == 0) {
                DEBUG("Requested sound sample %d has nothing to play", s->id);
                return;
            }

            // TODO decide on a fade in time
            s->playback_id =
                audio_play_sound_buf(src_buf + offset, src_len - offset, s->volume, s->panning, s->pitch, 500);
        }
    }
}

// This function is always called with the same interval, and game speed does not affect it
void game_state_static_tick(game_state *gs, bool replay) {
    // Set scene crossfade values
    if(gs->this_wait_ticks > 0) {
        gs->this_wait_ticks--;
    }
    if(gs->next_wait_ticks > 0) {
        gs->next_wait_ticks--;
    }

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

    // Tick controllers
    game_state_tick_controllers(gs);

    if(gs->new_state) {
        // merge the sounds
        game_state_merge_sounds(gs, gs->new_state);
        gs = gs->new_state;
        // remove the cloned flag
        gs->clone = false;
    }

    // Call static ticks for scene
    scene_static_tick(gs->sc, game_state_is_paused(gs));

    // Call static tick functions
    game_state_call_tick(gs, TICK_STATIC);
}

// This function is called when the game speed requires it
void game_state_dynamic_tick(game_state *gs, bool replay) {
    // Change the screen shake value downwards
    if(gs->screen_shake_horizontal > 0 && !gs->paused) {
        gs->screen_shake_horizontal--;
    }

    if(gs->screen_shake_vertical > 0 && !gs->paused) {
        gs->screen_shake_vertical--;
    }

    if((gs->screen_shake_horizontal > 0 || gs->screen_shake_vertical > 0) && !replay) {
        float shake_x = sin(gs->screen_shake_horizontal) * 5 * ((float)gs->screen_shake_horizontal / 15);
        float shake_y = sin(gs->screen_shake_vertical) * 5 * ((float)gs->screen_shake_vertical / 15);
        video_move_target((int)shake_x, (int)shake_y);
        for(int i = 0; i < game_state_num_players(gs); i++) {
            game_player *gp = game_state_get_player(gs, i);
            controller *c = game_player_get_ctrl(gp);
            // TODO, like audio rumble needs to be reworked to be done in slices
            if(c && !replay) {
                controller_rumble(c, max2(gs->screen_shake_horizontal, gs->screen_shake_vertical) / 12.0f,
                                  max2(gs->screen_shake_horizontal, gs->screen_shake_vertical) *
                                      game_state_ms_per_dyntick(gs));
            }
        }
    } else {
        // XXX Ocasionally the screen does not return back to normal position
        if(!replay) {
            video_move_target(0, 0);
        }
    }

    if(!replay) {
        game_state_dyntick_controllers(gs);
    }

    // Tick scene
    scene_dynamic_tick(gs->sc, game_state_is_paused(gs));

    // Poll input. If console is opened, do not poll the controllers.
    if(!console_window_is_open() && !replay) {
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

    if(!replay) {
        // Free extra controller events
        game_state_ctrl_events_free(gs);
    }

    // Speed back up
    if(gs->speed_slowdown_time == 0) {
        DEBUG("Slowdown: Speed back up from %d to %d.", gs->speed, gs->speed_slowdown_previous);
        gs->speed = gs->speed_slowdown_previous;
    }
    if(gs->speed_slowdown_time >= 0) {
        gs->speed_slowdown_time--;
    }

    // int_tick is used for ping calculation so it shouldn't be touched
    gs->int_tick++;
}

unsigned int game_state_get_tick(game_state *gs) {
    return gs->tick;
}

game_player *game_state_get_player(const game_state *gs, int player_id) {
    return gs->players[player_id];
}

int game_state_num_players(game_state *gs) {
    return N_ELEMENTS(gs->players);
}

void _setup_keyboard(game_state *gs, int player_id) {
    settings_keyboard *k = &settings_get()->keys;
    // Set up controller
    controller *ctrl = omf_calloc(1, sizeof(controller));
    game_player *player = game_state_get_player(gs, player_id);
    controller_init(ctrl, gs);

    // Set up keyboards
    keyboard_keys *keys = omf_calloc(1, sizeof(keyboard_keys));
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
    }

    keyboard_create(ctrl, keys, 0);

    // Set up player controller
    game_player_set_ctrl(player, ctrl);
    game_player_set_selectable(player, 1);
}

void _setup_ai(game_state *gs, int player_id) {
    controller *ctrl = omf_calloc(1, sizeof(controller));
    game_player *player = game_state_get_player(gs, player_id);
    controller_init(ctrl, gs);

    sd_pilot *pilot = game_player_get_pilot(player);
    ai_controller_create(ctrl, settings_get()->gameplay.difficulty, pilot, player->pilot->pilot_id);

    game_player_set_ctrl(player, ctrl);
    game_player_set_selectable(player, 0);
}

int _setup_joystick(game_state *gs, int player_id, const char *joyname, int offset) {
    controller *ctrl = omf_calloc(1, sizeof(controller));
    game_player *player = game_state_get_player(gs, player_id);
    controller_init(ctrl, gs);

    int res = joystick_create(ctrl, joystick_name_to_id(joyname, offset));
    game_player_set_ctrl(player, ctrl);
    game_player_set_selectable(player, 1);
    return res;
}

static void _setup_rec_controller(game_state *gs, int player_id, sd_rec_file *rec) {
    controller *ctrl = omf_calloc(1, sizeof(controller));
    game_player *player = game_state_get_player(gs, player_id);
    controller_init(ctrl, gs);

    rec_controller_create(ctrl, player_id, rec);
    game_player_set_ctrl(player, ctrl);
}

void reconfigure_controller(game_state *gs) {
    settings_keyboard *k = &settings_get()->keys;
    if(k->ctrl_type1 == CTRL_TYPE_KEYBOARD) {
        _setup_keyboard(gs, 0);
    } else if(k->ctrl_type1 == CTRL_TYPE_GAMEPAD) {
        if(!_setup_joystick(gs, 0, k->joy_name1, k->joy_offset1)) {
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
    for(int i = 0; i < game_state_num_players(gs); i++) {
        game_player *player = game_state_get_player(gs, i);
        controller *ctrl = omf_calloc(1, sizeof(controller));
        controller_init(ctrl, gs);
        sd_pilot *pl = game_player_get_pilot(player);
        ai_controller_create(ctrl, 4, pl, player->pilot->pilot_id);
        game_player_set_ctrl(player, ctrl);
        game_player_set_selectable(player, 0);

        // select random pilot and har
        player->pilot->pilot_id = rand_int(NUMBER_OF_PLAYABLE_PILOT_TYPES);
        player->pilot->har_id = rand_int(NUMBER_OF_HAR_TYPES);
        chr_score_reset(&player->score, 1);

        // set proper color
        pilot pilot_info;
        pilot_get_info(&pilot_info, player->pilot->pilot_id);
        sd_pilot_set_player_color(player->pilot, PRIMARY, pilot_info.colors[2]);
        sd_pilot_set_player_color(player->pilot, SECONDARY, pilot_info.colors[1]);
        sd_pilot_set_player_color(player->pilot, TERTIARY, pilot_info.colors[0]);
    }
}

void game_state_menu_poll(game_state *gs, ctrl_event **ev) {
    gs->menu_ctrl->last = gs->menu_ctrl->current;
    gs->menu_ctrl->current = 0;
    // poll keyboard
    keyboard_menu_poll(gs->menu_ctrl, ev);
    // poll joysticks
    joystick_menu_poll_all(gs->menu_ctrl, ev);
}

void game_state_clone_free(game_state *gs) {
    // Free objects
    render_obj *robj;
    iterator it;
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        object_clone_free(robj->obj);
        omf_free(robj->obj);
        vector_delete(&gs->objects, &it);
    }
    vector_free(&gs->objects);

    // Free scene
    scene_clone_free(gs->sc);
    // omf_free(gs->sc);

    // Free players
    for(int i = 0; i < 2; i++) {
        // game_player_set_ctrl(gs->players[i], NULL);
        game_player_clone_free(gs->players[i]);
        omf_free(gs->players[i]);
    }
    // omf_free(gs);
}

void game_state_free(game_state **_gs) {
    game_state *gs = *_gs;
    *_gs = NULL;

    // Free objects
    render_obj *robj;
    iterator it;
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        object_free(robj->obj);
        omf_free(robj->obj);
        vector_delete(&gs->objects, &it);
    }
    vector_free(&gs->objects);

    // Free scene
    scene_free(gs->sc);
    omf_free(gs->sc);

    // Free players
    for(int i = 0; i < 2; i++) {
        game_player_set_ctrl(gs->players[i], NULL);
        game_player_free(gs->players[i]);
        omf_free(gs->players[i]);
    }
    omf_free(gs->menu_ctrl);
    omf_free(gs);
}

int game_state_ms_per_dyntick(game_state *gs) {
    float tmp;

    switch(gs->this_id) {
        case SCENE_ARENA0:
        case SCENE_ARENA1:
        case SCENE_ARENA2:
        case SCENE_ARENA3:
        case SCENE_ARENA4:
            if(gs->warp_speed) {
                // If warp speed (for debugging) is turned on, go fast.
                return 1.0f;
            }
            tmp = 8.0f + MS_PER_OMF_TICK_SLOWEST - ((float)gs->speed / 15.0f) * MS_PER_OMF_TICK_SLOWEST;
            return (int)tmp;
    }
    return MS_PER_OMF_TICK;
}

int render_obj_clone(render_obj *src, render_obj *dst, game_state *gs) {
    memcpy(dst, src, sizeof(render_obj));
    dst->obj = omf_calloc(1, sizeof(object));
    return object_clone(src->obj, dst->obj, gs);
}

object *game_state_find_object(game_state *gs, uint32_t object_id) {
    iterator it;
    vector_iter_begin(&gs->objects, &it);
    render_obj *robj;
    while((robj = iter_next(&it)) != NULL) {
        if(robj->obj->id == object_id) {
            return robj->obj;
        }
    }
    return NULL;
}

void game_state_play_sound(game_state *gs, int id, float volume, float panning, float pitch) {
    if(id < 0 || id > 299)
        return;

    // Load sample (8000Hz, mono, 8bit)
    char *src_buf;
    int src_len;
    if(!sounds_loader_get(id, &src_buf, &src_len)) {
        PERROR("Requested sound sample %d not found", id);
        return;
    }
    if(src_len == 0) {
        DEBUG("Requested sound sample %d has nothing to play", id);
        return;
    }

    playing_sound s;
    s.tick = gs->int_tick;
    s.id = id;
    s.length = src_len;
    // TODO factor in pitch
    s.duration = (int)((src_len / 8000.0f) * 1000.0f);
    s.volume = volume;
    s.panning = panning;
    s.pitch = pitch;
    s.playback_id = -1;

    // TODO calculate what tick this sound should be finished after
    // so we can remove it when it is no longer relevant

    if(!gs->clone) {
        // do not actually begin playback if this is a cloned game state
        // cloned game states that are promoted to the active game state
        // will have this flag removed
        s.playback_id = audio_play_sound_buf(src_buf, src_len, volume, panning, pitch, 0);
        if(s.playback_id == -1) {
            // don't track sounds that failed to play
            return;
        }
    }

    vector_append(&gs->sounds, &s);
}

int game_state_clone(game_state *src, game_state *dst) {
    // copy all the static fields
    memcpy(dst, src, sizeof(game_state));
    // fix any pointers to volatile data
    vector_create(&dst->objects, sizeof(render_obj));
    vector_create(&dst->sounds, sizeof(playing_sound));

    dst->next_wait_ticks = 0;
    dst->this_wait_ticks = 0;

    iterator it;
    vector_iter_begin(&src->objects, &it);
    render_obj *robj;
    while((robj = iter_next(&it)) != NULL) {
        render_obj d;
        render_obj_clone(robj, &d, dst);
        vector_append(&dst->objects, &d);
    }

    vector_iter_begin(&src->sounds, &it);
    playing_sound *s;
    while((s = iter_next(&it)) != NULL) {
        vector_append(&dst->sounds, s);
    }

    for(int i = 0; i < 2; i++) {
        dst->players[i] = omf_calloc(1, sizeof(game_player));
        game_player_clone(src->players[i], dst->players[i]);
        // update HAR object pointers
        // dst->players[i]->har_obj_id = src->players[i]->har_obj_id;
    }

    dst->sc = omf_calloc(1, sizeof(scene));
    scene_clone(src->sc, dst->sc, dst);

    dst->new_state = NULL;

    dst->clone = true;

    return 0;
}

bool is_netplay(game_state *gs) {
    return game_state_get_player(gs, 0)->ctrl->type == CTRL_TYPE_NETWORK ||
           game_state_get_player(gs, 1)->ctrl->type == CTRL_TYPE_NETWORK;
}

bool is_singleplayer(game_state *gs) {
    return game_state_get_player(gs, 1)->ctrl->type == CTRL_TYPE_AI;
}

bool is_tournament(game_state *gs) {
    return game_state_get_player(gs, 0)->chr;
}

bool is_demoplay(game_state *gs) {
    return game_state_get_player(gs, 0)->ctrl->type == CTRL_TYPE_AI &&
           game_state_get_player(gs, 1)->ctrl->type == CTRL_TYPE_AI;
}

bool is_twoplayer(game_state *gs) {
    return !is_demoplay(gs) && !is_netplay(gs) && !is_singleplayer(gs);
}
