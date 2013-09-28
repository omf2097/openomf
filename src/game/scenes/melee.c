#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <shadowdive/vga_image.h>
#include <shadowdive/sprite_image.h>
#include "utils/log.h"
#include "game/text/text.h"
#include "game/text/languages.h"
#include "audio/music.h"
#include "video/video.h"
#include "game/settings.h"
#include "game/protos/scene.h"
#include "game/protos/object.h"
#include "resources/ids.h"
#include "resources/bk.h"
#include "resources/animation.h"
#include "resources/sprite.h"
#include "game/scenes/melee.h"
#include "game/scenes/progressbar.h"
#include "game/menu/menu_background.h"

#define MAX_STAT 20

struct pilot_t {
    int power, agility, endurance;
};

typedef struct melee_local_t {

    int selection; // 0 for player, 1 for HAR
    int row_a, row_b; // 0 or 1
    int column_a, column_b; // 0-4
    int done_a, done_b; // 0-1


    struct pilot_t pilots[10];

    sprite *harportraits[10];

    progress_bar bar_power[2];
    progress_bar bar_agility[2];
    progress_bar bar_endurance[2];

    int player_id_a;
    int player_id_b;

    texture feh;
    texture bleh;
    texture select_hilight;
    unsigned int ticks;
    unsigned int hartick;
    unsigned int pulsedir;

    object *harplayer_a;
    object *harplayer_b;
} melee_local;

void refresh_pilot_stats(melee_local *local);

/*void melee_switch_animation(scene *scene, object *harplayer, int id, int x, int y) {*/
    /*object *obj = malloc(sizeof(object));*/
    /*object_create(obj, x, y, 0, 0);*/
    /*object_set_gravity(obj, 0.0f);*/
    /*animationplayer_free(harplayer);*/
    /*if (harplayer->pobj) {*/
        /*object_free(harplayer->pobj);*/
        /*free(harplayer->pobj);*/
    /*}*/
    /*animationplayer_create(harplayer, id, array_get(&scene->animations, id), obj);*/
    /*animationplayer_set_repeat(harplayer, 1);*/
    /*animationplayer_run(harplayer);*/
/*}*/

void handle_action(scene *scene, int player, int action);

// extract part of a sprite as a new sprite
// we need this because the HAR portraits are one single sprite, unlike the player portraits
// so we need to chunk them up into individual sprites and strip out the black background
sd_rgba_image* sub_sprite(sprite *sprite, sd_palette *pal, int x, int y, int w, int h) {
    sd_rgba_image *img = 0;
    sd_rgba_image *out = sd_rgba_image_create(w, h);
    img = sd_sprite_image_decode(sprite->raw_sprite, pal, -1);
    for(int i = y; i < y+h; i++) {
        for(int j = x; j < x+w; j++) {
            int sw = ((sd_vga_image*)sprite->raw_sprite)->w;
            int offset = (i*sw*4)+(j*4);
            int local_offset = ((i-y)*w*4)+((j-x)*4);
            out->data[local_offset]   = (char)img->data[offset];
            out->data[local_offset+1] = (char)img->data[offset+1];
            out->data[local_offset+2] = (char)img->data[offset+2];
            if (!out->data[local_offset] && !out->data[local_offset+1] && !out->data[local_offset+2]) {
                // all three colors are black, set the pixel to be transparent!
                out->data[local_offset+3] = 0;
            } else {
                out->data[local_offset+3] = (char)img->data[offset+3];
            }
        }
    }
    sd_rgba_image_delete(img);
    return out;
}

void mask_sprite(sprite *sprite, int x, int y, int w, int h) {
    sd_vga_image *vga = (sd_vga_image*)sprite->raw_sprite;
    for(int i = 0; i < vga->h; i++) {
        for(int j = 0; j < vga->w; j++) {
            int offset = (i*vga->w)+j;
            if ((i < y || i > y+h) || (j < x || j > x+w)) {
                vga->stencil[offset] = 0;
            } else {
                if (vga->data[offset] == -48) {
                    // strip out the black pixels
                    vga->stencil[offset] = 0;
                } else {
                    vga->stencil[offset] = 1;
                }
            }
        }
    }
}

