#include <stdlib.h>
#include <SDL2/SDL.h>
#include "controller/keyboard.h"
#include "utils/log.h"
#include "game/serial.h"
#include "resources/ids.h"
#include "console/console.h"
#include "video/video.h"
#include "game/game_state.h"
#include "game/settings.h"
#include "game/ticktimer.h"
#include "game/protos/scene.h"
#include "game/protos/object.h"
#include "game/protos/intersect.h"
#include "game/scenes/intro.h"
#include "game/scenes/mainmenu.h"
#include "game/scenes/credits.h"
#include "game/scenes/arena.h"
#include "game/scenes/mechlab.h"
#include "game/scenes/newsroom.h"
#include "game/scenes/melee.h"
#include "game/scenes/vs.h"

#define MS_PER_OMF_TICK 10
#define MS_PER_OMF_TICK_SLOWEST 150

typedef struct {
    int layer;
    object *obj;
} render_obj;

int game_state_create(game_state *gs, int net_mode) {
    gs->run = 1;
    gs->tick = 0;
    gs->role = ROLE_CLIENT;
    gs->net_mode = net_mode;
    gs->speed = settings_get()->gameplay.speed;
    vector_create(&gs->objects, sizeof(render_obj));
    gs->tick_timer = malloc(sizeof(ticktimer));
    ticktimer_init(gs->tick_timer);
    int nscene = (net_mode == NET_MODE_NONE ? SCENE_INTRO : SCENE_MENU);
    gs->sc = malloc(sizeof(scene));
    for(int i = 0; i < 2; i++) {
        gs->players[i] = malloc(sizeof(game_player));
        game_player_create(gs->players[i]);
    }
    if(scene_create(gs->sc, gs, nscene)) {
        PERROR("Error while loading scene %d.", nscene);
        goto error_0;
    }
    if(net_mode == NET_MODE_NONE) {
        if(intro_create(gs->sc)) {
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
    scene_init(gs->sc);
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

void game_state_add_object(game_state *gs, object *obj, int layer) {
    render_obj o;
    o.obj = obj;
    o.layer = layer;
    vector_append(&gs->objects, &o);

#ifdef DEBUGMODE
    animation *ani = object_get_animation(obj);
    DEBUG("Added animation %i to game_state on layer %d.", ani->id, layer);
#endif
}

void game_state_set_speed(game_state *gs, int speed) {
    DEBUG("game speed set to %d", speed);
    gs->speed = speed;
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

void game_state_set_next(game_state *gs, unsigned int next_scene_id) {
    gs->next_id = next_scene_id;
}

scene* game_state_get_scene(game_state *gs) {
    return gs->sc;
}

int game_state_is_running(game_state *gs) {
    return gs->run;
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
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        object_palette_transform(robj->obj, video_get_pal_ref());
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
            if(robj->obj == har[0] || robj->obj == har[1]) continue;
            object_render(robj->obj);
        }
    }

    // cast HAR shadows
    for(int i = 0; i < 2; i++) {
        if(har[i] != NULL) {
            object_render_shadow(har[i], &gs->sc->shadow_buffer_img);
        }
    }

    // cast object shadows (scrap, projectiles, etc)
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        object_render_shadow(robj->obj, &gs->sc->shadow_buffer_img);
    }

    // render the shadow laywe
    scene_render_shadows(gs->sc);

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
            if(robj->obj == har[0] || robj->obj == har[1]) continue;
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
            if(robj->obj == har[0] || robj->obj == har[1]) continue;
            object_render(robj->obj);
        }
    }

    // If we are in debug mode, handle HAR debug layers
#ifdef DEBUGMODE
    for(int i = 0; i < 2; i++) {
        if(har[i] != NULL) {
            object_debug(har[i]);
        }
    }
#endif

    // Render scene overlay (menus, etc.)
    scene_render_overlay(gs->sc);
}

