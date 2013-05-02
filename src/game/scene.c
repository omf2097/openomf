#include <SDL2/SDL.h>
#include <shadowdive/shadowdive.h>
#include "utils/array.h"
#include "utils/log.h"
#include "video/texture.h"
#include "video/video.h"
#include "audio/sound.h"
#include "audio/sound_state.h"
#include "game/scene.h"
#include "game/scenes/intro.h"
#include "game/scenes/mainmenu.h"
#include "game/scenes/credits.h"
#include "game/scenes/arena.h"
#include "game/animation.h"
#include "game/animationplayer.h"

// Internal functions
void scene_add_ani_player(void *userdata, int id, int mx, int my);
void scene_set_ani_finished(void *userdata, int id);

// Loads BK file etc.
int scene_load(scene *scene, unsigned int scene_id) {
    scene->bk = sd_bk_create();
    scene->loop = 1;
    int ret = 0;
    
    // Load BK
    switch(scene_id) {
        case SCENE_INTRO:    ret = sd_bk_load(scene->bk, "resources/INTRO.BK");    break;
        case SCENE_MENU:     ret = sd_bk_load(scene->bk, "resources/MAIN.BK");     break;
        case SCENE_ARENA0:   ret = sd_bk_load(scene->bk, "resources/ARENA0.BK");   break;
        case SCENE_ARENA1:   ret = sd_bk_load(scene->bk, "resources/ARENA1.BK");   break;
        case SCENE_ARENA2:   ret = sd_bk_load(scene->bk, "resources/ARENA2.BK");   break;
        case SCENE_ARENA3:   ret = sd_bk_load(scene->bk, "resources/ARENA3.BK");   break;
        case SCENE_ARENA4:   ret = sd_bk_load(scene->bk, "resources/ARENA4.BK");   break;
        case SCENE_ARENA5:   ret = sd_bk_load(scene->bk, "resources/ARENA5.BK");   break;
        case SCENE_NEWSROOM: ret = sd_bk_load(scene->bk, "resources/NEWSROOM.BK"); break;
        case SCENE_END:      ret = sd_bk_load(scene->bk, "resources/END.BK");      break;
        case SCENE_END1:     ret = sd_bk_load(scene->bk, "resources/END1.BK");     break;
        case SCENE_END2:     ret = sd_bk_load(scene->bk, "resources/END2.BK");     break;
        case SCENE_CREDITS:  ret = sd_bk_load(scene->bk, "resources/CREDITS.BK");  break;
        case SCENE_MELEE:    ret = sd_bk_load(scene->bk, "resources/MELEE.BK");    break;
        case SCENE_VS:       ret = sd_bk_load(scene->bk, "resources/VS.BK");       break;
        default:
            sd_bk_delete(scene->bk);
            PERROR("Unknown scene_id!");
            return 1;
    }
    if(ret) {
        sd_bk_delete(scene->bk);
        PERROR("Unable to load BK file!");
        return 1;
    }
    scene->this_id = scene_id;
    scene->next_id = scene_id;
    
    // Load specific stuff
    switch(scene_id) {
        case SCENE_INTRO: intro_load(scene); break;
        case SCENE_MENU: mainmenu_load(scene); break;
        case SCENE_CREDITS: credits_load(scene); break;
        
        case SCENE_ARENA0:
        case SCENE_ARENA1:
        case SCENE_ARENA2:
        case SCENE_ARENA3:
        case SCENE_ARENA4:
        case SCENE_ARENA5:
            arena_load(scene); 
            break;
            
        default: 
            scene->render = NULL;
            scene->event = NULL;
            scene->init = NULL;
            scene->deinit = NULL;
    }
    
    // Init scene
    if(scene->init != NULL) {
        if(scene->init(scene)) {
            sd_bk_delete(scene->bk);
            return 1;
        }
    }
    
    // Convert background
    sd_rgba_image *bg = sd_vga_image_decode(scene->bk->background, scene->bk->palettes[0], -1);
    texture_create(&scene->background, bg->data, bg->w, bg->h);
    sd_rgba_image_delete(bg);
    
    // Players list
    list_create(&scene->child_players);
    list_create(&scene->root_players);
    
    // Handle animations
    animation *ani;
    sd_bk_anim *bka;
    array_create(&scene->animations);
    for(unsigned int i = 0; i < 50; i++) {
        bka = scene->bk->anims[i];
        if(bka) {
            // Create animation + textures, etc.
            ani = malloc(sizeof(animation));
            animation_create(ani, bka->animation, scene->bk->palettes[0], -1, scene->bk->soundtable);
            array_set(&scene->animations, i, ani);
            
            // Start playback on those animations, that have load_on_start flag as true 
            // or if we are handling animation 25 of intro
            // TODO: Maybe make the exceptions a bit more generic or something ?
            // TODO check other probabilites here
            if(bka->load_on_start || bka->probability == 1 || (scene_id == SCENE_INTRO && i == 25)) {
                animationplayer player;
                player.x = ani->sdani->start_x;
                player.y = ani->sdani->start_y;
                animationplayer_create(&player, i, ani);
                player.userdata = scene;
                player.add_player = scene_add_ani_player;
                player.del_player = scene_set_ani_finished;
                list_append(&scene->root_players, &player, sizeof(animationplayer));
                DEBUG("Create animation %d @ x,y = %d,%d", i, player.x, player.y);
            }
        }
    }
    
    // All done
    DEBUG("Scene %i loaded!", scene_id);
    return 0;
}