void melee_free(scene *scene) {
    melee_local *local = scene_get_userdata(scene);
    /*if (harplayer_a.pobj) {*/
        /*object_free(harplayer_a.pobj);*/
        /*free(harplayer_a.pobj);*/
    /*}*/
    /*animationplayer_free(&harplayer_a);*/
    /*if (scene->player2.selectable) {*/
        /*if (harplayer_b.pobj) {*/
            /*object_free(harplayer_b.pobj);*/
            /*free(harplayer_b.pobj);*/
        /*}*/
        /*animationplayer_free(&harplayer_b);*/
    /*}*/
    
    
    for(int i = 0; i < 10; i++) {
        sprite_free(local->harportraits[i]);
    }
    

    texture_free(&local->feh);
    texture_free(&local->bleh);
    texture_free(&local->select_hilight);
    for(int i = 0;i < 2;i++) {
        progressbar_free(&local->bar_power[i]);
        progressbar_free(&local->bar_agility[i]);
        progressbar_free(&local->bar_endurance[i]);
    }
    free(local);
}

void melee_tick(scene *scene) {
    melee_local *local = scene_get_userdata(scene);
    if(!local->pulsedir) {
        local->ticks++;
    } else {
        local->ticks--;
    }
    if(local->ticks > 120) {
        local->pulsedir = 1;
    }
    if(local->ticks == 0) {
        local->pulsedir = 0;
    }
    local->hartick++;
    if (local->selection == 1) {
        if(local->hartick > 10) {
            local->hartick = 0;
            /*animationplayer_run(&harplayer_a);*/
            /*if (scene->player2.selectable) {*/
                /*animationplayer_run(&harplayer_b);*/
            /*}*/
        }
    }

}

void refresh_pilot_stats(melee_local *local) {
    int current_a = 5*local->row_a + local->column_a;
    int current_b = 5*local->row_b + local->column_b;
    progressbar_set(&local->bar_power[0], (local->pilots[current_a].power*100)/MAX_STAT);
    progressbar_set(&local->bar_agility[0], (local->pilots[current_a].agility*100)/MAX_STAT);
    progressbar_set(&local->bar_endurance[0], (local->pilots[current_a].endurance*100)/MAX_STAT);
    progressbar_set(&local->bar_power[1], (local->pilots[current_b].power*100)/MAX_STAT);
    progressbar_set(&local->bar_agility[1], (local->pilots[current_b].agility*100)/MAX_STAT);
    progressbar_set(&local->bar_endurance[1], (local->pilots[current_b].endurance*100)/MAX_STAT);
}

