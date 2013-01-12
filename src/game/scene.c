#include "game/scene.h"
#include "game/scenes/intro.h"
#include "game/animation.h"
#include "game/animationplayer.h"
#include "utils/array.h"
#include "utils/log.h"
#include "video/video.h"
#include <SDL2/SDL.h>
#include <shadowdive/shadowdive.h>

void create_new_animation_instance(scene *scene, unsigned int id) {
    animation *ani = array_get(&scene->animations, id);
    if(ani != NULL) {
        animationplayer *player = malloc(sizeof(animationplayer));
        animationplayer_create(id, player, ani);
        player->x = ani->sdani->start_x;
        player->y = ani->sdani->start_y;
        list_push_last(&scene->players, player);
        DEBUG("Create animation %d @ x,y = %d,%d", id, player->x, player->y);
    } else {
        PERROR("Attempted to create an instance of nonexistent animation!");
    }
}

int scene_load(scene *scene, unsigned int scene_id) {
    scene->bk = sd_bk_create();
    int ret = 0;
    
    // Load BK
    switch(scene_id) {
        case SCENE_INTRO: ret = sd_bk_load(scene->bk, "resources/INTRO.BK"); break;
        case SCENE_MENU:  ret = sd_bk_load(scene->bk, "resources/MAIN.BK");  break;
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
    scene->real_ticks = 0;
    scene->omf_ticks = 1;
    
    // Load specific stuff
    switch(scene_id) {
        case SCENE_INTRO: intro_load(scene); break;
        //case SCENE_MENU: menu_load(scene); break;
        default: 
            scene->render = 0;
            scene->event = 0;
    }
    
    // Convert background
    sd_rgba_image *bg = sd_vga_image_decode(scene->bk->background, scene->bk->palettes[0], -1);
    texture_create(&scene->background, bg->data, bg->w, bg->h);
    video_set_background(&scene->background);
    sd_rgba_image_delete(bg);
    
    // Players list
    list_create(&scene->players);
    
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
            array_insert(&scene->animations, i, ani);
            
            // Start playback on those animations, that have load_on_start flag as true 
            // or if we are handling animation 25 of intro
            // TODO: Maybe make the exceptions a bit more generic or something ?
            if(bka->load_on_start || (scene_id == SCENE_INTRO && i == 25)) {
                create_new_animation_instance(scene, i);
            }
        }
    }
    
    // All done
    DEBUG("Scene %i loaded!", scene_id);
    return 0;
}

// Return 0 if event was handled here
int scene_handle_event(scene *scene, SDL_Event *event) {
    if(scene->event) {
        scene->event(scene, event);
    }
    return 1;
}

void scene_render(scene *scene, unsigned int delta) {
    scene->real_ticks += delta;
    list_iterator it;
    animationplayer *player;
    
    int kill_list[list_size(&scene->players)];
    int kill_list_len = 0;
    
    int tmp = (scene->real_ticks / MS_PER_OMF_TICK) - scene->omf_ticks;
    for(int i = 0; i < tmp; i++) {
        // Iterate through the players
        list_iter(&scene->players, &it);
        while((player = list_next(&it)) != 0) {
            animationplayer_run(player);
            
            // Got animation create request
            if(player->anim_create_req >= 0) {
                create_new_animation_instance(scene, player->anim_create_req);
                player->anim_create_req = -1;
            }
            
            // Got animation destroy request
            if(player->anim_destroy_req >= 0) {
                kill_list[kill_list_len++] = player->anim_destroy_req;
                DEBUG("Killing animation %d", player->anim_destroy_req);
                player->anim_destroy_req = -1;
            }
        }
        
        // Kill animations in killist
        unsigned int tmp;
        while(kill_list_len-- >= 0) {
            list_iter(&scene->players, &it);
            tmp = kill_list[kill_list_len-1];
            while((player = list_next(&it)) != 0) {
                if(tmp == player->id) {
                    list_delete(&scene->players, &it);
                    animationplayer_free(player);
                    free(player);
                    DEBUG("Killed animation %d", tmp);
                }
            }
        }
        
        // Run custom render function, if defined
        if(scene->render) {
            scene->render(scene, delta);
        }
        scene->omf_ticks++;
    }
    
    // If no animations to play, jump to next scene (if any)
    // TODO: Hackish, make this nicer.
    if(list_size(&scene->players) <= 0) {
        scene->next_id = SCENE_MENU;
        DEBUG("NEXT ID!");
    }
}

void scene_free(scene *scene) {
    if(!scene) return;
    
    // Release background
    video_set_background(0);
    texture_free(&scene->background);
    
    // Free players
    list_iterator lit;
    animationplayer *player = 0;
    list_iter(&scene->players, &lit);
    while((player = list_next(&lit)) != 0) {
        animationplayer_free(player);
        free(player);
    }
    list_free(&scene->players);
    
    // Free animations
    array_iterator it;
    animation *ani = 0;
    array_iter(&scene->animations, &it);
    while((ani = array_next(&it)) != 0) {
        animation_free(ani);
        free(ani);
    }
    array_free(&scene->animations);

    // Free BK
    sd_bk_delete(scene->bk);
}
