#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <shadowdive/vga_image.h>
#include <shadowdive/sprite_image.h>

#include "utils/log.h"
#include "utils/random.h"
#include "audio/music.h"
#include "video/video.h"
#include "resources/ids.h"
#include "resources/bk.h"
#include "resources/animation.h"
#include "resources/sprite.h"
#include "game/text/text.h"
#include "game/text/languages.h"
#include "game/settings.h"
#include "game/game_state.h"
#include "game/protos/scene.h"
#include "game/protos/object.h"
#include "game/scenes/melee.h"
#include "game/scenes/progressbar.h"
#include "game/menu/menu_background.h"

#define MAX_STAT 20

struct pilot_t {
    int power, agility, endurance;
    int colors[3];
    object obj;
};

typedef struct melee_local_t {

    int selection; // 0 for player, 1 for HAR
    int row_a, row_b; // 0 or 1
    int column_a, column_b; // 0-4
    int done_a, done_b; // 0-1

    object bigportrait1;
    object bigportrait2;
    object player2_placeholder;
    object unselected_har_portraits;

    struct pilot_t pilots[10];

    object harportraits_player1[10];
    object harportraits_player2[10];

    object har_player1[10];
    object har_player2[10];

    progress_bar bar_power[2];
    progress_bar bar_agility[2];
    progress_bar bar_endurance[2];

    int pilot_id_a;
    int pilot_id_b;

    surface feh;
    surface bleh;
    surface select_hilight;
    unsigned int ticks;
    unsigned int hartick;
    unsigned int pulsedir;

    object *harplayer_a;
    object *harplayer_b;
} melee_local;

void refresh_pilot_stats(melee_local *local);

void handle_action(scene *scene, int player, int action);

