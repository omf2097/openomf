#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>

#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/random.h"
#include "audio/music.h"
#include "audio/sound.h"
#include "video/video.h"
#include "resources/ids.h"
#include "resources/bk.h"
#include "resources/pilots.h"
#include "resources/animation.h"
#include "resources/sprite.h"
#include "game/gui/text_render.h"
#include "resources/languages.h"
#include "game/utils/settings.h"
#include "game/game_state.h"
#include "game/protos/scene.h"
#include "game/protos/object.h"
#include "game/scenes/melee.h"
#include "game/gui/progressbar.h"
#include "game/gui/menu_background.h"

#define MAX_STAT 20

typedef struct melee_local_t {

    int selection; // 0 for player, 1 for HAR
    int row_a, row_b; // 0 or 1
    int column_a, column_b; // 0-4
    int done_a, done_b; // 0-1

    object bigportrait1;
    object bigportrait2;
    object player2_placeholder;
    object unselected_har_portraits;

    object pilots[10];

    object harportraits_player1[10];
    object harportraits_player2[10];

    object har_player1[10];
    object har_player2[10];

    component *bar_power[2];
    component *bar_agility[2];
    component *bar_endurance[2];

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

    // nova selection cheat
    unsigned char har_selected[2][10];
    unsigned char katana_down_count[2];
} melee_local;

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
    for(int i = 0; i < 2; i++) {
        component_free(local->bar_power[i]);
        component_free(local->bar_agility[i]);
        component_free(local->bar_endurance[i]);
    }

    for(int i = 0; i < 10; i++) {
        object_free(&local->pilots[i]);
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
    omf_free(local);
    scene_set_userdata(scene, local);
}

void melee_tick(scene *scene, int paused) {
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
    if (local->selection == 1 && local->hartick > 10) {
        local->hartick = 0;
        object_dynamic_tick(&local->har_player1[5*local->row_a + local->column_a]);
        if (player2->selectable) {
            object_dynamic_tick(&local->har_player2[5*local->row_b + local->column_b]);
        }
    }

}