int game_load_new(game_state *gs, int scene_id) {
    // Free old scene
    scene_free(gs->sc);
    free(gs->sc);

    render_obj *robj;
    iterator it;
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        object_free(robj->obj);
        free(robj->obj);
        vector_delete(&gs->objects, &it);
    }

    // Initialize new scene with BK data etc.
    gs->sc = malloc(sizeof(scene));
    if(scene_create(gs->sc, gs, scene_id)) {
        PERROR("Error while loading scene %d.", scene_id);
        return 1;
    }

    // Load scene specifics
    switch(scene_id) {
        case SCENE_INTRO: 
            if(intro_create(gs->sc)) {
                PERROR("Error while creating intro scene.");
                return 1;
            }
            break;
        case SCENE_MENU: 
            if(mainmenu_create(gs->sc)) {
                PERROR("Error while creating mainmenu scene.");
                return 1;
            }
            break;
        case SCENE_CREDITS: 
            if(credits_create(gs->sc)) {
                PERROR("Error while creating credits scene.");
                return 1;
            }
            break;
        case SCENE_MELEE:
            if(melee_create(gs->sc)) {
                PERROR("Error while creating melee scene.");
                return 1;
            } 
            break;
        case SCENE_VS:
            if(vs_create(gs->sc)) {
                PERROR("Error while creating VS scene.");
                return 1;
            }
            break;
        case SCENE_MECHLAB:
            if(mechlab_create(gs->sc)) {
                PERROR("Error while creating Mechlab scene.");
                return 1;
            }
            break;
        case SCENE_NEWSROOM:
            if(newsroom_create(gs->sc)) {
                PERROR("Error while creating Newsroom scene.");
                return 1;
            }
            break;
        case SCENE_ARENA0:
        case SCENE_ARENA1:
        case SCENE_ARENA2:
        case SCENE_ARENA3:
        case SCENE_ARENA4:
            if(arena_create(gs->sc)) {
                PERROR("Error while creating arena scene.");
                return 1;
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

void game_state_call_tick(game_state *gs) {
    render_obj *robj;
    iterator it;
    vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        object_tick(robj->obj);
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

void game_state_tick(game_state *gs) {
    // Tick timers
    ticktimer_run(gs->tick_timer);

    // We want to load another scene
    if(gs->this_id != gs->next_id) {
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
    }

    // Tick controllers
    game_state_tick_controllers(gs);

    // Poll input. If console is opened, do not poll the controllers.
    if(!console_window_is_open()) { scene_input_poll(gs->sc); }

    // Tick scene
    scene_tick(gs->sc);

    // Clean up objects
    game_state_cleanup(gs);

    // Call object_move for all objects
    game_state_call_move(gs);

    // Handle physics for all pairs of objects
    game_state_call_collide(gs);

    // Tick all objects
    game_state_call_tick(gs);

    // Free extra controller events
    game_state_ctrl_events_free(gs);

    // Increment tick
    gs->tick++;
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
    
    // Free players
    for(int i = 0; i < 2; i++) {
        game_player_free(gs->players[i]);
        free(gs->players[i]);
    }

    // Free scene
    scene_free(gs->sc);
    free(gs->sc);

    // Free ticktimer
    ticktimer_close(gs->tick_timer);
    free(gs->tick_timer);
}

int game_state_ms_per_tick(game_state *gs) {
    switch(gs->this_id) {
        case SCENE_ARENA0:
        case SCENE_ARENA1:
        case SCENE_ARENA2:
        case SCENE_ARENA3:
        case SCENE_ARENA4:
            return MS_PER_OMF_TICK_SLOWEST / gs->speed;
    }
    return MS_PER_OMF_TICK;
}

ticktimer *game_state_get_ticktimer(game_state *gs) {
    return gs->tick_timer;
}

int game_state_serialize(game_state *gs, serial *ser) {
    // serialize tick time and random seed, so client can reply state from this point
    serial_write_int32(ser, game_state_get_tick(gs));
    serial_write_int32(ser, rand_get_seed());

    object *har[2];
    har[0] = game_state_get_player(gs, 0)->har;
    har[1] = game_state_get_player(gs, 1)->har;

    serial *harser;

    harser = object_get_last_serialization_point(har[0]);
    serial_write(ser, harser->data, harser->len);
    harser = object_get_last_serialization_point(har[1]);
    serial_write(ser, harser->data, harser->len);


    /*object_serialize(har[0], ser);*/
    /*object_serialize(har[1], ser);*/
    DEBUG("scene serialized to %d bytes", serial_len(ser));
    return 0;
}

int game_state_unserialize(game_state *gs, serial *ser, int rtt) {
#ifdef DEBUGMODE
    int oldtick = gs->tick;
#endif
    gs->tick = serial_read_int32(ser);
    int endtick = gs->tick + (rtt / 2);
    rand_seed(serial_read_int32(ser));

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
        game_state_add_object(gs, obj, RENDER_LAYER_MIDDLE);

        // Set HAR for player
        game_player_set_har(player, obj);
        game_player_get_ctrl(player)->har = obj;

        // XXX this is a hack because the arena holds the player palette!
        // after this function returns, the arena will restore the palette
        // but we need it when we replay time, apparently
        object_set_palette(obj, bk_get_palette(&gs->sc->bk_data, 0), 0);

    }
    // tick things back to the current time
    DEBUG("replaying %d ticks", endtick - gs->tick);
    DEBUG("adjusting clock from %d to %d (%d)", oldtick, endtick, rtt / 2);
    while (gs->tick <= endtick) {
        game_state_cleanup(gs);
        game_state_call_move(gs);
        game_state_call_collide(gs);
        game_state_call_tick(gs);
        gs->tick++;
    }

    return 0;
}

int game_state_rewind(game_state *gs, int rtt) {
    int ticks = rtt/2;
    gs->tick -= ticks;
    if (ticks > OBJECT_EVENT_BUFFER_SIZE) {
        // too stale, reject it
        return 1;
    }

    object *har[2];
    har[0] = game_state_get_player(gs, 0)->har;
    har[1] = game_state_get_player(gs, 1)->har;

    if (object_get_age(har[0]) < ticks || object_get_age(har[1]) < ticks) {
        // event is older than our HARs, should not be possible, so drop it as invalid
        return 1;
    }

    /*render_obj *robj;*/
    /*iterator it;*/

    // cull any non-hars younger than 'ticks', rewind the others
    /*vector_iter_begin(&gs->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        if (robj->obj == har[0] || robj->obj == har[1]) {
            // TODO handle other scene objects here
            if(object_get_age(robj->obj) >= ticks) {
                object *obj = malloc(sizeof(object));
                object_create(obj, gs, vec2i_create(0, 0), vec2f_create(0,0));
                object_unserialize(obj, , gs);

                game_state_add_object(gs, obj, robj->layer);
            }
            object_free(robj->obj);
            free(robj->obj);
            vector_delete(&gs->objects, &it);
        }
    }*/
    for(int i = 0; i < 2; i++) {
        // Declare some vars
        game_player *player = game_state_get_player(gs, i);
        object *oldobject = player->har;
        serial *ser = object_get_serialization_point(player->har, ticks);
        if (ser == NULL || ser->data == NULL) {
            DEBUG("holy shit, event buffer has a NULL entry for this!");
            continue;
        }
        object *obj = malloc(sizeof(object));

        // Create object and specialize it as HAR.
        // Errors are unlikely here, but check anyway.

        object_create(obj, gs, vec2i_create(0, 0), vec2f_create(0,0));
        object_unserialize(obj, ser, gs);

        game_state_add_object(gs, obj, RENDER_LAYER_MIDDLE);

        // copy over the ringbuffer of actions
        har_copy_actions(obj, oldobject);

        // Set HAR for player and controller
        game_player_set_har(player, obj);
        game_player_get_ctrl(player)->har = obj;

        // XXX this is a hack because the arena holds the player palette!
        // after this function returns, the arena will restore the palette
        // but we need it when we replay time, apparently
        object_set_palette(obj, bk_get_palette(&gs->sc->bk_data, 0), 0);

        game_state_del_object(gs, oldobject);

    }

    return 0;
}

void game_state_replay(game_state *gs, int rtt) {
    int ticks = rtt/2;
    int endtick = gs->tick + ticks;

    // TODO we need to replay the non-client HAR correctly here, right now we discard all inputs received in the last 'ticks' ticks

    DEBUG("replaying %d ticks", ticks);
    DEBUG("adjusting clock from %d to %d (%d)", gs->tick, endtick, ticks);
    while (gs->tick <= endtick) {
        game_state_cleanup(gs);
        game_state_call_move(gs);
        game_state_call_collide(gs);
        game_state_call_tick(gs);
        gs->tick++;
    }
}


