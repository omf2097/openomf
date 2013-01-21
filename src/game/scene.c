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
#include "game/scenes/menu.h"
#include "game/animation.h"
#include "game/animationplayer.h"

// Internal functions
void scene_add_ani_player(scene *scene, animationplayer *player);
void scene_set_ani_finished(scene *scene, int id);

// Loads BK file etc.
int scene_load(scene *scene, unsigned int scene_id) {
    scene->bk = sd_bk_create();
    int ret = 0;
    
    // Load BK
    switch(scene_id) {
        case SCENE_INTRO:    ret = sd_bk_load(scene->bk, "resources/INTRO.BK");    break;
        case SCENE_MENU:     ret = sd_bk_load(scene->bk, "resources/MAIN.BK");     break;
        case SCENE_NEWSROOM: ret = sd_bk_load(scene->bk, "resources/NEWSROOM.BK"); break;
        case SCENE_END:      ret = sd_bk_load(scene->bk, "resources/END.BK");      break;
        case SCENE_END1:     ret = sd_bk_load(scene->bk, "resources/END1.BK");     break;
        case SCENE_END2:     ret = sd_bk_load(scene->bk, "resources/END2.BK");     break;
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
        case SCENE_MENU: menu_load(scene); break;
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
            if(bka->load_on_start || (scene_id == SCENE_INTRO && i == 25)) {
                animationplayer player;
                animationplayer_create(i, &player, ani, &scene->animations);
                player.x = ani->sdani->start_x;
                player.y = ani->sdani->start_y;
                player.scene = scene;
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

void scene_add_ani_player(scene *scene, animationplayer *player) {
    list_append(&scene->child_players, player, sizeof(animationplayer));
}

void scene_set_ani_finished(scene *scene, int id) {
    iterator it;
    animationplayer *tmp = 0;
    
    list_iter_begin(&scene->child_players, &it);
    while((tmp = iter_next(&it)) != NULL) {
        if(tmp->id == id) {
            tmp->finished = 1;
            return;
        }
    }
    
    list_iter_begin(&scene->root_players, &it);
    while((tmp = iter_next(&it)) != NULL) {
        if(tmp->id == id) {
            tmp->finished = 1;
            return;
        }
    }
}

void scene_clean_ani_players(scene *scene) {
    iterator it;
    animationplayer *tmp = 0;

    list_iter_begin(&scene->child_players, &it);
    while((tmp = iter_next(&it)) != NULL) {
        if(tmp->finished) {
            animationplayer_free(tmp);
            list_delete(&scene->child_players, &it); 
        }
    }
    
    list_iter_begin(&scene->root_players, &it);
    while((tmp = iter_next(&it)) != NULL) {
        if(tmp->finished) { 
            animationplayer_free(tmp);
            list_delete(&scene->root_players, &it); 
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
        
    // If no animations to play, jump to next scene (if any)
    // TODO: Hackish, make this nicer.
    if(list_size(&scene->root_players) <= 0) {
        scene->next_id = SCENE_MENU;
        DEBUG("NEXT ID!");
    }
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

    // Free BK
    sd_bk_delete(scene->bk);
}
