#include <SDL2/SDL.h>
#include <shadowdive/shadowdive.h>
#include "utils/log.h"
#include "game/text/text.h"
#include "game/text/languages.h"
#include "game/har.h"
#include "game/scene.h"
#include "video/video.h"
#include "game/scenes/vs.h"
#include "game/menu/menu_background.h"

texture player2_background;
animationplayer welder;
animationplayer scientist;
list child_players;
texture arena_select_bg;
int arena;

sd_rgba_image* sub_image(sd_vga_image *image, sd_palette *pal, int x, int y, int w, int h) {
    sd_rgba_image *img = 0;
    sd_rgba_image *out = sd_rgba_image_create(w, h);
    img = sd_vga_image_decode(image, pal, -1);
    for(int i = y; i < y+h; i++) {
        for(int j = x; j < x+w; j++) {
            int offset = (i*image->w*4)+(j*4);
            int local_offset = ((i-y)*w*4)+((j-x)*4);
            out->data[local_offset]   = (char)img->data[offset];
            out->data[local_offset+1] = (char)img->data[offset+1];
            out->data[local_offset+2] = (char)img->data[offset+2];
            out->data[local_offset+3] = (char)img->data[offset+3];
        }
    }
    return out;
}

void vs_add_ani_player(void *userdata, int id, int mx, int my) {
    scene *scene = userdata;
    animation *ani = array_get(&scene->animations, id);
    if(ani != NULL) {
        animationplayer np;
        DEBUG("spawning %id at %d + %d +%d", id, ani->sdani->start_x, mx, welder.x);
        np.x = ani->sdani->start_x + mx + welder.x;
        np.y = ani->sdani->start_y + my + welder.y;
        animationplayer_create(&np, id, ani);
        /*animationplayer_set_direction(&np, har->direction);*/
        /*np.userdata = userdata;*/
        /*np.add_player = vs_add_ani_player;*/
        /*np.del_player = vs_set_ani_finished;*/
        list_append(&child_players, &np, sizeof(animationplayer));
        animationplayer_run(&np);
        DEBUG("Create animation %d @ x,y = %d,%d", id, np.x, np.y);
        return;
    }
}

void vs_set_ani_finished(void *userdata, int id) {
    /*scene *scene = userdata;*/
    iterator it;
    animationplayer *tmp = 0;

    list_iter_begin(&child_players, &it);
    while((tmp = iter_next(&it)) != NULL) {
        if(tmp->id == id) {
            tmp->finished = 1;
            return;
        }
    }
}


int vs_init(scene *scene) {
    // clone the left side of the background image
    sd_rgba_image * out = sub_image(scene->bk->background, scene->bk->palettes[0], 0, 0, 160, 200);

    arena = rand() % 5; // srand was done in melee

    list_create(&child_players);

    texture_create(&player2_background, out->data, 160, 200);

    menu_background2_create(&arena_select_bg, 211, 50);
    sd_rgba_image_delete(out);
    return 0;
}

void vs_post_init(scene *scene) {
    animation *ani;
    ani = array_get(&scene->animations, 7);
    if (ani != NULL) {
        animationplayer_create(&welder, 7, ani);
        welder.x = 90;
        welder.y = 80;
        welder.userdata = scene;
        welder.add_player = vs_add_ani_player;
        welder.del_player = vs_set_ani_finished;
    } else {
        DEBUG("could not load welder animation");
    }

    ani = array_get(&scene->animations, 8);
    if (ani != NULL) {
        animationplayer_create(&scientist, 8, ani);
        scientist.x = 320-114;
        scientist.y = 118;
        scientist.direction = -1;
    } else {
        DEBUG("could not load scientist animation");
    }
}

void vs_deinit(scene *scene) {
    texture_free(&player2_background);
    animationplayer_free(&welder);
    animationplayer_free(&scientist);

    iterator it;
    animationplayer *tmp = 0;
    list_iter_begin(&child_players, &it);
    while((tmp = iter_next(&it)) != NULL) {
        animationplayer_free(tmp);
    }

}

void vs_tick(scene *scene) {
    animationplayer_run(&welder);
    animationplayer_run(&scientist);

    iterator it;
    animationplayer *tmp = 0;
    list_iter_begin(&child_players, &it);
    while((tmp = iter_next(&it)) != NULL) {
        animationplayer_run(tmp);
    }
}

