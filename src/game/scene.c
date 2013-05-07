#include <SDL2/SDL.h>
#include <shadowdive/shadowdive.h>
#include "utils/array.h"
#include "utils/log.h"
#include "video/texture.h"
#include "video/texturelist.h"
#include "video/video.h"
#include "audio/sound.h"
#include "audio/sound_state.h"
#include "game/scene.h"
#include "game/scenes/intro.h"
#include "game/scenes/mainmenu.h"
#include "game/scenes/credits.h"
#include "game/scenes/arena.h"
#include "game/scenes/melee.h"
#include "game/animation.h"
#include "game/animationplayer.h"

// Internal functions
void scene_add_ani_player(void *userdata, int id, int mx, int my);
void scene_set_ani_finished(void *userdata, int id);
void fixup_palette(sd_palette *palette);

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
        case SCENE_MELEE:
            fixup_palette(scene->bk->palettes[0]);
            melee_load(scene); break;
        case SCENE_ARENA0:
        case SCENE_ARENA1:
        case SCENE_ARENA2:
        case SCENE_ARENA3:
        case SCENE_ARENA4:
        case SCENE_ARENA5:
            fixup_palette(scene->bk->palettes[0]);
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
    DEBUG("Scene %i loaded! Textures now using %d bytes of (v)ram!", scene_id, texturelist_get_bsize());
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

    // Render hars
    if(scene->player1.har != NULL) {
        har_render(scene->player1.har);
    }
    if(scene->player2.har != NULL) {
        har_render(scene->player2.har);
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
    
    // Har ticks
    if(scene->player1.har != NULL) {
        har_tick(scene->player1.har);
    }
    if(scene->player2.har != NULL) {
        har_tick(scene->player2.har);
    }
    
        
    // If no animations to play, jump to next scene (if any)
    // TODO: Hackish, make this nicer.
    if(list_size(&scene->root_players) <= 0 && scene->this_id != SCENE_MELEE) {
        if (scene->this_id == SCENE_CREDITS) {
            scene->next_id = SCENE_NONE;
        } else {
            scene->next_id = SCENE_MENU;
        }
        DEBUG("NEXT ID!");
    }
}

void scene_set_player1_har(scene *scene, har *har) {
    if(scene->player1.har != NULL) 
        free(scene->player1.har);
    scene->player1.har = har;
}

void scene_set_player2_har(scene *scene, har *har) {
    if(scene->player2.har != NULL) 
        free(scene->player2.har);
    scene->player2.har = har;
}

void scene_set_player1_ctrl(scene *scene, controller *ctrl) {
    if(scene->player1.ctrl != NULL) 
        free(scene->player1.ctrl);
    scene->player1.ctrl = ctrl;
}

void scene_set_player2_ctrl(scene *scene, controller *ctrl) {
    if(scene->player2.ctrl != NULL) 
        free(scene->player2.ctrl);
    scene->player2.ctrl = ctrl;
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

    // XXX do NOT free hars/controllers here!

    // Free BK
    sd_bk_delete(scene->bk);
}

