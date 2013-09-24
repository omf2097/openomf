#include "game/scene.h"
#include "video/video.h"

// Loads BK file etc.
int scene_create(scene *scene, unsigned int scene_id) {
    // Load BK
    if(scene_id == SCENE_NONE || bk_load(&scene->bk_data, scene_id)) {
        PERROR("Unable to load BK file!");
        return 1;
    }

    // Set ID
    scene->userdata = NULL;
    scene->free = NULL;
    scene->event = NULL;
    scene->render = NULL;
    scene->tick = NULL;
    scene->act = NULL;
    return 0;
}

// Return 0 if event was handled here
int scene_event(scene *scene, SDL_Event *event) {
    if(scene->event) {
        return scene->event(scene, event);
    }
    return 1;
}

void scene_render(scene *scene) {
    video_render_background(&scene->background->tex);
    if(scene->render != NULL) {
        scene->render(scene);
    }
}

void scene_tick(scene *scene) {
    if(scene->tick != NULL) {
        scene->tick(scene);
    }
}

void scene_act(scene *scene, controller *ctrl, int action) {
    if(scene->act != NULL) {
        scene->act(scene, ctrl, action);
    }
}

void scene_free(scene *scene) {
    if(scene->free != NULL) {
        scene->free(scene);
    }
    bk_free(&scene->bk);
}

int scene_is_valid(int id) {
    switch(id) {
        case SCENE_INTRO:
        case SCENE_MENU:
        case SCENE_ARENA0:
        case SCENE_ARENA1:
        case SCENE_ARENA2:
        case SCENE_ARENA3:
        case SCENE_ARENA4:
        case SCENE_ARENA5:
        case SCENE_NEWSROOM:
        case SCENE_END:
        case SCENE_END1:
        case SCENE_END2: 
        case SCENE_CREDITS:
        case SCENE_MECHLAB:
        case SCENE_MELEE:
        case SCENE_VS:
        case SCENE_NORTHAM:
        case SCENE_KATUSHAI:
        case SCENE_WAR:
        case SCENE_WORLD:
            return 1;
    }
    return 0;
}