int vs_event(scene *scene, SDL_Event *event) {
    if(event->type == SDL_KEYDOWN) {
        har *h1, *h2;
        switch (event->key.keysym.sym) {
            case SDLK_ESCAPE:
                scene->next_id = SCENE_MELEE;
                break;
            case SDLK_RETURN:
                h1 = malloc(sizeof(har));
                h2 = malloc(sizeof(har));
                har_load(h1, scene->bk->palettes[0], scene->player1.har_id, 60, 190, 1);
                har_load(h2, scene->bk->palettes[0], scene->player2.har_id, 260, 190, -1);
                scene_set_player1_har(scene, h1);
                scene_set_player2_har(scene, h2);
                scene->next_id = SCENE_ARENA0+arena;
                break;
            case SDLK_UP:
            case SDLK_LEFT:
                arena--;
                if (arena < 0) {
                    arena = 4;
                }
                break;
            case SDLK_DOWN:
            case SDLK_RIGHT:
                arena++;
                if (arena > 4) {
                    arena = 0;
                }
                break;
        }
        return 1;
    }
    return 0;
}

void vs_render(scene *scene) {
    // render the right side of the background
    video_render_sprite_flip(&player2_background, 160, 0, BLEND_ALPHA, FLIP_HORIZONTAL);

    animation *ani = array_get(&scene->animations, 5);

    // player 1 HAR
    video_render_sprite_flip(array_get(&ani->sprites, scene->player1.har_id), 160+ani->sdani->sprites[scene->player1.har_id]->pos_x,
            0+ani->sdani->sprites[scene->player1.har_id]->pos_y, BLEND_ALPHA, FLIP_NONE);

    // player 2 HAR
    video_render_sprite_flip(array_get(&ani->sprites, scene->player2.har_id), 160+ (ani->sdani->sprites[scene->player2.har_id]->pos_x * -1) - ani->sdani->sprites[scene->player2.har_id]->img->w,
            0+ani->sdani->sprites[scene->player2.har_id]->pos_y, BLEND_ALPHA, FLIP_HORIZONTAL);

    ani = array_get(&scene->animations, 4);

    // player 1 portrait
    video_render_sprite_flip(array_get(&ani->sprites, scene->player1.player_id), 0,
            200-ani->sdani->sprites[scene->player1.player_id]->img->w, BLEND_ALPHA, FLIP_NONE);

    // player 2 portrait
    video_render_sprite_flip(array_get(&ani->sprites, scene->player2.player_id), 320-ani->sdani->sprites[scene->player2.player_id]->img->w,
            200-ani->sdani->sprites[scene->player1.player_id]->img->w, BLEND_ALPHA, FLIP_HORIZONTAL);


    if (scene->player2.selectable) {
        // arena selection
        video_render_sprite_flip(&arena_select_bg, 55, 150, BLEND_ALPHA, FLIP_NONE);

        ani = array_get(&scene->animations, 3);
        video_render_sprite_flip(array_get(&ani->sprites, arena), 59, 155, BLEND_ALPHA, FLIP_NONE);


        // arena name
        font_render_wrapped(&font_small, lang_get(56+arena), 59+72, 153, (211-72)-4, 0, 255, 0);

        // arena description
        font_render_wrapped(&font_small, lang_get(66+arena), 59+72, 161, (211-72)-4, 0, 255, 0);

    } else {
        font_render_wrapped(&font_small, lang_get(749+(11*scene->player1.player_id)+scene->player2.player_id), 59, 160, 150, 255, 255, 0);
        font_render_wrapped(&font_small, lang_get(870+(11*scene->player2.player_id)+scene->player1.player_id), 320-(59+150), 180, 150, 255, 255, 0);
    }

    // welder & scientist
    animationplayer_render(&welder);
    animationplayer_render(&scientist);

    iterator it;
    animationplayer *tmp = 0;
    list_iter_begin(&child_players, &it);
    while((tmp = iter_next(&it)) != NULL) {
        animationplayer_render(tmp);
    }

    // gantries
    ani = array_get(&scene->animations, 11);
    video_render_sprite_flip(array_get(&ani->sprites, 0), ani->sdani->sprites[0]->pos_x,
            ani->sdani->sprites[0]->pos_y, BLEND_ALPHA, FLIP_NONE);
    video_render_sprite_flip(array_get(&ani->sprites, 0), 320 - (ani->sdani->sprites[0]->pos_x*-1) - ani->sdani->sprites[0]->img->w,
            ani->sdani->sprites[0]->pos_y, BLEND_ALPHA, FLIP_HORIZONTAL);
}

void vs_load(scene *scene) {
    scene->event = vs_event;
    scene->render = vs_render;
    scene->init = vs_init;
    scene->post_init = vs_post_init;
    scene->deinit = vs_deinit;
    scene->tick = vs_tick;
}