void mask_sprite(surface *vga, int x, int y, int w, int h) {
    for(int i = 0; i < vga->h; i++) {
        for(int j = 0; j < vga->w; j++) {
            int offset = (i * vga->w) + j;
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
    game_player *player2 = game_state_get_player(scene->gs, 1);

    surface_free(&local->feh);
    surface_free(&local->bleh);
    surface_free(&local->select_hilight);
    for(int i = 0;i < 2;i++) {
        progressbar_free(&local->bar_power[i]);
        progressbar_free(&local->bar_agility[i]);
        progressbar_free(&local->bar_endurance[i]);
    }

    for(int i = 0; i < 10; i++) {
        object_free(&local->pilots[i].obj);
        object_free(&local->harportraits_player1[i]);
        object_free(&local->har_player1[i]);
        if (player2->selectable) {
            object_free(&local->harportraits_player2[i]);
            object_free(&local->har_player2[i]);
        }
    }

    object_free(&local->player2_placeholder);
    object_free(&local->unselected_har_portraits);
    object_free(&local->bigportrait1);
    if (player2->selectable) {
        object_free(&local->bigportrait2);
    }
    free(local);
}

void melee_tick(scene *scene) {
    melee_local *local = scene_get_userdata(scene);
    game_player *player1 = game_state_get_player(scene->gs, 0);
    game_player *player2 = game_state_get_player(scene->gs, 1);
    ctrl_event *i = NULL;

    // Handle extra controller inputs
    i = player1->ctrl->extra_events;
    if (i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                handle_action(scene, 1, i->event_data.action);
            } else if (i->type == EVENT_TYPE_CLOSE) {
                game_state_set_next(scene->gs, SCENE_MENU);
                return;
            }
        } while((i = i->next));
    }
    i = player2->ctrl->extra_events;
    if (i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                handle_action(scene, 2, i->event_data.action);
            } else if (i->type == EVENT_TYPE_CLOSE) {
                game_state_set_next(scene->gs, SCENE_MENU);
                return;
            }
        } while((i = i->next));
    }

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
            object_tick(&local->har_player1[5*local->row_a + local->column_a]);
            if (player2->selectable) {
                object_tick(&local->har_player2[5*local->row_b + local->column_b]);
            }
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
    game_player *player1 = game_state_get_player(scene->gs, 0);
    game_player *player2 = game_state_get_player(scene->gs, 1);
    melee_local *local = scene_get_userdata(scene);
    int *row, *column, *done;
    if (player == 1) {
        row = &local->row_a;
        column = &local->column_a;
        done = &local->done_a;
    } else {
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
                    local->pilot_id_a = 5*local->row_a + local->column_a;
                    local->pilot_id_b = 5*local->row_b + local->column_b;

                    object_select_sprite(&local->bigportrait1, local->pilot_id_a);
                    // update the player palette
                    palette *base_pal = video_get_base_palette();
                    palette_set_player_color(base_pal, 0, local->pilots[local->pilot_id_a].colors[0], 2);
                    palette_set_player_color(base_pal, 0, local->pilots[local->pilot_id_a].colors[1], 1);
                    palette_set_player_color(base_pal, 0, local->pilots[local->pilot_id_a].colors[2], 0);
                    video_force_pal_refresh();
                    player1->colors[0] = local->pilots[local->pilot_id_a].colors[0];
                    player1->colors[1] = local->pilots[local->pilot_id_a].colors[1];
                    player1->colors[2] = local->pilots[local->pilot_id_a].colors[2];

                    if (player2->selectable) {
                        object_select_sprite(&local->bigportrait2, local->pilot_id_b);
                        // update the player palette
                        palette_set_player_color(base_pal, 1, local->pilots[local->pilot_id_b].colors[0], 2);
                        palette_set_player_color(base_pal, 1, local->pilots[local->pilot_id_b].colors[1], 1);
                        palette_set_player_color(base_pal, 1, local->pilots[local->pilot_id_b].colors[2], 0);
                        video_force_pal_refresh();
                        player2->colors[0] = local->pilots[local->pilot_id_b].colors[0];
                        player2->colors[1] = local->pilots[local->pilot_id_b].colors[1];
                        player2->colors[2] = local->pilots[local->pilot_id_b].colors[2];
                    }

                } else {
                    player1->har_id = HAR_JAGUAR + 5*local->row_a+local->column_a;
                    player1->pilot_id = local->pilot_id_a;
                    if (player2->selectable) {
                        player2->har_id = HAR_JAGUAR + 5*local->row_b+local->column_b;
                        player2->pilot_id = local->pilot_id_b;
                    } else {
                        // randomly pick opponent and HAR
                        player2->har_id = HAR_JAGUAR + rand_int(10);
                        int i;
                        while((i = rand_int(10)) == local->pilot_id_a) {}
                        player2->pilot_id = i;

                        player2->colors[0] = local->pilots[player2->pilot_id].colors[0];
                        player2->colors[1] = local->pilots[player2->pilot_id].colors[1];
                        player2->colors[2] = local->pilots[player2->pilot_id].colors[2];
                    }
                    game_state_set_next(scene->gs, SCENE_VS);
                }
            }
            break;
    }

    if (local->selection == 0) {
        object_select_sprite(&local->bigportrait1, 5*local->row_a + local->column_a);
        if (player2->selectable) {
            object_select_sprite(&local->bigportrait2, 5*local->row_b + local->column_b);
        }
    }

    refresh_pilot_stats(local);
}

int melee_event(scene *scene, SDL_Event *event) {
    melee_local *local = scene_get_userdata(scene);
    if(event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_ESCAPE) {
        if (local->selection == 1) {
            // restore the player selection
            local->column_a = local->pilot_id_a % 5;
            local->row_a = local->pilot_id_a / 5;
            local->column_b = local->pilot_id_b % 5;
            local->row_b = local->pilot_id_b / 5;

            local->selection = 0;
            local->done_a = 0;
            local->done_b = 0;
        } else {
            game_state_set_next(scene->gs, SCENE_MENU);
        }
    } else {
        game_player *player1 = game_state_get_player(scene->gs, 0);
        game_player *player2 = game_state_get_player(scene->gs, 1);
        ctrl_event *p1=NULL, *p2 = NULL, *i;
        controller_event(player1->ctrl, event, &p1);
        controller_event(player2->ctrl, event, &p2);
        i = p1;
        if (i) {
            do {
                if(i->type == EVENT_TYPE_ACTION) {
                    handle_action(scene, 1, i->event_data.action);
                } else if (i->type == EVENT_TYPE_CLOSE) {
                    game_state_set_next(scene->gs, SCENE_MENU);
                    return 0;
                }
            } while((i = i->next));
        }
        controller_free_chain(p1);
        i = p2;
        if (i) {
            do {
                if(i->type == EVENT_TYPE_ACTION) {
                    handle_action(scene, 2, i->event_data.action);
                } else if (i->type == EVENT_TYPE_CLOSE) {
                    game_state_set_next(scene->gs, SCENE_MENU);
                    return 0;
                }
            } while((i = i->next));
        }
        controller_free_chain(p2);
    }
    return 0;
}