void refresh_pilot_stats(melee_local *local) {
    int current_a = 5*local->row_a + local->column_a;
    int current_b = 5*local->row_b + local->column_b;
    pilot p_a, p_b;

    pilot_get_info(&p_a, current_a);
    pilot_get_info(&p_b, current_b);
    progressbar_set_progress(local->bar_power[0], (p_a.power*100)/MAX_STAT);
    progressbar_set_progress(local->bar_agility[0], (p_a.agility*100)/MAX_STAT);
    progressbar_set_progress(local->bar_endurance[0], (p_a.endurance*100)/MAX_STAT);
    progressbar_set_progress(local->bar_power[1], (p_b.power*100)/MAX_STAT);
    progressbar_set_progress(local->bar_agility[1], (p_b.agility*100)/MAX_STAT);
    progressbar_set_progress(local->bar_endurance[1], (p_b.endurance*100)/MAX_STAT);
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
            sound_play(19, 0.5f, 0.0f, 2.0f);
            break;
        case ACT_RIGHT:
            (*column)++;
            if (*column > 4) {
                *column = 0;
            }
            sound_play(19, 0.5f, 0.0f, 2.0f);
            break;
        case ACT_UP:
            if(*row == 1) {
                *row = 0;
            }
            sound_play(19, 0.5f, 0.0f, 2.0f);
            break;
        case ACT_DOWN:
            if(*row == 0) {
                *row = 1;
            }
            // nova selection cheat
            if(*row == 1 && *column == 0) {
                local->katana_down_count[player-1]++;
                if(local->katana_down_count[player-1] > 11) {
                    local->katana_down_count[player-1] = 11;
                }
            }
            sound_play(19, 0.5f, 0.0f, 2.0f);
            break;
        case ACT_KICK:
        case ACT_PUNCH:
            *done = 1;
            sound_play(20, 0.5f, 0.0f, 2.0f);
            if (local->done_a && (local->done_b || !player2->selectable)) {
                local->done_a = 0;
                local->done_b = 0;
                if (local->selection == 0) {
                    local->selection = 1;
                    local->pilot_id_a = 5*local->row_a + local->column_a;
                    local->pilot_id_b = 5*local->row_b + local->column_b;

                    // nova selection cheat
                    local->har_selected[0][local->pilot_id_a] = 1;
                    local->har_selected[1][local->pilot_id_b] = 1;

                    object_select_sprite(&local->bigportrait1, local->pilot_id_a);
                    // update the player palette
                    palette *base_pal = video_get_base_palette();
                    pilot p_a;
                    pilot_get_info(&p_a, local->pilot_id_a);
                    palette_set_player_color(base_pal, 0, p_a.colors[0], 2);
                    palette_set_player_color(base_pal, 0, p_a.colors[1], 1);
                    palette_set_player_color(base_pal, 0, p_a.colors[2], 0);
                    video_force_pal_refresh();
                    player1->colors[0] = p_a.colors[0];
                    player1->colors[1] = p_a.colors[1];
                    player1->colors[2] = p_a.colors[2];

                    if (player2->selectable) {
                        object_select_sprite(&local->bigportrait2, local->pilot_id_b);
                        // update the player palette
                        pilot_get_info(&p_a, local->pilot_id_b);
                        palette_set_player_color(base_pal, 1, p_a.colors[0], 2);
                        palette_set_player_color(base_pal, 1, p_a.colors[1], 1);
                        palette_set_player_color(base_pal, 1, p_a.colors[2], 0);
                        video_force_pal_refresh();
                        player2->colors[0] = p_a.colors[0];
                        player2->colors[1] = p_a.colors[1];
                        player2->colors[2] = p_a.colors[2];
                    }
                } else {
                    int nova_activated[2] = {1, 1};
                    for(int i = 0;i < 2;i++) {
                        for(int j = 0;j < 10;j++) {
                            if(local->har_selected[i][j] == 0) {
                                nova_activated[i] = 0;
                                break;
                            }
                        }
                        if(local->katana_down_count[i] < 11) {
                            nova_activated[i] = 0;
                        }
                    }
                    if(nova_activated[0] && local->row_a == 1 && local->column_a == 2) {
                        player1->har_id = HAR_NOVA;
                    } else {
                        player1->har_id = 5*local->row_a+local->column_a;
                    }
                    player1->pilot_id = local->pilot_id_a;
                    if (player2->selectable) {
                        if(nova_activated[1] && local->row_b == 1 && local->column_b == 2) {
                            player2->har_id = HAR_NOVA;
                        } else {
                            player2->har_id = 5*local->row_b+local->column_b;
                        }
                        player2->pilot_id = local->pilot_id_b;
                    } else {
                        if (player1->sp_wins == (2046 ^ (2 << player1->pilot_id))) {
                            // everyone but kriessack
                            player2->pilot_id = 10;
                            player2->har_id = HAR_NOVA;
                        } else {
                            // pick an opponent we have not yet beaten
                            while(1) {
                                int i = rand_int(10);
                                if ((2 << i) & player1->sp_wins || i == player1->pilot_id) {
                                    continue;
                                }
                                player2->pilot_id = i;
                                player2->har_id = rand_int(10);
                                break;
                            }
                        }

                        pilot p_a;
                        pilot_get_info(&p_a, player2->pilot_id);
                        player2->colors[0] = p_a.colors[0];
                        player2->colors[1] = p_a.colors[1];
                        player2->colors[2] = p_a.colors[2];
                    }
                    game_state_set_next(scene->gs, SCENE_VS);
                }
            }
            break;
    }

    if(local->selection == 0) {
        object_select_sprite(&local->bigportrait1, 5*local->row_a + local->column_a);
        if (player2->selectable) {
            object_select_sprite(&local->bigportrait2, 5*local->row_b + local->column_b);
        }
    }

    // nova selection cheat
    if(local->selection == 1) {
        local->har_selected[player-1][5 * (*row) + *column] = 1;
    }

    refresh_pilot_stats(local);
}

