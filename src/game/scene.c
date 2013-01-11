#include "game/scene.h"
#include "utils/log.h"
#include "game/scenes/intro.h"
#include "video/video.h"
#include "audio/music.h"
#include "audio/soundloader.h"
#include <SDL2/SDL.h>
#include <shadowdive/shadowdive.h>

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
    scene->ticks = 0;
    
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
    
    // All done
    DEBUG("Scene %i loaded!", scene_id);
    return 0;
}

// Return 0 if event was handled here
int scene_handle_event(scene *scene, SDL_Event *event) {
    return 1;
}

void scene_render(scene *scene) {

}

void scene_free(scene *scene) {
    if(!scene) return;
    video_set_background(0);
    sd_bk_delete(scene->bk);
}