void handle_action(scene *scene, int player, int action) {
    game_player *player1 = scene_get_game_player(scene, 0);
    game_player *player2 = scene_get_game_player(scene, 1);
    melee_local *local = scene_get_userdata(scene);
    int *row, *column, *done;
    if (player == 1) {
        DEBUG("event for player 1");
        row = &local->row_a;
        column = &local->column_a;
        done = &local->done_a;
    } else {
        DEBUG("event for player 2");
        row = &local->row_b;
        column = &local->column_b;
        done = &local->done_b;
    }

    if (*done) {
        return;
    }

    switch (action) {
        case ACT_LEFT:
            (*column)--;
            if (*column < 0) {
                *column = 4;
            }
            break;
        case ACT_RIGHT:
            (*column)++;
            if (*column > 4) {
                *column = 0;
            }
            break;
        case ACT_UP:
        case ACT_DOWN:
            *row = *row == 0 ? 1 : 0;
            break;
        case ACT_KICK:
        case ACT_PUNCH:
            *done = 1;
            if (local->done_a && (local->done_b || !player2->selectable)) {
                local->done_a = 0;
                local->done_b = 0;
                if (local->selection == 0) {
                    local->selection = 1;
                    local->player_id_a = 5*local->row_a + local->column_a;
                    local->player_id_b = 5*local->row_b + local->column_b;
                } else {
                    player1->har_id = HAR_JAGUAR + 5*local->row_a+local->column_a;
                    player1->player_id = local->player_id_a;
                    if (player2->selectable) {
                        player2->har_id = HAR_JAGUAR + 5*local->row_b+local->column_b;
                        player2->player_id = local->player_id_b;
                    } else {
                        // randomly pick opponent and HAR
                        srand(time(NULL));
                        player2->har_id = rand() % 10;
                        int i;
                        while((i = rand() % 10) == local->player_id_a) {}
                        player2->player_id = i;
                    }
                    scene_load_new_scene(scene, SCENE_VS);
                }
            }
            break;
    }

    refresh_pilot_stats(local);
    /*if (local->selection == 1) {
        int har_animation_a = (5*local->row_a) + local->column_a + 18;
        if (harplayer_a.id != har_animation_a) {
            melee_switch_animation(scene, &harplayer_a, har_animation_a, 110, 95);
            animationplayer_set_direction(&harplayer_a, 1);
        }
        if (player2->selectable) {
            int har_animation_b = (5*local->row_b) + local->column_b + 18;
            if (harplayer_b.id != har_animation_b) {
                melee_switch_animation(scene, &harplayer_b, har_animation_b, 210, 95);
                animationplayer_set_direction(&harplayer_b, -1);
            }
        }
    }*/
}

int melee_event(scene *scene, SDL_Event *event) {
    melee_local *local = scene_get_userdata(scene);
    if(event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_ESCAPE) {
                if (local->selection == 1) {
                    // restore the player selection
                    local->column_a = local->player_id_a % 5;
                    local->row_a = local->player_id_a / 5;
                    local->column_b = local->player_id_b % 5;
                    local->row_b = local->player_id_b / 5;

                    local->selection = 0;
                    local->done_a = 0;
                    local->done_b = 0;
                } else {
                    scene_load_new_scene(scene, SCENE_MENU);
                }
            } else {
                game_player *player1 = scene_get_game_player(scene, 0);
                game_player *player2 = scene_get_game_player(scene, 1);
                ctrl_event *p1=NULL, *p2 = NULL, *i;
                controller_event(player1->ctrl, event, &p1);
                controller_event(player2->ctrl, event, &p2);
                i = p1;
                if (i) {
                    do {
                        handle_action(scene, 1, i->action);
                    } while((i = i->next));
                    DEBUG("done");
                }
                i = p2;
                if (i) {
                    do {
                        handle_action(scene, 2, i->action);
                    } while((i = i->next));
                    DEBUG("done");
                }
            }
    }
    return 0;
}

void render_highlights(scene *scene) {
    melee_local *local = scene_get_userdata(scene);
    /*game_player *player1 = scene_get_game_player(scene, 1);*/
    game_player *player2 = scene_get_game_player(scene, 1);
    int trans;
    if (player2->selectable && local->row_a == local->row_b && local->column_a == local->column_b) {
        video_render_char(&local->select_hilight, 11 + (62*local->column_a), 115 + (42*local->row_a), color_create(250-local->ticks, 0, 250-local->ticks, 0));
    } else {
        if (player2->selectable) {
            if (local->done_b) {
                trans = 250;
            } else {
                trans = 250 - local->ticks;
            }
            video_render_char(&local->select_hilight, 11 + (62*local->column_b), 115 + (42*local->row_b), color_create(0, 0, trans, 0));
        }
        if (local->done_a) {
                trans = 250;
            } else {
                trans = 250 - local->ticks;
            }
        video_render_char(&local->select_hilight, 11 + (62*local->column_a), 115 + (42*local->row_a), color_create(trans, 0, 0, 0));
    }
}

