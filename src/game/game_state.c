#include <stdlib.h>
#include <SDL2/SDL.h>
#include "controller/keyboard.h"
#include "utils/log.h"
#include "resources/ids.h"
#include "console/console.h"
#include "game/game_state.h"
#include "game/settings.h"
#include "game/protos/scene.h"
#include "game/protos/object.h"
#include "game/protos/intersect.h"
#include "game/scenes/intro.h"
#include "game/scenes/mainmenu.h"
#include "game/scenes/credits.h"
#include "game/scenes/arena.h"
//#include "game/scenes/mechlab.h"
#include "game/scenes/melee.h"
#include "game/scenes/vs.h"

#define MS_PER_OMF_TICK 10
#define MS_PER_OMF_TICK_SLOWEST 35

game_state *gamestate = NULL;

typedef struct {
    int layer;
    object *obj;
} render_obj;

int game_state_create() {
    gamestate = malloc(sizeof(game_state));
    gamestate->run = 1;
    vector_create(&gamestate->objects, sizeof(render_obj));
    int nscene = SCENE_INTRO;
    if(scene_create(&gamestate->sc, nscene)) {
        PERROR("Error while loading scene %d.", nscene);
        return 1;
    }
    if(intro_create(&gamestate->sc)) {
        scene_free(&gamestate->sc);
        PERROR("Error while creating intro scene.");
        return 1;
    }
    scene_init(&gamestate->sc);
    gamestate->this_id = nscene;
    gamestate->next_id = nscene;
    for(int i = 0; i < 2; i++) {
        game_player_create(&gamestate->players[i]);
    }
    return 0;
}

void game_state_add_object(object *obj, int layer) {
    render_obj o;
    o.obj = obj;
    o.layer = layer;
    vector_append(&gamestate->objects, &o);

#ifdef DEBUGMODE
    animation *ani = object_get_animation(obj);
    DEBUG("Added animation %i to game_state on layer %d.", ani->id, layer);
#endif
}

void game_state_del_animation(int anim_id) {
    iterator it;
    render_obj *robj;
    vector_iter_begin(&gamestate->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        animation *ani = object_get_animation(robj->obj);
        if(ani != NULL && ani->id == anim_id) {
            object_free(robj->obj);
            free(robj->obj);
            vector_delete(&gamestate->objects, &it);
            DEBUG("Deleted animation %i from game_state.", anim_id);
            return;
        }
    }
    DEBUG("Attempted to delete animation %i from game_state, but no such animation was playing.", anim_id);
}

void game_state_del_object(object *target) {
    iterator it;
    render_obj *robj;
    vector_iter_begin(&gamestate->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        if(target == robj->obj) {
            object_free(robj->obj);
            free(robj->obj);
            vector_delete(&gamestate->objects, &it);
            return;
        }
    }
}

void game_state_set_next(unsigned int next_scene_id) {
    gamestate->next_id = next_scene_id;
}

scene* game_state_get_scene() {
    return &gamestate->sc;
}

int game_state_is_running() {
    return gamestate->run;
}

// Return 0 if event was handled here
int game_state_handle_event(SDL_Event *event) {
    if(scene_event(&gamestate->sc, event) == 0) {
        return 0;
    }
    return 1;
}

void game_state_render() {
    iterator it;
    render_obj *robj;

    // Render scene background
    scene_render(&gamestate->sc);

    // Get har objects
    object *har[2];
    har[0] = game_state_get_player(0)->har;
    har[1] = game_state_get_player(1)->har;

    // Render BOTTOM layer
    vector_iter_begin(&gamestate->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        if(robj->layer == RENDER_LAYER_BOTTOM) {
            if(robj->obj == har[0] || robj->obj == har[1]) continue;
            object_render(robj->obj);
        }
    }

    // Render passive HARs here
    for(int i = 0; i < 2; i++) {
        if(har[i] != NULL && !har_is_active(har[i])) {
            object_render(har[i]);
        }
    }

    // Render MIDDLE layer
    vector_iter_begin(&gamestate->objects, &it);
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
    vector_iter_begin(&gamestate->objects, &it);
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

    // Render scene
    scene_render_overlay(&gamestate->sc);
}