void render_highlights(scene *scene) {
    melee_local *local = scene_get_userdata(scene);
    game_player *player2 = game_state_get_player(scene->gs, 1);
    int trans;
    if (player2->selectable && local->row_a == local->row_b && local->column_a == local->column_b) {
        video_render_sprite_tint(&local->select_hilight, 
                                11 + (62*local->column_a), 
                                115 + (42*local->row_a), 
                                color_create(250-local->ticks, 0, 250-local->ticks, 0),
                                0);
    } else {
        if (player2->selectable) {
            if (local->done_b) {
                trans = 250;
            } else {
                trans = 250 - local->ticks;
            }
            video_render_sprite_tint(&local->select_hilight, 
                                    11 + (62*local->column_b), 
                                    115 + (42*local->row_b), 
                                    color_create(0, 0, trans, 0),
                                    0);
        }
        if (local->done_a) {
            trans = 250;
        } else {
            trans = 250 - local->ticks;
        }
        video_render_sprite_tint(&local->select_hilight, 
                                11 + (62*local->column_a), 
                                115 + (42*local->row_a), 
                                color_create(trans, 0, 0, 0),
                                0);
    }
}

void melee_render(scene *scene) {
    melee_local *local = scene_get_userdata(scene);
    game_player *player2 = game_state_get_player(scene->gs, 1);
    int current_a = 5*local->row_a + local->column_a;
    int current_b = 5*local->row_b + local->column_b;

    if (local->selection == 0) {
        video_render_sprite(&local->feh, 70, 0, BLEND_ALPHA, 0);
        video_render_sprite(&local->bleh, 0, 62, BLEND_ALPHA, 0);

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
            video_render_sprite(&local->feh, 320-70-local->feh.w, 0, BLEND_ALPHA, 0);
            video_render_sprite(&local->bleh, 320-local->bleh.w, 62, BLEND_ALPHA, 0);
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

    object_render(&local->player2_placeholder);

    if (local->selection == 0) {
        // player 1 name
        font_render_wrapped(&font_small, lang_get(20+current_a), 0, 52, 66, COLOR_BLACK);

        if (player2->selectable) {
            // player 2 name
            font_render_wrapped(&font_small, lang_get(20+current_b), 320-66, 52, 66, COLOR_BLACK);
        }

        render_highlights(scene);
        for(int i = 0; i < 10; i++) {
            object_render(&local->pilots[i].obj);
        }
        object_render(&local->bigportrait1);
        if (player2->selectable) {
            object_render(&local->bigportrait2);
        }
    } else {
        // render the stupid unselected HAR portraits before anything
        // so we can render anything else on top of them
        object_render(&local->unselected_har_portraits);
        render_highlights(scene);

        // currently selected player
        object_render(&local->bigportrait1);

        //currently selected HAR
        object_render(&local->harportraits_player1[5*local->row_a + local->column_a]);
        object_render(&local->har_player1[5*local->row_a + local->column_a]);

        // player 1 name
        font_render_wrapped(&font_small, lang_get(20+local->pilot_id_a), 0, 52, 66, COLOR_BLACK);

        if (player2->selectable) {
            // player 2 name
            font_render_wrapped(&font_small, lang_get(20+local->pilot_id_b), 320-66, 52, 66, COLOR_BLACK);

            // currently selected player
            object_render(&local->bigportrait2);

            // currently selected HAR
            object_render(&local->harportraits_player2[5*local->row_b + local->column_b]);
            object_render(&local->har_player2[5*local->row_b + local->column_b]);
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
    memset(local, 0, sizeof(melee_local));
    scene_set_userdata(scene, local);

    game_player *player1 = game_state_get_player(scene->gs, 0);
    game_player *player2 = game_state_get_player(scene->gs, 1);

    controller *player1_ctrl = game_player_get_ctrl(player1);
    controller *player2_ctrl = game_player_get_ctrl(player2);

    palette *mpal = video_get_base_palette();
    palette_set_player_color(mpal, 0, 8, 0);
    palette_set_player_color(mpal, 0, 8, 1);
    palette_set_player_color(mpal, 0, 8, 2);
    video_force_pal_refresh();

    // TODO read this from MASTER.DAT
    // XXX the colors are eyeballed
    local->pilots[0].power=5;
    local->pilots[0].agility=16;
    local->pilots[0].endurance=9;
    local->pilots[0].colors[0] = 5;
    local->pilots[0].colors[1] = 11;
    local->pilots[0].colors[2] = 15;

    local->pilots[1].power=13;
    local->pilots[1].agility=9;
    local->pilots[1].endurance=8;
    local->pilots[1].colors[0] = 10;
    local->pilots[1].colors[1] = 15;
    local->pilots[1].colors[2] = 7;

    local->pilots[2].power=7;
    local->pilots[2].agility=20;
    local->pilots[2].endurance=4;
    local->pilots[2].colors[0] = 11;
    local->pilots[2].colors[1] = 12;
    local->pilots[2].colors[2] = 8;


    local->pilots[3].power=9;
    local->pilots[3].agility=7;
    local->pilots[3].endurance=15;
    local->pilots[3].colors[0] = 8;
    local->pilots[3].colors[1] = 15;
    local->pilots[3].colors[2] = 12;

    local->pilots[4].power=20;
    local->pilots[4].agility=1;
    local->pilots[4].endurance=8;
    local->pilots[4].colors[0] = 4;
    local->pilots[4].colors[1] = 7;
    local->pilots[4].colors[2] = 14;

    local->pilots[5].power=9;
    local->pilots[5].agility=10;
    local->pilots[5].endurance=11;
    local->pilots[5].colors[0] = 1;
    local->pilots[5].colors[1] = 7;
    local->pilots[5].colors[2] = 6;

    local->pilots[6].power=10;
    local->pilots[6].agility=1;
    local->pilots[6].endurance=20;
    local->pilots[6].colors[0] = 8;
    local->pilots[6].colors[1] = 6;
    local->pilots[6].colors[2] = 14;

    local->pilots[7].power=7;
    local->pilots[7].agility=10;
    local->pilots[7].endurance=13;
    local->pilots[7].colors[0] = 0;
    local->pilots[7].colors[1] = 15;
    local->pilots[7].colors[2] = 7;

    local->pilots[8].power=14;
    local->pilots[8].agility=8;
    local->pilots[8].endurance=8;
    local->pilots[8].colors[0] = 0;
    local->pilots[8].colors[1] = 8;
    local->pilots[8].colors[2] = 2;

    local->pilots[9].power=14;
    local->pilots[9].agility=4;
    local->pilots[9].endurance=12;
    local->pilots[9].colors[0] = 9;
    local->pilots[9].colors[1] = 10;
    local->pilots[9].colors[2] = 4;

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
    surface_create_from_data(&local->select_hilight, SURFACE_TYPE_RGBA, 51, 36, bitmap);

    // set up the magic controller hooks
    if(player1_ctrl && player2_ctrl) {
        if(player1_ctrl->type == CTRL_TYPE_NETWORK) {
            controller_add_hook(player2_ctrl, player1_ctrl, player1_ctrl->controller_hook);
        }

        if (player2_ctrl->type == CTRL_TYPE_NETWORK) {
            controller_add_hook(player1_ctrl, player2_ctrl, player2_ctrl->controller_hook);
        }
    }

    animation *ani;
    sprite *spr;
    for(int i = 0; i < 10; i++) {
        ani = &bk_get_info(&scene->bk_data, 3)->ani;
        object_create(&local->pilots[i].obj, scene->gs, vec2i_create(0,0), vec2f_create(0, 0));
        object_set_animation(&local->pilots[i].obj, ani);
        object_select_sprite(&local->pilots[i].obj, i);

        ani = &bk_get_info(&scene->bk_data, 18+i)->ani;
        object_create(&local->har_player1[i], scene->gs, vec2i_create(110,95), vec2f_create(0, 0));
        object_set_animation(&local->har_player1[i], ani);
        object_select_sprite(&local->har_player1[i], 0);
        object_set_repeat(&local->har_player1[i], 1);

        int row = i / 5;
        int col = i % 5;
        spr = sprite_copy(animation_get_sprite(&bk_get_info(&scene->bk_data, 1)->ani, 0));
        mask_sprite(spr->data, 62*col, 42*row, 51, 36);
        ani = create_animation_from_single(spr, spr->pos);
        object_create(&local->harportraits_player1[i], scene->gs, vec2i_create(0, 0), vec2f_create(0, 0));
        object_set_animation(&local->harportraits_player1[i], ani);
        object_select_sprite(&local->harportraits_player1[i], 0);
        object_set_animation_owner(&local->harportraits_player1[i], OWNER_OBJECT);
        if (player2->selectable) {
            spr = sprite_copy(animation_get_sprite(&bk_get_info(&scene->bk_data, 1)->ani, 0));
            mask_sprite(spr->data, 62*col, 42*row, 51, 36);
            ani = create_animation_from_single(spr, spr->pos);
            object_create(&local->harportraits_player2[i], scene->gs, vec2i_create(0, 0), vec2f_create(0, 0));
            object_set_animation(&local->harportraits_player2[i], ani);
            object_select_sprite(&local->harportraits_player2[i], 0);
            object_set_animation_owner(&local->harportraits_player2[i], OWNER_OBJECT);

            ani = &bk_get_info(&scene->bk_data, 18+i)->ani;
            object_create(&local->har_player2[i], scene->gs, vec2i_create(210,95), vec2f_create(0, 0));
            object_set_animation(&local->har_player2[i], ani);
            object_select_sprite(&local->har_player2[i], 0);
            object_set_repeat(&local->har_player2[i], 1);
            object_set_direction(&local->har_player2[i], OBJECT_FACE_LEFT);
        }
    }

    ani = &bk_get_info(&scene->bk_data, 4)->ani;
    object_create(&local->bigportrait1, scene->gs, vec2i_create(0,0), vec2f_create(0, 0));
    object_set_animation(&local->bigportrait1, ani);
    object_select_sprite(&local->bigportrait1, 0);

    if (player2->selectable) {
        object_create(&local->bigportrait2, scene->gs, vec2i_create(320,0), vec2f_create(0, 0));
        object_set_animation(&local->bigportrait2, ani);
        object_select_sprite(&local->bigportrait2, 4);
        object_set_direction(&local->bigportrait2, OBJECT_FACE_LEFT);
    }

    ani = &bk_get_info(&scene->bk_data, 5)->ani;
    object_create(&local->player2_placeholder, scene->gs, vec2i_create(0,0), vec2f_create(0, 0));
    object_set_animation(&local->player2_placeholder, ani);
    if (player2->selectable) {
        object_select_sprite(&local->player2_placeholder, 0);
    } else {
        object_select_sprite(&local->player2_placeholder, 1);
    }

    ani = &bk_get_info(&scene->bk_data, 1)->ani;
    object_create(&local->unselected_har_portraits, scene->gs, vec2i_create(0,0), vec2f_create(0, 0));
    object_set_animation(&local->unselected_har_portraits, ani);
    object_select_sprite(&local->unselected_har_portraits, 0);

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

    // Set callbacks
    scene_set_event_cb(scene, melee_event);
    scene_set_render_cb(scene, melee_render);
    scene_set_free_cb(scene, melee_free);
    scene_set_tick_cb(scene, melee_tick);

    // All done
    return 0;
}