void scene_add_ani_player(void *userdata, int id, int mx, int my) {
    scene *sc = userdata;
    animation *ani = array_get(&sc->animations, id);
    if(ani != NULL) {
        animationplayer np;
        np.x = ani->sdani->start_x + mx;
        np.y = ani->sdani->start_y + my;
        animationplayer_create(&np, id, ani);
        np.userdata = userdata;
        np.add_player = scene_add_ani_player;
        np.del_player = scene_set_ani_finished;
        list_append(&sc->child_players, &np, sizeof(animationplayer));
        DEBUG("Create animation %d @ x,y = %d,%d", id, np.x, np.y);
        return;
    } 
}

void scene_set_ani_finished(void *userdata, int id) {
    scene *sc = userdata;
    iterator it;
    animationplayer *tmp = 0;
    
    list_iter_begin(&sc->child_players, &it);
    while((tmp = iter_next(&it)) != NULL) {
        if(tmp->id == id) {
            tmp->finished = 1;
            return;
        }
    }
    
    list_iter_begin(&sc->root_players, &it);
    while((tmp = iter_next(&it)) != NULL) {
        if(tmp->id == id) {
            tmp->finished = 1;
            return;
        }
    }
}

void scene_clean_ani_players(void *userdata) {
    scene *sc = (scene*)userdata;
    iterator it;
    animationplayer *tmp = 0;

    list_iter_begin(&sc->child_players, &it);
    while((tmp = iter_next(&it)) != NULL) {
        if(tmp->finished) {
            animationplayer_free(tmp);
            list_delete(&sc->child_players, &it); 
        }
    }
    
    list_iter_begin(&sc->root_players, &it);
    while((tmp = iter_next(&it)) != NULL) {
        if(tmp->finished) {
            // if their probability is 1, go around again
            if (sc->loop && sc->bk->anims[tmp->id]->probability == 1) {
                animationplayer_reset(tmp);
            } else {
                animationplayer_free(tmp);
                list_delete(&sc->root_players, &it);
            }
        }
    }
}

// Return 0 if event was handled here
int scene_handle_event(scene *scene, SDL_Event *event) {
    if(scene->event) {
        scene->event(scene, event);
    }
    return 1;
}