void melee_input_tick(scene *scene) {
    melee_local *local = scene_get_userdata(scene);
    game_player *player1 = game_state_get_player(scene->gs, 0);
    game_player *player2 = game_state_get_player(scene->gs, 1);
    ctrl_event *p1=NULL, *p2 = NULL, *i;
    controller_poll(player1->ctrl, &p1);
    controller_poll(player2->ctrl, &p2);
    i = p1;
    if (i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                if (i->event_data.action == ACT_ESC) {
                    sound_play(20, 0.5f, 0.0f, 2.0f);
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
                    handle_action(scene, 1, i->event_data.action);
                }
            } else if (i->type == EVENT_TYPE_CLOSE) {
                game_state_set_next(scene->gs, SCENE_MENU);
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
            }
        } while((i = i->next));
    }
    controller_free_chain(p2);
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
        font_render_wrapped_shadowed(&font_small, lang_get(135+current_a), 4, 66, 152, COLOR_GREEN, TEXT_SHADOW_RIGHT|TEXT_SHADOW_BOTTOM);
        // player stats
        font_render_shadowed(&font_small, lang_get(216), 74+27, 4, COLOR_GREEN, TEXT_SHADOW_RIGHT|TEXT_SHADOW_BOTTOM);
        font_render_shadowed(&font_small, lang_get(217), 74+19, 22, COLOR_GREEN, TEXT_SHADOW_RIGHT|TEXT_SHADOW_BOTTOM);
        font_render_shadowed(&font_small, lang_get(218), 74+12, 40, COLOR_GREEN, TEXT_SHADOW_RIGHT|TEXT_SHADOW_BOTTOM);
        component_render(local->bar_power[0]);
        component_render(local->bar_agility[0]);
        component_render(local->bar_endurance[0]);

        if (player2->selectable) {
            video_render_sprite(&local->feh, 320-70-local->feh.w, 0, BLEND_ALPHA, 0);
            video_render_sprite(&local->bleh, 320-local->bleh.w, 62, BLEND_ALPHA, 0);
            // player bio
            font_render_wrapped_shadowed(&font_small, lang_get(135+current_b), 320-local->bleh.w+4, 66, 152, COLOR_GREEN, TEXT_SHADOW_RIGHT|TEXT_SHADOW_BOTTOM);
            // player stats
            font_render_shadowed(&font_small, lang_get(216), 320-66-local->feh.w+27, 4, COLOR_GREEN, TEXT_SHADOW_RIGHT|TEXT_SHADOW_BOTTOM);
            font_render_shadowed(&font_small, lang_get(217), 320-66-local->feh.w+19, 22, COLOR_GREEN, TEXT_SHADOW_RIGHT|TEXT_SHADOW_BOTTOM);
            font_render_shadowed(&font_small, lang_get(218), 320-66-local->feh.w+12, 40, COLOR_GREEN, TEXT_SHADOW_RIGHT|TEXT_SHADOW_BOTTOM);
            component_render(local->bar_power[1]);
            component_render(local->bar_agility[1]);
            component_render(local->bar_endurance[1]);
        } else {
            // 'choose your pilot'
            font_render_wrapped_shadowed(&font_small, lang_get(187), 160, 97, 160, COLOR_GREEN, TEXT_SHADOW_RIGHT|TEXT_SHADOW_BOTTOM);
        }
    }

    object_render(&local->player2_placeholder);

    if (local->selection == 0) {
        // player 1 name
        font_render_wrapped_shadowed(&font_small, lang_get(20+current_a), 0, 52, 66, COLOR_BLACK, TEXT_SHADOW_TOP|TEXT_SHADOW_LEFT);

        if (player2->selectable) {
            // player 2 name
            font_render_wrapped_shadowed(&font_small, lang_get(20+current_b), 320-66, 52, 66, COLOR_BLACK, TEXT_SHADOW_TOP|TEXT_SHADOW_LEFT);
        }

        render_highlights(scene);
        for(int i = 0; i < 10; i++) {
            object_render(&local->pilots[i]);
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
        font_render_wrapped_shadowed(&font_small, lang_get(20+local->pilot_id_a), 0, 52, 66, COLOR_BLACK, TEXT_SHADOW_TOP|TEXT_SHADOW_LEFT);

        if (player2->selectable) {
            // player 2 name
            font_render_wrapped_shadowed(&font_small, lang_get(20+local->pilot_id_b), 320-66, 52, 66, COLOR_BLACK, TEXT_SHADOW_TOP|TEXT_SHADOW_LEFT);

            // currently selected player
            object_render(&local->bigportrait2);

            // currently selected HAR
            object_render(&local->harportraits_player2[5*local->row_b + local->column_b]);
            object_render(&local->har_player2[5*local->row_b + local->column_b]);

            // render HAR name (Har1 VS. Har2)
            str vstext;
            str_create(&vstext);
            str_printf(&vstext, "%s VS. %s",
                har_get_name(5*local->row_a + local->column_a),
                har_get_name(5*local->row_b + local->column_b));
            font_render_wrapped_shadowed(&font_small, str_c(&vstext), 80, 107, 150, COLOR_BLACK, TEXT_SHADOW_TOP|TEXT_SHADOW_LEFT);
            str_free(&vstext);
        } else {
            // 'choose your HAR'
            font_render_wrapped_shadowed(&font_small, lang_get(186), 160, 97, 160, COLOR_GREEN, TEXT_SHADOW_RIGHT|TEXT_SHADOW_BOTTOM);

            // render HAR name
            font_render_wrapped_shadowed(&font_small, har_get_name(5*local->row_a + local->column_a), 130, 107, 66, COLOR_BLACK, TEXT_SHADOW_TOP|TEXT_SHADOW_LEFT);
        }
    }

    if (player2->selectable) {
        chr_score *s1 = game_player_get_score(game_state_get_player(scene->gs, 0));
        chr_score *s2 = game_player_get_score(game_state_get_player(scene->gs, 1));
        char winstext[48];
        snprintf(winstext, 48, "Wins: %d", s1->wins);
        font_render_shadowed(&font_small, winstext, 8, 107, COLOR_BLACK, TEXT_SHADOW_TOP|TEXT_SHADOW_LEFT);
        snprintf(winstext, 48, "Wins: %d", s2->wins);
        font_render_shadowed(&font_small, winstext, 312-(strlen(winstext)*font_small.w), 107, COLOR_BLACK, TEXT_SHADOW_TOP|TEXT_SHADOW_LEFT);
    }
}

