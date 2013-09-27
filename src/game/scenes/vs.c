#include <stdlib.h>
#include <SDL2/SDL.h>
#include <shadowdive/shadowdive.h>
#include "utils/log.h"
#include "game/text/text.h"
#include "game/text/languages.h"
#include "game/protos/scene.h"
#include "video/video.h"
#include "game/scenes/vs.h"
#include "resources/ids.h"
#include "game/menu/menu_background.h"
#include "controller/controller.h"
#include "controller/keyboard.h"


typedef struct vs_local_t {
    texture player2_background;
    /*animationplayer welder;*/
    /*animationplayer scientist;*/
    /*list child_players;*/
    texture arena_select_bg;
    int arena;
} vs_local;

sd_rgba_image* sub_image(sd_vga_image *image, palette *pal, int x, int y, int w, int h) {
    sd_rgba_image *img = 0;
    sd_rgba_image *out = sd_rgba_image_create(w, h);
    img = sd_vga_image_decode(image, (sd_palette*)pal, -1);
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
    sd_rgba_image_delete(img);
    return out;
}

/*void vs_add_ani_player(void *userdata, int id, int mx, int my, int mg) {
    scene *scene = userdata;
    animation *ani = &bk_get_info(&scene->bk_data, id)->ani;
    if(ani != NULL) {
        animationplayer np;
        object *obj = malloc(sizeof(object));
        int welder_x, welder_y;
        object_get_pos(welder.pobj, &welder_x, &welder_y);
        object_set_gravity(obj, 0.0f);
        DEBUG("spawning %id at %d + %d +%d", id, ani->sdani->start_x, mx, welder_x);
        object_create(obj, ani->sdani->start_x + mx + welder_x, ani->sdani->start_y + my + welder_y, 0, 0);
        animationplayer_create(&np, id, ani, obj);
        list_append(&child_players, &np, sizeof(animationplayer));
        animationplayer_run(&np);
        return;
    }
}

void vs_set_ani_finished(void *userdata, int id) {
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



void vs_post_init(scene *scene) {
    animation *ani;
    ani = array_get(&scene->animations, 7);
    if (ani != NULL) {
        object *obj = malloc(sizeof(object));
        object_create(obj, 90, 80, 0, 0);
        object_set_gravity(obj, 0.0f);
        animationplayer_create(&welder, 7, ani, obj);
        welder.userdata = scene;
        welder.add_player = vs_add_ani_player;
        welder.del_player = vs_set_ani_finished;
    } else {
        DEBUG("could not load welder animation");
    }

    ani = array_get(&scene->animations, 8);
    if (ani != NULL) {
        object *obj = malloc(sizeof(object));
        object_create(obj, 320-114, 118, 0, 0);
        object_set_gravity(obj, 0.0f);
        animationplayer_create(&scientist, 8, ani, obj);
        scientist.direction = -1;
    } else {
        DEBUG("could not load scientist animation");
    }
}*/

void vs_free(scene *scene) {
    vs_local *local = scene_get_userdata(scene);
    texture_free(&local->player2_background);
    texture_free(&local->arena_select_bg);
}

void vs_handle_action(scene *scene, int action) {
    vs_local *local = scene_get_userdata(scene);
    switch (action) {
        case ACT_KICK:
        case ACT_PUNCH:
            scene_load_new_scene(scene, SCENE_ARENA0+local->arena);
            break;
        case ACT_UP:
        case ACT_LEFT:
            local->arena--;
            if (local->arena < 0) {
                local->arena =4;
            }
            break;
        case ACT_DOWN:
        case ACT_RIGHT:
            local->arena++;
            if (local->arena > 4) {
                local->arena = 0;
            }
            break;
    }
}

void vs_tick(scene *scene) {
    game_player *player1 = scene_get_game_player(scene, 0);
    game_player *player2 = scene_get_game_player(scene, 1);
    ctrl_event *p1 = NULL, *p2 = NULL, *i;
    if(controller_tick(player1->ctrl, &p1) ||
            controller_tick(player2->ctrl, &p2)) {
        // one of the controllers bailed

        /*if(scene->player1.ctrl->type == CTRL_TYPE_NETWORK) {*/
            /*net_controller_free(scene->player1.ctrl);*/
        /*}*/

        /*if(scene->player2.ctrl->type == CTRL_TYPE_NETWORK) {*/
            /*net_controller_free(scene->player2.ctrl);*/
        /*}*/
        scene_load_new_scene(scene, SCENE_MENU);
    }
    i = p1;
    if (i) {
        do {
            vs_handle_action(scene, i->action);
        } while((i = i->next));
    }
}

int vs_event(scene *scene, SDL_Event *event) {
    if(event->type == SDL_KEYDOWN) {
        if(event->key.keysym.sym == SDLK_ESCAPE) {
                scene_load_new_scene(scene, SCENE_MELEE);
        } else {
            ctrl_event *p1=NULL, *i;
            game_player *player1 = scene_get_game_player(scene, 0);
            controller_event(player1->ctrl, event, &p1);
            i = p1;
            if (i) {
                do {
                    vs_handle_action(scene, i->action);
                } while((i = i->next));
                DEBUG("done");
            }
        }
        return 1;
    }
    return 0;
}