void fixup_palette(sd_palette *palette) {
    // XXX just cram some known-good values in the player part of the palette for now
    
    /*palette->data[0][0] = 147;*/
    /*palette->data[0][1] = 0;*/
    /*palette->data[0][2] = 0;*/

    palette->data[1][0] = 0;
    palette->data[1][1] = 7;
    palette->data[1][2] = 15;

    palette->data[2][0] = 0;
    palette->data[2][1] = 11;
    palette->data[2][1] = 31;

    palette->data[3][0] = 0;
    palette->data[3][1] = 15;
    palette->data[3][2] = 51;

    palette->data[4][0] = 0;
    palette->data[4][1] = 23;
    palette->data[4][2] = 67;

    palette->data[4][0] = 0;
    palette->data[4][1] = 23;
    palette->data[4][2] = 67;

    palette->data[5][0] = 0;
    palette->data[5][1] = 23;
    palette->data[5][2] = 67;

    palette->data[6][0] = 0;
    palette->data[6][1] = 31;
    palette->data[6][2] = 99;

    palette->data[7][0] = 0;
    palette->data[7][1] = 39;
    palette->data[7][2] = 119;

    palette->data[8][0] = 0;
    palette->data[8][1] = 43;
    palette->data[8][2] = 135;

    palette->data[9][0] = 0;
    palette->data[9][1] = 51;
    palette->data[9][2] = 151;

    palette->data[10][0] = 0;
    palette->data[10][1] = 55;
    palette->data[10][2] = 167;

    palette->data[11][0] = 0;
    palette->data[11][1] = 63;
    palette->data[11][2] = 187;

    palette->data[12][0] = 0;
    palette->data[12][1] = 67;
    palette->data[12][2] = 203;

    palette->data[13][0] = 0;
    palette->data[13][1] = 75;
    palette->data[13][2] = 219;

    palette->data[14][0] = 0;
    palette->data[14][1] = 79;
    palette->data[14][2] = 235;

    palette->data[15][0] = 0;
    palette->data[15][1] = 87;
    palette->data[15][2] = 255;

    palette->data[16][0] = 21;
    palette->data[16][1] = 10;
    palette->data[16][2] = 5;

    palette->data[17][0] = 15;
    palette->data[17][1] = 15;
    palette->data[17][2] = 7;

    palette->data[18][0] = 27;
    palette->data[18][1] = 31;
    palette->data[18][2] = 19;

    palette->data[19][0] = 43;
    palette->data[19][1] = 51;
    palette->data[19][2] = 27;

    palette->data[20][0] = 59;
    palette->data[20][1] = 67;
    palette->data[20][2] = 35;

    palette->data[21][0] = 71;
    palette->data[21][1] = 83;
    palette->data[21][2] = 47;

    palette->data[22][0] = 89;
    palette->data[22][1] = 99;
    palette->data[22][2] = 55;

    palette->data[23][0] = 102;
    palette->data[23][1] = 119;
    palette->data[23][2] = 67;

    palette->data[24][0] = 119;
    palette->data[24][1] = 135;
    palette->data[24][2] = 75;

    palette->data[25][0] = 131;
    palette->data[25][1] = 151;
    palette->data[25][2] = 83;

    palette->data[26][0] = 147;
    palette->data[26][1] = 167;
    palette->data[26][2] = 95;

    palette->data[27][0] = 163;
    palette->data[27][1] = 187;
    palette->data[27][2] = 103;

    palette->data[28][0] = 175;
    palette->data[28][1] = 203;
    palette->data[28][2] = 111;

    palette->data[29][0] = 191;
    palette->data[29][1] = 219;
    palette->data[29][2] = 123;

    palette->data[30][0] = 207;
    palette->data[30][1] = 235;
    palette->data[30][2] = 131;

    palette->data[31][0] = 223;
    palette->data[31][1] = 255;
    palette->data[31][2] = 143;

    palette->data[32][0] = 15;
    palette->data[32][1] = 16;
    palette->data[32][2] = 53;

    palette->data[33][0] = 15;
    palette->data[33][1] = 23;
    palette->data[33][2] = 59;

    palette->data[34][0] = 19;
    palette->data[34][1] = 27;
    palette->data[34][2] = 63;

    palette->data[35][0] = 23;
    palette->data[35][1] = 31;
    palette->data[35][2] = 71;

    palette->data[36][0] = 23;
    palette->data[36][1] = 35;
    palette->data[36][2] = 75;

    palette->data[37][0] = 27;
    palette->data[37][1] = 43;
    palette->data[37][2] = 87;

    palette->data[38][0] = 35;
    palette->data[38][1] = 55;
    palette->data[38][2] = 95;

    palette->data[39][0] = 43;
    palette->data[39][1] = 67;
    palette->data[39][2] = 107;

    palette->data[40][0] = 51;
    palette->data[40][1] = 83;
    palette->data[40][2] = 110;

    palette->data[41][0] = 63;
    palette->data[41][1] = 95;
    palette->data[41][2] = 131;

    palette->data[42][0] = 71;
    palette->data[42][1] = 111;
    palette->data[42][2] = 143;

    palette->data[43][0] = 83;
    palette->data[43][1] = 127;
    palette->data[43][2] = 155;

    palette->data[44][0] = 95;
    palette->data[44][1] = 143;
    palette->data[44][2] = 167;

    palette->data[45][0] = 107;
    palette->data[45][1] = 159;
    palette->data[45][2] = 179;

    palette->data[46][0] = 143;
    palette->data[46][1] = 203;
    palette->data[46][2] = 211;

    palette->data[47][0] = 186;
    palette->data[47][1] = 240;
    palette->data[47][2] = 239;
}