int game_load_new(int scene_id) {
    // Free old scene
    scene_free(&gamestate->sc);

    render_obj *robj;
    iterator it;
    vector_iter_begin(&gamestate->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        object_free(robj->obj);
        free(robj->obj);
        vector_delete(&gamestate->objects, &it);
    }

    // Initialize new scene with BK data etc.
    if(scene_create(&gamestate->sc, scene_id)) {
        PERROR("Error while loading scene %d.", scene_id);
        return 1;
    }

    // Load scene specifics
    switch(scene_id) {
        case SCENE_INTRO: 
            if(intro_create(&gamestate->sc)) {
                PERROR("Error while creating intro scene.");
                return 1;
            }
            break;
        case SCENE_MENU: 
            if(mainmenu_create(&gamestate->sc)) {
                PERROR("Error while creating mainmenu scene.");
                return 1;
            }
            break;
        case SCENE_CREDITS: 
            if(credits_create(&gamestate->sc)) {
                PERROR("Error while creating credits scene.");
                return 1;
            }
            break;
        case SCENE_MELEE:
            if(melee_create(&gamestate->sc)) {
                PERROR("Error while creating melee scene.");
                return 1;
            } 
            break;
        case SCENE_VS:
            if(vs_create(&gamestate->sc)) {
                PERROR("Error while creating VS scene.");
                return 1;
            }
            break;
        case SCENE_MECHLAB:
            /*mechlab_load(&game->sc);
            break;*/
        case SCENE_ARENA0:
        case SCENE_ARENA1:
        case SCENE_ARENA2:
        case SCENE_ARENA3:
        case SCENE_ARENA4:
            if(arena_create(&gamestate->sc)) {
                PERROR("Error while creating arena scene.");
                return 1;
            } 
            break;
    }

    // Zap scene to produce objects & background
    scene_init(&gamestate->sc);

    // All done.
    gamestate->this_id = scene_id;
    gamestate->next_id = scene_id;
    return 0;
}

void game_state_call_collide() {
    object *a, *b;
    unsigned int size = vector_size(&gamestate->objects);
    for(int i = 0; i < size; i++) {
        a = ((render_obj*)vector_get(&gamestate->objects, i))->obj;
        for(int k = i+1; k < size; k++) {
            b = ((render_obj*)vector_get(&gamestate->objects, k))->obj;
            if(a->group != b->group || a->group == OBJECT_NO_GROUP || b->group == OBJECT_NO_GROUP) {
                if(a->layers & b->layers) {
                    object_collide(a, b);
                }
            }
        }
    }
}

void game_state_cleanup() {
    render_obj *robj;
    iterator it;
    vector_iter_begin(&gamestate->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        if(object_finished(robj->obj)) {
            DEBUG("Animation object %d is finished, removing.", robj->obj->cur_animation->id);
            object_free(robj->obj);
            free(robj->obj);
            vector_delete(&gamestate->objects, &it);
        }
    }
}

void game_state_call_move() {
    render_obj *robj;
    iterator it;
    vector_iter_begin(&gamestate->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        object_move(robj->obj);
    }
}

void game_state_call_tick() {
    render_obj *robj;
    iterator it;
    vector_iter_begin(&gamestate->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        object_tick(robj->obj);
    }
}

void game_state_tick() {
    // We want to load another scene
    if(gamestate->this_id != gamestate->next_id) {
        // If this is the end, set run to 0 so that engine knows to close here
        if(gamestate->next_id == SCENE_NONE) {
            DEBUG("Next ID is SCENE_NONE! bailing.");
            gamestate->run = 0;
            return;
        }

        // Load up new scene
        if(game_load_new(gamestate->next_id)) {
            PERROR("Error while loading new scene! bailing.");
            gamestate->run = 0;
            return;
        }
    }

    // Tick input. If console is opened, do not tick the controllers.
    if(!console_window_is_open()) { scene_input_tick(&gamestate->sc); }

    // Tick scene
    scene_tick(&gamestate->sc);

    // Clean up objects
    game_state_cleanup();

    // Call object_move for all objects
    game_state_call_move();

    // Handle physics for all pairs of objects
    game_state_call_collide();

    // Tick all objects
    game_state_call_tick();
}

game_player* game_state_get_player(int player_id) {
    return &gamestate->players[player_id];
}

void game_state_free() {
    // Free objects
    render_obj *robj;
    iterator it;
    vector_iter_begin(&gamestate->objects, &it);
    while((robj = iter_next(&it)) != NULL) {
        object_free(robj->obj);
        free(robj->obj);
        vector_delete(&gamestate->objects, &it);
    }
    vector_free(&gamestate->objects);
    
    // Free players
    for(int i = 0; i < 2; i++) {
        game_player_free(&gamestate->players[i]);
    }

    // Free scene
    scene_free(&gamestate->sc);

    // Free up state
    free(gamestate);
}

int game_state_ms_per_tick() {
    switch(gamestate->this_id) {
        case SCENE_ARENA0:
        case SCENE_ARENA1:
        case SCENE_ARENA2:
        case SCENE_ARENA3:
        case SCENE_ARENA4:
            return MS_PER_OMF_TICK_SLOWEST - settings_get()->gameplay.speed;
    }
    return MS_PER_OMF_TICK;
}