void scene_render(scene *scene) {
    // Render background
    video_render_background(&scene->background);

    // Render objects
    iterator it;
    animationplayer *tmp = 0;
    list_iter_begin(&scene->child_players, &it);
    while((tmp = iter_next(&it)) != NULL) {
        animationplayer_render(tmp);
    }
    list_iter_begin(&scene->root_players, &it);
    while((tmp = iter_next(&it)) != NULL) {
        animationplayer_render(tmp);
    }
    
    // Run custom render function, if defined
    if(scene->render != NULL) {
        scene->render(scene);
    }
    
    // Render hars
    if(scene->player1_har != NULL) {
        har_render(scene->player1_har);
    }
    if(scene->player2_har != NULL) {
        har_render(scene->player2_har);
    }
}

void scene_tick(scene *scene) {
    // Remove finished players
    scene_clean_ani_players(scene);

    // Tick all players
    iterator it;
    animationplayer *tmp = 0;
    list_iter_begin(&scene->child_players, &it);
    while((tmp = iter_next(&it)) != NULL) {
        animationplayer_run(tmp);
    }
    list_iter_begin(&scene->root_players, &it);
    while((tmp = iter_next(&it)) != NULL) {
        animationplayer_run(tmp);
    }
        
    // Run custom tick function, if defined
    if(scene->tick != NULL) {
        scene->tick(scene);
    }
    
    // Har ticks
    if(scene->player1_har != NULL) {
        har_tick(scene->player1_har);
    }
    if(scene->player2_har != NULL) {
        har_tick(scene->player2_har);
    }
    
        
    // If no animations to play, jump to next scene (if any)
    // TODO: Hackish, make this nicer.
    if(list_size(&scene->root_players) <= 0) {
        if (scene->this_id == SCENE_CREDITS) {
            scene->next_id = SCENE_NONE;
        } else {
            scene->next_id = SCENE_MENU;
        }
        DEBUG("NEXT ID!");
    }
}

void scene_set_player1_har(scene *scene, har *har) {
    if(scene->player1_har != NULL) 
        free(scene->player1_har);
    scene->player1_har = har;
}

void scene_set_player2_har(scene *scene, har *har) {
    if(scene->player2_har != NULL) 
        free(scene->player2_har);
    scene->player2_har = har;
}

void scene_set_player1_ctrl(scene *scene, controller *ctrl) {
    if(scene->player1_ctrl != NULL) 
        free(scene->player1_ctrl);
    scene->player1_ctrl = ctrl;
}

void scene_set_player2_ctrl(scene *scene, controller *ctrl) {
    if(scene->player2_ctrl != NULL) 
        free(scene->player2_ctrl);
    scene->player2_ctrl = ctrl;
}


void scene_free(scene *scene) {
    if(!scene) return;
    
    // Deinit scene
    if(scene->deinit != NULL) {
        scene->deinit(scene);
    }
    
    // Release background
    texture_free(&scene->background);
    
    // Free players
    iterator it;
    animationplayer *tmp = 0;
    list_iter_begin(&scene->child_players, &it);
    while((tmp = iter_next(&it)) != NULL) {
        animationplayer_free(tmp);
    }
    list_iter_begin(&scene->root_players, &it);
    while((tmp = iter_next(&it)) != NULL) {
        animationplayer_free(tmp);
    }
    list_free(&scene->child_players);
    list_free(&scene->root_players);
    
    // Free animations
    animation *ani = 0;
    array_iter_begin(&scene->animations, &it);
    while((ani = iter_next(&it)) != 0) {
        animation_free(ani);
        free(ani);
    }
    array_free(&scene->animations);

    // Free hars and their controllers
    scene_set_player1_har(scene, NULL);
    scene_set_player2_har(scene, NULL);
    scene_set_player1_ctrl(scene, NULL);
    scene_set_player2_ctrl(scene, NULL);
    
    // Free BK
    sd_bk_delete(scene->bk);
}