void melee_render(scene *scene) {
    animation *ani;
    sprite *sprite;
    melee_local *local = scene_get_userdata(scene);
    game_player *player2 = scene_get_game_player(scene, 1);
    int current_a = 5*local->row_a + local->column_a;
    int current_b = 5*local->row_b + local->column_b;

    //palette *mpal = bk_get_palette(&scene->bk_data, 0);

    if (local->selection == 0) {
        video_render_sprite_flip(&local->feh, 70, 0, BLEND_ALPHA, FLIP_NONE);
        video_render_sprite_flip(&local->bleh, 0, 62, BLEND_ALPHA, FLIP_NONE);

        // player bio
        font_render_wrapped(&font_small, lang_get(135+current_a), 4, 66, 152, COLOR_GREEN);
        // player stats
        font_render(&font_small, lang_get(216), 74+27, 4, COLOR_GREEN);
        font_render(&font_small, lang_get(217), 74+19, 22, COLOR_GREEN);
        font_render(&font_small, lang_get(218), 74+12, 40, COLOR_GREEN);
        progressbar_render(&local->bar_power[0]);
        progressbar_render(&local->bar_agility[0]);
        progressbar_render(&local->bar_endurance[0]);

        if (player2->selectable) {
            video_render_sprite_flip(&local->feh, 320-70-local->feh.w, 0, BLEND_ALPHA, FLIP_NONE);
            video_render_sprite_flip(&local->bleh, 320-local->bleh.w, 62, BLEND_ALPHA, FLIP_NONE);
            // player bio
            font_render_wrapped(&font_small, lang_get(135+current_b), 320-local->bleh.w+4, 66, 152, COLOR_GREEN);
            // player stats
            font_render(&font_small, lang_get(216), 320-66-local->feh.w+27, 4, COLOR_GREEN);
            font_render(&font_small, lang_get(217), 320-66-local->feh.w+19, 22, COLOR_GREEN);
            font_render(&font_small, lang_get(218), 320-66-local->feh.w+12, 40, COLOR_GREEN);
            progressbar_render(&local->bar_power[1]);
            progressbar_render(&local->bar_agility[1]);
            progressbar_render(&local->bar_endurance[1]);
        } else {
            // 'choose your pilot'
            font_render_wrapped(&font_small, lang_get(187), 160, 97, 160, COLOR_GREEN);
        }
    }

    ani = &bk_get_info(&scene->bk_data, 5)->ani;
    if (player2->selectable) {
        video_render_sprite_flip(&animation_get_sprite(ani, 0)->tex, 254, 0, BLEND_ALPHA, FLIP_NONE);
    } else {
        video_render_sprite_flip(&animation_get_sprite(ani, 1)->tex, 162, 0, BLEND_ALPHA, FLIP_NONE);
    }

    if (local->selection == 0) {
        // player 1 name
        font_render_wrapped(&font_small, lang_get(20+current_a), 0, 52, 66, COLOR_BLACK);

        if (player2->selectable) {
            // player 2 name
            font_render_wrapped(&font_small, lang_get(20+current_b), 320-66, 52, 66, COLOR_BLACK);
        }

        render_highlights(scene);
        for(int i = 0; i < 10; i++) {
            sprite = animation_get_sprite(&bk_get_info(&scene->bk_data, 3)->ani, i);
            video_render_sprite_flip(&sprite->tex, sprite->pos.x, sprite->pos.y, BLEND_ALPHA, FLIP_NONE);

            if (i == current_a) {
                // render the big portrait
                sprite = animation_get_sprite(&bk_get_info(&scene->bk_data, 4)->ani, i);
                video_render_sprite_flip(&sprite->tex, sprite->pos.x, sprite->pos.y, BLEND_ALPHA, FLIP_NONE);
            }

            if (player2->selectable && i == current_b) {
                // render the big portrait
                sprite = animation_get_sprite(&bk_get_info(&scene->bk_data, 4)->ani, i);
                video_render_sprite_flip(&sprite->tex, 320-(sprite->tex.w + sprite->pos.x), sprite->pos.y, BLEND_ALPHA, FLIP_HORIZONTAL);
            }
        }
    } else {
        // render the stupid unselected HAR portraits before anything
        // so we can render anything else on top of them
        sprite = animation_get_sprite(&bk_get_info(&scene->bk_data, 1)->ani, 0);
        video_render_sprite_flip(&sprite->tex, sprite->pos.x, sprite->pos.y, BLEND_ALPHA, FLIP_NONE);
        render_highlights(scene);

        // currently selected player
        sprite = animation_get_sprite(&bk_get_info(&scene->bk_data, 4)->ani, local->player_id_a);
        video_render_sprite_flip(&sprite->tex, sprite->pos.x, sprite->pos.y, BLEND_ALPHA, FLIP_NONE);

        //currently selected HAR
        sprite = local->harportraits[5*local->row_a + local->column_a];
        video_render_sprite_flip(&sprite->tex, sprite->pos.x, sprite->pos.y, BLEND_ALPHA, FLIP_NONE);
        /*animationplayer_render(&harplayer_a);*/

        // player 1 name
        font_render_wrapped(&font_small, lang_get(20+local->player_id_a), 0, 52, 66, COLOR_BLACK);

        if (player2->selectable) {
            // player 2 name
            font_render_wrapped(&font_small, lang_get(20+local->player_id_b), 320-66, 52, 66, COLOR_BLACK);

            // currently selected player
            sprite = animation_get_sprite(&bk_get_info(&scene->bk_data, 4)->ani, local->player_id_b);
            video_render_sprite_flip(&sprite->tex, 320-(sprite->tex.w + sprite->pos.x),
                    sprite->pos.y, BLEND_ALPHA, FLIP_HORIZONTAL);
            // currently selected HAR
            sprite = local->harportraits[5*local->row_b + local->column_b];
            video_render_sprite_flip(&sprite->tex, sprite->pos.x, sprite->pos.y, BLEND_ALPHA, FLIP_NONE);
            /*animationplayer_render(&harplayer_b);*/
        } else {
            // 'choose your HAR'
            font_render_wrapped(&font_small, lang_get(186), 160, 97, 160, COLOR_GREEN);
        }
    }
}

