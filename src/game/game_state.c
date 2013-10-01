#include <stdlib.h>
#include <SDL2/SDL.h>
#include "controller/keyboard.h"
#include "utils/log.h"
#include "resources/ids.h"
#include "game/game_state.h"
#include "game/settings.h"
#include "game/protos/scene.h"
#include "game/protos/object.h"
#include "game/scenes/intro.h"
#include "game/scenes/mainmenu.h"
#include "game/scenes/credits.h"
#include "game/scenes/arena.h"
//#include "game/scenes/mechlab.h"
#include "game/scenes/melee.h"
#include "game/scenes/vs.h"

#define MS_PER_OMF_TICK 10
#define MS_PER_OMF_TICK_SLOWEST 16

game_state *gamestate = NULL;

int game_state_create() {
    gamestate = malloc(sizeof(game_state));
    gamestate->run = 1;
    vector_create(&gamestate->objects, sizeof(object));
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

void game_state_add_object(object *obj) {
    vector_append(&gamestate->objects, obj);

#ifdef DEBUG
    animation *ani = object_get_animation(obj);
    DEBUG("Added animation %i to game_state.", ani->id);
#endif
}

void game_state_del_object(int anim_id) {
    iterator it;
    object *obj;
    vector_iter_begin(&gamestate->objects, &it);
    while((obj = iter_next(&it)) != NULL) {
        animation *ani = object_get_animation(obj);
        if(ani != NULL && ani->id == anim_id) {
            object_free(obj);
            vector_delete(&gamestate->objects, &it);
            DEBUG("Deleted animation %i from game_state.", anim_id);
            return;
        }
    }
    DEBUG("Attempted to delete animation %i from game_state, but no such animation was playing.", anim_id);
}

object* game_state_get_latest_obj() {
    return vector_get(&gamestate->objects, vector_size(&gamestate->objects)-1);
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
    // Render scene background
    scene_render_bg(&gamestate->sc);

    // Render objects
    iterator it;
    object *obj = NULL;
    vector_iter_begin(&gamestate->objects, &it);
    while((obj = iter_next(&it)) != NULL) {
        object_render(obj);
    }

    // Render scene
    scene_render(&gamestate->sc);
}

int game_load_new(int scene_id) {
    // Free old scene
    scene_free(&gamestate->sc);

    object *obj = NULL;
    iterator it;
    vector_iter_begin(&gamestate->objects, &it);
    while((obj = iter_next(&it)) != NULL) {
        object_free(obj);
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

void game_state_tick() {
    // We want to load another scene
    if(gamestate->this_id != gamestate->next_id) {
        // If this is the end, set run to 0 so that engine knows to close here
        if(gamestate->next_id == SCENE_NONE) {
            gamestate->run = 0;
            return;
        }

        // Load up new scene
        if(game_load_new(gamestate->next_id)) {
            PERROR("Error while loading new scene! bail.");
            gamestate->run = 0;
            return;
        }
    }

    // Tick scene
    scene_tick(&gamestate->sc);

    // Tick all objects, clean up if necessary
    object *obj = NULL;
    iterator it;
    vector_iter_begin(&gamestate->objects, &it);
    while((obj = iter_next(&it)) != NULL) {
        if(object_finished(obj)) {
            object_free(obj);
            vector_delete(&gamestate->objects, &it);
            DEBUG("Animation object %d is finished, removing.", obj->cur_animation->id);
        } else {
            object_tick(obj);
        }
    }
}

game_player* game_state_get_player(int player_id) {
    return &gamestate->players[player_id];
}

void game_state_free() {
    // Free objects
    object *obj = NULL;
    iterator it;
    vector_iter_begin(&gamestate->objects, &it);
    while((obj = iter_next(&it)) != NULL) {
        object_free(obj);
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

unsigned int game_state_ms_per_tick() {
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

/*
void game_state_physics_tick(scene *scene) {
    iterator it;
    object *a, *b, **t;
    unsigned int size;

    // Handle movement
    vector_iter_begin(&global_space->objects, &it);
    while((t = iter_next(&it)) != NULL) {
        a = *t;
        if(!a->is_static) {
            a->pos.x += a->vel.x;
            a->pos.y += a->vel.y;
            a->vel.y += a->gravity;
        }
    }
    
    // Check for collisions
    size = vector_size(&global_space->objects);
    for(int i = 0; i < size; i++) {
        a = *((object**)vector_get(&global_space->objects, i));
        for(int k = i+1; k < size; k++) {
            b = *((object**)vector_get(&global_space->objects, k));
            if((a->group != b->group || a->group == OBJECT_NO_GROUP || b->group == OBJECT_NO_GROUP) && 
                a->layers & b->layers) {

                if(a->col_shape != NULL && b->col_shape != NULL) {
                    if(shape_intersect(a->col_shape, vec2f_to_i(a->pos), 
                                       b->col_shape, vec2f_to_i(b->pos))) {

                        // Try calling collision handler for one of the objects
                        if(a->ev_collision != NULL) {
                            a->ev_collision(a,b,a->userdata,b->userdata);
                        } else if(b->ev_collision != NULL) {
                            b->ev_collision(a,b,a->userdata,b->userdata);
                        }
                    }
                }
            }
        }
    }
}

*/