int melee_create(scene *scene) {
    char bitmap[51*36*4];

    // Init local data
    melee_local *local = omf_calloc(1, sizeof(melee_local));
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
        object_create(&local->pilots[i], scene->gs, vec2i_create(0,0), vec2f_create(0, 0));
        object_set_animation(&local->pilots[i], ani);
        object_select_sprite(&local->pilots[i], i);

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
            object_set_pal_offset(&local->harportraits_player2[i], 48);

            ani = &bk_get_info(&scene->bk_data, 18+i)->ani;
            object_create(&local->har_player2[i], scene->gs, vec2i_create(210,95), vec2f_create(0, 0));
            object_set_animation(&local->har_player2[i], ani);
            object_select_sprite(&local->har_player2[i], 0);
            object_set_repeat(&local->har_player2[i], 1);
            object_set_direction(&local->har_player2[i], OBJECT_FACE_LEFT);
            object_set_pal_offset(&local->har_player2[i], 48);
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

    spr = sprite_copy(animation_get_sprite(&bk_get_info(&scene->bk_data, 1)->ani, 0));
    surface_convert_to_rgba(spr->data, video_get_pal_ref(), 0);
    ani = create_animation_from_single(spr, spr->pos);
    object_create(&local->unselected_har_portraits, scene->gs, vec2i_create(0,0), vec2f_create(0, 0));
    object_set_animation(&local->unselected_har_portraits, ani);
    object_select_sprite(&local->unselected_har_portraits, 0);
    object_set_animation_owner(&local->unselected_har_portraits, OWNER_OBJECT);

    for(int i = 0; i < 2; i++) {
        local->bar_power[i] = progressbar_create(PROGRESSBAR_THEME_MELEE, PROGRESSBAR_LEFT, 50);
        local->bar_agility[i] = progressbar_create(PROGRESSBAR_THEME_MELEE, PROGRESSBAR_LEFT, 50);
        local->bar_endurance[i] = progressbar_create(PROGRESSBAR_THEME_MELEE, PROGRESSBAR_LEFT, 50);
    }
    component_layout(local->bar_power[0], 74, 12, 20*4, 8);
    component_layout(local->bar_agility[0], 74, 30, 20*4, 8);
    component_layout(local->bar_endurance[0], 74, 48, 20*4, 8);
    component_layout(local->bar_power[1], 320-66-local->feh.w, 12, 20*4, 8);
    component_layout(local->bar_agility[1], 320-66-local->feh.w, 30, 20*4, 8);
    component_layout(local->bar_endurance[1], 320-66-local->feh.w, 48, 20*4, 8);

    refresh_pilot_stats(local);

    // initialize nova selection cheat
    memset(local->har_selected, 0, sizeof(local->har_selected));
    memset(local->katana_down_count, 0, sizeof(local->katana_down_count));

    // Set callbacks
    scene_set_input_poll_cb(scene, melee_input_tick);
    scene_set_render_cb(scene, melee_render);
    scene_set_free_cb(scene, melee_free);
    scene_set_dynamic_tick_cb(scene, melee_tick);

    // Play correct music
    music_play(PSM_MENU);

    // Don't render background on its own layer
    // Fix for some additive blending tricks.
    video_render_bg_separately(false);

    // All done
    return 0;
}