int melee_create(scene *scene) {
    char bitmap[51*36*4];

    // Init local data
    melee_local *local = malloc(sizeof(melee_local));
    scene_set_userdata(scene, local);

    palette *mpal = bk_get_palette(&scene->bk_data, 0);
    fixup_palette(mpal);

    // TODO read this from MASTER.DAT
    local->pilots[0].power=5;
    local->pilots[0].agility=16;
    local->pilots[0].endurance=9;
    local->pilots[1].power=13;
    local->pilots[1].agility=9;
    local->pilots[1].endurance=8;
    local->pilots[2].power=7;
    local->pilots[2].agility=20;
    local->pilots[2].endurance=4;
    local->pilots[3].power=9;
    local->pilots[3].agility=7;
    local->pilots[3].endurance=15;
    local->pilots[4].power=20;
    local->pilots[4].agility=1;
    local->pilots[4].endurance=8;
    local->pilots[5].power=9;
    local->pilots[5].agility=10;
    local->pilots[5].endurance=11;
    local->pilots[6].power=10;
    local->pilots[6].agility=1;
    local->pilots[6].endurance=20;
    local->pilots[7].power=7;
    local->pilots[7].agility=10;
    local->pilots[7].endurance=13;
    local->pilots[8].power=14;
    local->pilots[8].agility=8;
    local->pilots[8].endurance=8;
    local->pilots[9].power=14;
    local->pilots[9].agility=4;
    local->pilots[9].endurance=12;

    memset(&bitmap, 255, 51*36*4);
    local->ticks = 0;
    local->pulsedir = 0;
    local->selection = 0;
    local->row_a = 0;
    local->column_a = 0;
    local->row_b = 0;
    local->column_b = 4;
    local->done_a = 0;
    local->done_b = 0;

    menu_background2_create(&local->feh, 90, 61);
    menu_background2_create(&local->bleh, 160, 43);
    texture_create(&local->select_hilight);
    texture_init(&local->select_hilight, bitmap, 51, 36);

    // set up the magic controller hooks
    /*if (scene->player1.ctrl->type == CTRL_TYPE_NETWORK) {*/
        /*controller_add_hook(scene->player2.ctrl, scene->player1.ctrl, scene->player1.ctrl->controller_hook);*/
    /*}*/

    /*if (scene->player2.ctrl->type == CTRL_TYPE_NETWORK) {*/
        /*controller_add_hook(scene->player1.ctrl, scene->player2.ctrl, scene->player2.ctrl->controller_hook);*/
    /*}*/

    sprite *full = animation_get_sprite(&bk_get_info(&scene->bk_data, 1)->ani, 0);
    for(int i = 0; i < 10; i++) {
        int row = i / 5;
        int col = i % 5;
        local->harportraits[i] = malloc(sizeof(sprite));
        sprite_create_custom(local->harportraits[i], full->pos, sd_vga_image_clone(full->raw_sprite));
        mask_sprite(local->harportraits[i], 62*col, 42*row, 51, 36);
        sprite_init(local->harportraits[i], mpal, 0);
    }

    // Preinit some animations with a palette
    // TODO use the player's palette for some animations/sprites
    int to_init[5] = {0,3,4,5};
    for(int i = 0; i < 5; i++) {
        animation_init(&bk_get_info(&scene->bk_data, to_init[i])->ani, mpal, 0);
    }

    const color bar_color = color_create(0, 190, 0, 255);
    const color bar_bg_color = color_create(80, 220, 80, 0);
    const color bar_border_color = color_create(0, 96, 0, 255);
    const color bar_top_left_border_color = color_create(0, 255, 0, 255);
    const color bar_bottom_right_border_color = color_create(0, 125, 0, 255);
    progressbar_create(&local->bar_power[0],     74, 12, 20*4, 8, bar_border_color, bar_border_color, bar_bg_color, bar_top_left_border_color, bar_bottom_right_border_color, bar_color, PROGRESSBAR_LEFT);
    progressbar_create(&local->bar_agility[0],   74, 30, 20*4, 8, bar_border_color, bar_border_color, bar_bg_color, bar_top_left_border_color, bar_bottom_right_border_color, bar_color, PROGRESSBAR_LEFT);
    progressbar_create(&local->bar_endurance[0], 74, 48, 20*4, 8, bar_border_color, bar_border_color, bar_bg_color, bar_top_left_border_color, bar_bottom_right_border_color, bar_color, PROGRESSBAR_LEFT);
    progressbar_create(&local->bar_power[1],     320-66-local->feh.w, 12, 20*4, 8, bar_border_color, bar_border_color, bar_bg_color, bar_top_left_border_color, bar_bottom_right_border_color, bar_color, PROGRESSBAR_LEFT);
    progressbar_create(&local->bar_agility[1],   320-66-local->feh.w, 30, 20*4, 8, bar_border_color, bar_border_color, bar_bg_color, bar_top_left_border_color, bar_bottom_right_border_color, bar_color, PROGRESSBAR_LEFT);
    progressbar_create(&local->bar_endurance[1], 320-66-local->feh.w, 48, 20*4, 8, bar_border_color, bar_border_color, bar_bg_color, bar_top_left_border_color, bar_bottom_right_border_color, bar_color, PROGRESSBAR_LEFT);
    for(int i = 0;i < 2;i++) {
        progressbar_set(&local->bar_power[i], 50);
        progressbar_set(&local->bar_agility[i], 50);
        progressbar_set(&local->bar_endurance[i], 50);
    }
    refresh_pilot_stats(local);

    /*memset(&harplayer_a, 0, sizeof(harplayer_a));*/
    /*harplayer_a.pobj = NULL;*/
    /*memset(&harplayer_b, 0, sizeof(harplayer_b));*/
    /*harplayer_b.pobj = NULL;*/

    // Set callbacks
    scene_set_event_cb(scene, melee_event);
    scene_set_render_cb(scene, melee_render);
    scene_set_free_cb(scene, melee_free);
    scene_set_tick_cb(scene, melee_tick);

    // All done
    return 0;
}
