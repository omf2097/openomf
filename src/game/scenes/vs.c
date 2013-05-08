#include <SDL2/SDL.h>
#include <shadowdive/shadowdive.h>
#include "utils/log.h"
#include "game/har.h"
#include "game/scene.h"
#include "video/video.h"
#include "game/scenes/vs.h"

texture player2_background;

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

int vs_init(scene *scene) {
    // clone the left side of the background image
    sd_rgba_image * out = sub_image(scene->bk->background, scene->bk->palettes[0], 0, 0, 160, 200);
    texture_create(&player2_background, out->data, 160, 200);
    sd_rgba_image_delete(out);
    return 0;
}

void vs_deinit(scene *scene) {
    texture_free(&player2_background);
}

void vs_tick(scene *scene) {
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
                har_load(h1, scene->bk->palettes[0], scene->bk->soundtable, scene->player1.har_id, 60, 190, 1);
                har_load(h2, scene->bk->palettes[0], scene->bk->soundtable, scene->player2.har_id, 260, 190, -1);
                scene_set_player1_har(scene, h1);
                scene_set_player2_har(scene, h2);
                scene->next_id = SCENE_ARENA3;
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
}

void vs_load(scene *scene) {
    scene->event = vs_event;
    scene->render = vs_render;
    scene->init = vs_init;
    scene->deinit = vs_deinit;
    scene->tick = vs_tick;
}