void vs_render(scene *scene) {
    vs_local *local = scene_get_userdata(scene);

    palette *mpal = bk_get_palette(&scene->bk_data, 0);

    // render the right side of the background
    video_render_sprite_flip(&local->player2_background, 160, 0, BLEND_ALPHA, FLIP_HORIZONTAL);

    game_player *player1 = scene_get_game_player(scene, 0);
    game_player *player2 = scene_get_game_player(scene, 1);

    // player 1 HAR

    sprite *sprite = animation_get_sprite(&bk_get_info(&scene->bk_data, 5)->ani, player1->har_id);
    sprite_init(sprite, mpal, 0);
    video_render_sprite_flip(&sprite->tex, 160+sprite->pos.x, sprite->pos.y, BLEND_ALPHA, FLIP_NONE);

    // player 2 HAR
    sprite = animation_get_sprite(&bk_get_info(&scene->bk_data, 5)->ani, player1->har_id);
    sprite_init(sprite, mpal, 0);
    video_render_sprite_flip(&sprite->tex, 160+ (sprite->pos.x * -1) - sprite->tex.w, sprite->pos.y, BLEND_ALPHA, FLIP_HORIZONTAL);

    // player 1 portrait
    sprite = animation_get_sprite(&bk_get_info(&scene->bk_data, 4)->ani, player1->player_id);
    sprite_init(sprite, mpal, 0);
    video_render_sprite_flip(&sprite->tex, 0, 200 - sprite->tex.w, BLEND_ALPHA, FLIP_NONE);

    // player 2 portrait
    sprite = animation_get_sprite(&bk_get_info(&scene->bk_data, 4)->ani, player2->player_id);
    sprite_init(sprite, mpal, 0);
    video_render_sprite_flip(&sprite->tex, 320 - sprite->tex.w, 200 - sprite->tex.w, BLEND_ALPHA, FLIP_HORIZONTAL);


    if (player2->selectable) {
        // arena selection
        video_render_sprite_flip(&local->arena_select_bg, 55, 150, BLEND_ALPHA, FLIP_NONE);

        sprite = animation_get_sprite(&bk_get_info(&scene->bk_data, 3)->ani, local->arena);
        sprite_init(sprite, mpal, 0);
        video_render_sprite_flip(&sprite->tex, 59, 155, BLEND_ALPHA, FLIP_NONE);


        // arena name
        font_render_wrapped(&font_small, lang_get(56+local->arena), 59+72, 153, (211-72)-4, COLOR_GREEN);

        // arena description
        font_render_wrapped(&font_small, lang_get(66+local->arena), 59+72, 161, (211-72)-4, COLOR_GREEN);

    } else {
        font_render_wrapped(&font_small, lang_get(749+(11*player1->player_id)+player2->player_id), 59, 160, 150, COLOR_YELLOW);
        font_render_wrapped(&font_small, lang_get(870+(11*player2->player_id)+player1->player_id), 320-(59+150), 180, 150, COLOR_YELLOW);
    }

    // welder & scientist
    /*animationplayer_render(&welder);*/
    /*animationplayer_render(&scientist);*/

    /*iterator it;*/
    /*animationplayer *tmp = 0;*/
    /*list_iter_begin(&child_players, &it);*/
    /*while((tmp = iter_next(&it)) != NULL) {*/
        /*animationplayer_render(tmp);*/
    /*}*/

    // gantries
    sprite = animation_get_sprite(&bk_get_info(&scene->bk_data, 11)->ani, 0);
    sprite_init(sprite, mpal, 0);
    video_render_sprite_flip(&sprite->tex, sprite->pos.x, sprite->pos.y, BLEND_ALPHA, FLIP_NONE);
    video_render_sprite_flip(&sprite->tex, 320 - (sprite->pos.x*-1) - sprite->tex.w, sprite->pos.y, BLEND_ALPHA, FLIP_HORIZONTAL);
}

int vs_create(scene *scene) {
    // Init local data
    vs_local *local = malloc(sizeof(vs_local));
    scene_set_userdata(scene, local);

    palette *mpal = bk_get_palette(&scene->bk_data, 0);
    fixup_palette(mpal);

    game_player *player2 = scene_get_game_player(scene, 1);

    // clone the left side of the background image
    sd_rgba_image * out = sub_image((sd_vga_image*)scene->bk_data.background.raw_sprite, bk_get_palette(&scene->bk_data, 0), 0, 0, 160, 200);

    if (player2->selectable) {
        // player1 gets to choose, start at arena
        local->arena = 0;
    } else {
        // pick a random arena for 1 player mode
        local->arena = rand() % 5; // srand was done in melee
    }

    texture_create(&local->player2_background, out->data, 160, 200);

    menu_background2_create(&local->arena_select_bg, 211, 50);
    sd_rgba_image_delete(out);

    scene_set_render_cb(scene, vs_render);
    scene_set_event_cb(scene, vs_event);
    scene_set_tick_cb(scene, vs_tick);
    scene_set_free_cb(scene, vs_free);
    return 0;
}

