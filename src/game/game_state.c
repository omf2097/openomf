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

int game_state_create(game_state *game) {
    game->run = 1;
    vector_create(&game->objects, sizeof(object));
    game->this_id = SCENE_INTRO;
    game->next_id = SCENE_INTRO;
    intro_create(&game->sc);
    for(int i = 0; i < 2; i++) {
        game_player_create(&game->players[i]);
    }
    return 1;
}

void game_state_add_object(game_state *game, object *obj) {
    vector_append(&game->objects, obj);
}

void game_state_set_next(game_state *game, unsigned int next_scene_id) {
    game->next_id = next_scene_id;
}

int game_state_is_running(game_state *game) {
    return game->run;
}

// Return 0 if event was handled here
int game_state_handle_event(game_state *game, SDL_Event *event) {
    if(scene_event(&game->sc, event) == 0) {
        return 0;
    }
    return 1;
}

void game_state_render(game_state *game) {
    // Render scene
    scene_render(&game->sc);

    // Render objects
    iterator it;
    object *obj = NULL;
    vector_iter_begin(&game->objects, &it);
    while((obj = iter_next(&it)) != NULL) {
        object_render(obj);
    }
}

int game_load_new(game_state *game, int scene_id) {
    // Free old scene
    scene_free(&game->sc);

    // Initialize new scene with BK data etc.
    if(scene_create(&game->sc, game, scene_id)) {
        PERROR("Error while loading scene %d.", scene_id);
        return 1;
    }

    // Load scene specifics
    switch(scene_id) {
        case SCENE_INTRO: 
            if(intro_create(&game->sc)) {
                PERROR("Error while creating intro scene.");
                return 1;
            }
            break;
        case SCENE_MENU: 
            if(mainmenu_create(&game->sc)) {
                PERROR("Error while creating mainmenu scene.");
                return 1;
            }
            break;
        case SCENE_CREDITS: 
            if(credits_create(&game->sc)) {
                PERROR("Error while creating credits scene.");
                return 1;
            }
            break;
        case SCENE_MELEE:
            if(melee_create(&game->sc)) {
                PERROR("Error while creating melee scene.");
                return 1;
            } 
            break;
        case SCENE_VS:
            if(vs_create(&game->sc)) {
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
        case SCENE_ARENA5:
            if(arena_create(&game->sc)) {
                PERROR("Error while creating arena scene.");
                return 1;
            } 
            break;
    }
    game->this_id = scene_id;
    game->next_id = scene_id;
    return 0;
}

void game_state_tick(game_state *game) {
    // We want to load another scene
    if(game->this_id != game->next_id) {
        // If this is the end, set run to 0 so that engine knows to close here
        if(game->next_id == SCENE_NONE) {
            game->run = 0;
            return;
        }

        // Load up new scene
        if(game_load_new(game, game->next_id)) {
            PERROR("Error while loading new scene! bail.");
            game->run = 0;
            return;
        }
    }

    // Tick scene
    scene_tick(&game->sc);

    // Tick all objects, clean up if necessary
    object *obj = NULL;
    iterator it;
    vector_iter_begin(&game->objects, &it);
    while((obj = iter_next(&it)) != NULL) {
        if(object_finished(obj)) {
            object_free(obj);
            vector_delete(&game->objects, &it);
        } else {
            object_tick(obj);
        }
    }
}

game_player* game_state_get_player(game_state *game, int player_id) {
    return &game->players[player_id];
}

void game_state_free(game_state *game) {
    // Deinit scene
    scene_free(&game->sc);
    
    // Free objects
    object *obj = NULL;
    iterator it;
    vector_iter_begin(&game->objects, &it);
    while((obj = iter_next(&it)) != NULL) {
        object_free(obj);
        vector_delete(&game->objects, &it);
    }
    
    // Free players
    for(int i = 0; i < 2; i++) {
        game_player_free(&game->players[i]);
    }

    // Free scene
    scene_free(&game->sc);
}

unsigned int game_state_ms_per_tick(game_state *game) {
    switch(game->this_id) {
        case SCENE_ARENA0:
        case SCENE_ARENA1:
        case SCENE_ARENA2:
        case SCENE_ARENA3:
        case SCENE_ARENA4:
        case SCENE_ARENA5:
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