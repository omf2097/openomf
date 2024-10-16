#include <stdlib.h>

#include "audio/audio.h"
#include "formats/pilot.h"
#include "game/game_state.h"
#include "game/gui/menu_background.h"
#include "game/gui/progressbar.h"
#include "game/gui/text_render.h"
#include "game/protos/object.h"
#include "game/protos/scene.h"
#include "game/scenes/melee.h"
#include "resources/animation.h"
#include "resources/bk.h"
#include "resources/languages.h"
#include "resources/pilots.h"
#include "resources/sprite.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/random.h"
#include "video/video.h"

#define MAX_STAT 20
#define TEXT_GREEN 0xA6
#define TEXT_SHADOW_GREEN 0xA2
#define TEXT_BLACK 0xD1
#define TEXT_SHADOW_BLACK 0xD7
#define RED_CURSOR_INDEX 0xF6
#define BLUE_CURSOR_INDEX 0xF7
#define VIOLET_CURSOR_INDEX 0xF8

typedef struct {
    int x;
    int y;
    int disabled_offset;
    surface enabled;
    surface disabled;
} portrait;

typedef struct {
    int row;
    int column;
    bool done;
} cursor_data;

#define CURSOR_INDEX(local, player) (5 * local->cursor[player].row + local->cursor[player].column)
#define CURSOR_A_DONE(local) (local->cursor[0].done)
#define CURSOR_B_DONE(local) (local->cursor[1].done)
#define CURSORS_MATCH(local)                                                                                           \
    (local->cursor[0].column == local->cursor[1].column && local->cursor[0].row == local->cursor[1].row)
#define CURSORS_DONE(local) (local->cursor[0].done && local->cursor[1].done)
#define CURSOR_NOVA_SELECT(local, player) (local->cursor[player].row == 1 && local->cursor[player].column == 2)

typedef enum
{
    PILOT_SELECT = 0,
    HAR_SELECT = 1
} selection_page;

typedef struct {
    cursor_data cursor[2];
    selection_page page;

    object big_portrait_1;
    object big_portrait_2;
    object player2_placeholder;
    object unselected_har_portraits;

    portrait pilot_portraits[10];
    portrait har_portraits[10];

    object har_player1[10];
    object har_player2[10];

    component *bar_power[2];
    component *bar_agility[2];
    component *bar_endurance[2];

    int pilot_id_a;
    int pilot_id_b;

    surface bg_player_stats;
    surface bg_player_bio;
    surface select_hilight;

    unsigned int ticks;

    // nova selection cheat
    unsigned char har_selected[2][10];
    unsigned char katana_down_count[2];
} melee_local;

void handle_action(scene *scene, int player, int action);

void melee_free(scene *scene) {
    melee_local *local = scene_get_userdata(scene);
    game_player *player2 = game_state_get_player(scene->gs, 1);

    surface_free(&local->bg_player_stats);
    surface_free(&local->bg_player_bio);
    surface_free(&local->select_hilight);
    for(int i = 0; i < 2; i++) {
        component_free(local->bar_power[i]);
        component_free(local->bar_agility[i]);
        component_free(local->bar_endurance[i]);
    }

    for(int i = 0; i < 10; i++) {
        surface_free(&local->har_portraits[i].enabled);
        surface_free(&local->har_portraits[i].disabled);
        surface_free(&local->pilot_portraits[i].enabled);
        surface_free(&local->pilot_portraits[i].disabled);
        object_free(&local->har_player1[i]);
        if(player2->selectable) {
            object_free(&local->har_player2[i]);
        }
    }

    object_free(&local->player2_placeholder);
    object_free(&local->unselected_har_portraits);
    object_free(&local->big_portrait_1);
    if(player2->selectable) {
        object_free(&local->big_portrait_2);
    }
    omf_free(local);
    scene_set_userdata(scene, local);
}

static void set_cursor_colors(int offset) {
    palette *pal = video_get_base_palette();
    int base = 120;
    pal->data[RED_CURSOR_INDEX][0] = base + offset;
    pal->data[RED_CURSOR_INDEX][1] = 0;
    pal->data[RED_CURSOR_INDEX][2] = 0;
    pal->data[BLUE_CURSOR_INDEX][0] = 0;
    pal->data[BLUE_CURSOR_INDEX][1] = 0;
    pal->data[BLUE_CURSOR_INDEX][2] = base + offset;
    pal->data[VIOLET_CURSOR_INDEX][0] = base + offset;
    pal->data[VIOLET_CURSOR_INDEX][1] = 0;
    pal->data[VIOLET_CURSOR_INDEX][2] = base + offset;
    video_force_pal_refresh();
}

void melee_tick(scene *scene, int paused) {
    melee_local *local = scene_get_userdata(scene);
    game_player *player1 = game_state_get_player(scene->gs, 0);
    game_player *player2 = game_state_get_player(scene->gs, 1);
    ctrl_event *i = NULL;

    // Handle extra controller inputs
    i = player1->ctrl->extra_events;
    if(i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                handle_action(scene, 1, i->event_data.action);
            } else if(i->type == EVENT_TYPE_CLOSE) {
                game_state_set_next(scene->gs, SCENE_MENU);
                return;
            }
        } while((i = i->next) != NULL);
    }
    i = player2->ctrl->extra_events;
    if(i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                handle_action(scene, 2, i->event_data.action);
            } else if(i->type == EVENT_TYPE_CLOSE) {
                game_state_set_next(scene->gs, SCENE_MENU);
                return;
            }
        } while((i = i->next) != NULL);
    }

    if(local->page == HAR_SELECT && local->ticks % 10 == 1) {
        object_dynamic_tick(&local->har_player1[CURSOR_INDEX(local, 0)]);
        if(player2->selectable) {
            object_dynamic_tick(&local->har_player2[CURSOR_INDEX(local, 1)]);
        }
    }

    // Tick cursor colors
    local->ticks++;
    double rate = ((double)local->ticks) / 25.0;
    int num = round((sin(rate) + 1.0) * 64);
    set_cursor_colors(num);
}

void refresh_pilot_stats(melee_local *local) {
    pilot p_a, p_b;
    pilot_get_info(&p_a, CURSOR_INDEX(local, 0));
    pilot_get_info(&p_b, CURSOR_INDEX(local, 1));
    progressbar_set_progress(local->bar_power[0], (p_a.power * 100) / MAX_STAT);
    progressbar_set_progress(local->bar_agility[0], (p_a.agility * 100) / MAX_STAT);
    progressbar_set_progress(local->bar_endurance[0], (p_a.endurance * 100) / MAX_STAT);
    progressbar_set_progress(local->bar_power[1], (p_b.power * 100) / MAX_STAT);
    progressbar_set_progress(local->bar_agility[1], (p_b.agility * 100) / MAX_STAT);
    progressbar_set_progress(local->bar_endurance[1], (p_b.endurance * 100) / MAX_STAT);
}

void handle_action(scene *scene, int player, int action) {
    game_player *player1 = game_state_get_player(scene->gs, 0);
    game_player *player2 = game_state_get_player(scene->gs, 1);
    melee_local *local = scene_get_userdata(scene);
    int *row = &local->cursor[player - 1].row;
    int *column = &local->cursor[player - 1].column;
    bool *done = &local->cursor[player - 1].done;

    if(*done) {
        return;
    }

    switch(action) {
        case ACT_LEFT:
            (*column)--;
            if(*column < 0) {
                *column = 4;
            }
            audio_play_sound(19, 0.5f, 0.0f, 2.0f);
            break;
        case ACT_RIGHT:
            (*column)++;
            if(*column > 4) {
                *column = 0;
            }
            audio_play_sound(19, 0.5f, 0.0f, 2.0f);
            break;
        case ACT_UP:
            if(*row == 1) {
                *row = 0;
            }
            audio_play_sound(19, 0.5f, 0.0f, 2.0f);
            break;
        case ACT_DOWN:
            if(*row == 0) {
                *row = 1;
            }
            // nova selection cheat
            if(*row == 1 && *column == 0) {
                local->katana_down_count[player - 1]++;
                if(local->katana_down_count[player - 1] > 11) {
                    local->katana_down_count[player - 1] = 11;
                }
            }
            audio_play_sound(19, 0.5f, 0.0f, 2.0f);
            break;
        case ACT_KICK:
        case ACT_PUNCH:
            *done = 1;
            audio_play_sound(20, 0.5f, 0.0f, 2.0f);
            if(CURSOR_A_DONE(local) && (CURSOR_B_DONE(local) || !player2->selectable)) {
                local->cursor[0].done = 0;
                local->cursor[1].done = 0;
                if(local->page == PILOT_SELECT) {
                    local->page = HAR_SELECT;
                    local->pilot_id_a = CURSOR_INDEX(local, 0);
                    local->pilot_id_b = CURSOR_INDEX(local, 1);

                    // nova selection cheat
                    local->har_selected[0][local->pilot_id_a] = 1;
                    local->har_selected[1][local->pilot_id_b] = 1;

                    object_select_sprite(&local->big_portrait_1, local->pilot_id_a);
                    // update the player palette
                    palette *base_pal = video_get_base_palette();
                    pilot p_a;
                    pilot_get_info(&p_a, local->pilot_id_a);
                    player1->pilot->endurance = p_a.endurance;
                    player1->pilot->power = p_a.power;
                    player1->pilot->agility = p_a.agility;
                    sd_pilot_set_player_color(player1->pilot, TERTIARY, p_a.colors[0]);
                    sd_pilot_set_player_color(player1->pilot, SECONDARY, p_a.colors[1]);
                    sd_pilot_set_player_color(player1->pilot, PRIMARY, p_a.colors[2]);

                    palette_load_player_colors(base_pal, &player1->pilot->palette, 0);

                    if(player2->selectable) {
                        object_select_sprite(&local->big_portrait_2, local->pilot_id_b);
                        // update the player palette
                        pilot_get_info(&p_a, local->pilot_id_b);
                        player2->pilot->endurance = p_a.endurance;
                        player2->pilot->power = p_a.power;
                        player2->pilot->agility = p_a.agility;
                        sd_pilot_set_player_color(player2->pilot, TERTIARY, p_a.colors[0]);
                        sd_pilot_set_player_color(player2->pilot, SECONDARY, p_a.colors[1]);
                        sd_pilot_set_player_color(player2->pilot, PRIMARY, p_a.colors[2]);

                        palette_load_player_colors(base_pal, &player2->pilot->palette, 1);
                    }
                    video_force_pal_refresh();
                } else {
                    int nova_activated[2] = {1, 1};
                    for(int i = 0; i < 2; i++) {
                        for(int j = 0; j < 10; j++) {
                            if(local->har_selected[i][j] == 0) {
                                nova_activated[i] = 0;
                                break;
                            }
                        }
                        if(local->katana_down_count[i] < 11) {
                            nova_activated[i] = 0;
                        }
                    }
                    if(nova_activated[0] && CURSOR_NOVA_SELECT(local, 0)) {
                        player1->pilot->har_id = HAR_NOVA;
                    } else {
                        player1->pilot->har_id = CURSOR_INDEX(local, 0);
                    }
                    player1->pilot->pilot_id = local->pilot_id_a;
                    if(player2->selectable) {
                        if(nova_activated[1] && CURSOR_NOVA_SELECT(local, 1)) {
                            player2->pilot->har_id = HAR_NOVA;
                        } else {
                            player2->pilot->har_id = CURSOR_INDEX(local, 1);
                        }
                        player2->pilot->pilot_id = local->pilot_id_b;
                    } else {
                        if(player1->sp_wins == (2046 ^ (2 << player1->pilot->pilot_id))) {
                            // everyone but kreissack
                            player2->pilot->pilot_id = PILOT_KREISSACK;
                            player2->pilot->har_id = HAR_NOVA;
                        } else {
                            // pick an opponent we have not yet beaten
                            while(1) {
                                int i = rand_int(10);
                                if((2 << i) & player1->sp_wins || i == player1->pilot->pilot_id) {
                                    continue;
                                }
                                player2->pilot->pilot_id = i;
                                player2->pilot->har_id = rand_int(10);
                                break;
                            }
                        }

                        pilot p_a;
                        pilot_get_info(&p_a, player2->pilot->pilot_id);
                        player2->pilot->endurance = p_a.endurance;
                        player2->pilot->power = p_a.power;
                        player2->pilot->agility = p_a.agility;
                        sd_pilot_set_player_color(player2->pilot, TERTIARY, p_a.colors[0]);
                        sd_pilot_set_player_color(player2->pilot, SECONDARY, p_a.colors[1]);
                        sd_pilot_set_player_color(player2->pilot, PRIMARY, p_a.colors[2]);
                    }
                    game_state_set_next(scene->gs, SCENE_VS);
                }
            }
            break;
    }

    if(local->page == PILOT_SELECT) {
        object_select_sprite(&local->big_portrait_1, CURSOR_INDEX(local, 0));
        if(player2->selectable) {
            object_select_sprite(&local->big_portrait_2, CURSOR_INDEX(local, 1));
        }
    }

    // nova selection cheat
    if(local->page == HAR_SELECT) {
        local->har_selected[player - 1][5 * (*row) + *column] = 1;
    }

    refresh_pilot_stats(local);
}

void melee_input_tick(scene *scene) {
    melee_local *local = scene_get_userdata(scene);
    game_player *player1 = game_state_get_player(scene->gs, 0);
    game_player *player2 = game_state_get_player(scene->gs, 1);
    ctrl_event *p1 = NULL, *p2 = NULL, *i;
    controller_poll(player1->ctrl, &p1);
    controller_poll(player2->ctrl, &p2);
    i = p1;
    if(i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                if(i->event_data.action == ACT_ESC) {
                    audio_play_sound(20, 0.5f, 0.0f, 2.0f);
                    if(local->page == HAR_SELECT) {
                        // restore the player selection
                        local->cursor[0].column = local->pilot_id_a % 5;
                        local->cursor[0].row = local->pilot_id_a / 5;
                        local->cursor[0].done = 0;
                        local->cursor[1].column = local->pilot_id_b % 5;
                        local->cursor[1].row = local->pilot_id_b / 5;
                        local->cursor[1].done = 0;
                        local->page = PILOT_SELECT;
                    } else {
                        game_state_set_next(scene->gs, SCENE_MENU);
                    }
                } else {
                    handle_action(scene, 1, i->event_data.action);
                }
            } else if(i->type == EVENT_TYPE_CLOSE) {
                game_state_set_next(scene->gs, SCENE_MENU);
            }
        } while((i = i->next) != NULL);
    }
    controller_free_chain(p1);
    i = p2;
    if(i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                handle_action(scene, 2, i->event_data.action);
            } else if(i->type == EVENT_TYPE_CLOSE) {
                game_state_set_next(scene->gs, SCENE_MENU);
            }
        } while((i = i->next) != NULL);
    }
    controller_free_chain(p2);
}

static void draw_highlight(const melee_local *local, const cursor_data *cursor, int offset) {
    int x = 11 + (62 * cursor->column);
    int y = 115 + (42 * cursor->row);
    video_draw_offset(&local->select_hilight, x, y, offset, 255);
}

static void render_highlights(const melee_local *local, bool player2_is_selectable) {
    if(player2_is_selectable && CURSORS_MATCH(local)) {
        draw_highlight(local, &local->cursor[0], VIOLET_CURSOR_INDEX);
    } else {
        if(player2_is_selectable) {
            draw_highlight(local, &local->cursor[1], BLUE_CURSOR_INDEX);
        }
        draw_highlight(local, &local->cursor[0], RED_CURSOR_INDEX);
    }
}

static void render_disabled_portraits(const portrait *portraits) {
    for(int i = 0; i < 10; i++) {
        const portrait *p = &portraits[i];
        video_draw_offset(&p->disabled, p->x, p->y, p->disabled_offset, 255);
    }
}

static void render_enabled_portrait(const portrait *portraits, cursor_data *cursor) {
    const portrait *p = &portraits[5 * cursor->row + cursor->column];
    video_draw(&p->enabled, p->x, p->y);
}

static void render_pilot_select(melee_local *local, bool player2_is_selectable) {
    int current_a = CURSOR_INDEX(local, 0);
    int current_b = CURSOR_INDEX(local, 1);

    video_draw(&local->bg_player_stats, 70, 0);
    video_draw(&local->bg_player_bio, 0, 62);

    text_settings tconf_green;
    text_defaults(&tconf_green);
    tconf_green.font = FONT_SMALL;
    tconf_green.cforeground = TEXT_GREEN;
    tconf_green.shadow = TEXT_SHADOW_RIGHT | TEXT_SHADOW_BOTTOM;
    tconf_green.halign = TEXT_CENTER;
    tconf_green.cshadow = TEXT_SHADOW_GREEN;

    text_settings tconf_black;
    text_defaults(&tconf_black);
    tconf_black.font = FONT_SMALL;
    tconf_black.cforeground = TEXT_BLACK;
    tconf_black.shadow = TEXT_SHADOW_TOP | TEXT_SHADOW_LEFT;
    tconf_black.halign = TEXT_CENTER;
    tconf_black.cshadow = TEXT_SHADOW_BLACK;

    // player bio
    text_render(&tconf_green, TEXT_DEFAULT, 4, 66, 152, 40, lang_get(135 + current_a));
    // player stats
    text_render(&tconf_green, TEXT_DEFAULT, 74, 4, 85, 8, lang_get(216));
    text_render(&tconf_green, TEXT_DEFAULT, 74, 22, 85, 8, lang_get(217));
    text_render(&tconf_green, TEXT_DEFAULT, 74, 40, 85, 8, lang_get(218));
    component_render(local->bar_power[0]);
    component_render(local->bar_agility[0]);
    component_render(local->bar_endurance[0]);

    if(player2_is_selectable) {
        // player 2 name
        text_render(&tconf_black, TEXT_DEFAULT, 320 - 66, 52, 66, 8, lang_get(20 + current_b));

        video_draw(&local->bg_player_stats, 320 - 70 - local->bg_player_stats.w, 0);
        video_draw(&local->bg_player_bio, 320 - local->bg_player_bio.w, 62);
        // player bio
        text_render(&tconf_green, TEXT_DEFAULT, 320 - local->bg_player_bio.w + 4, 66, 152, 40,
                    lang_get(135 + current_b));

        // player stats
        text_render(&tconf_green, TEXT_DEFAULT, 320 - 66 - local->bg_player_stats.w, 4, 85, 8, lang_get(216));
        text_render(&tconf_green, TEXT_DEFAULT, 320 - 66 - local->bg_player_stats.w, 22, 85, 8, lang_get(217));
        text_render(&tconf_green, TEXT_DEFAULT, 320 - 66 - local->bg_player_stats.w, 40, 85, 8, lang_get(218));

        component_render(local->bar_power[1]);
        component_render(local->bar_agility[1]);
        component_render(local->bar_endurance[1]);
    } else {
        // 'choose your pilot'
        text_render(&tconf_green, TEXT_DEFAULT, 160, 97, 160, 8, lang_get(187));
    }

    object_render(&local->player2_placeholder);

    // player 1 name
    text_render(&tconf_black, TEXT_DEFAULT, 0, 52, 66, 8, lang_get(20 + current_a));

    render_highlights(local, player2_is_selectable);
    render_disabled_portraits(local->pilot_portraits);
    render_enabled_portrait(local->pilot_portraits, &local->cursor[0]);
    object_render(&local->big_portrait_1);
    if(player2_is_selectable) {
        render_enabled_portrait(local->pilot_portraits, &local->cursor[0]);
        object_render(&local->big_portrait_2);
    }
}

static void render_har_select(melee_local *local, bool player2_is_selectable) {
    object_render(&local->player2_placeholder);

    // render the stupid unselected HAR portraits before anything
    // so we can render anything else on top of them
    render_highlights(local, player2_is_selectable);
    render_disabled_portraits(local->har_portraits);

    // currently selected player
    object_render(&local->big_portrait_1);

    // currently selected HAR
    render_enabled_portrait(local->har_portraits, &local->cursor[0]);
    object_render(&local->har_player1[CURSOR_INDEX(local, 0)]);

    text_settings tconf_green;
    text_defaults(&tconf_green);
    tconf_green.font = FONT_SMALL;
    tconf_green.cforeground = TEXT_GREEN;
    tconf_green.shadow = TEXT_SHADOW_RIGHT | TEXT_SHADOW_BOTTOM;
    tconf_green.halign = TEXT_CENTER;
    tconf_green.cshadow = TEXT_SHADOW_GREEN;

    text_settings tconf_black;
    text_defaults(&tconf_black);
    tconf_black.font = FONT_SMALL;
    tconf_black.cforeground = TEXT_BLACK;
    tconf_black.shadow = TEXT_SHADOW_TOP | TEXT_SHADOW_LEFT;
    tconf_black.halign = TEXT_CENTER;
    tconf_black.cshadow = TEXT_SHADOW_BLACK;

    // player 1 name
    text_render(&tconf_black, TEXT_DEFAULT, 0, 52, 66, 8, lang_get(20 + local->pilot_id_a));

    if(player2_is_selectable) {
        // player 2 name
        text_render(&tconf_black, TEXT_DEFAULT, 320 - 66, 52, 66, 8, lang_get(20 + local->pilot_id_b));

        // currently selected player
        object_render(&local->big_portrait_2);

        // currently selected HAR
        render_enabled_portrait(local->har_portraits, &local->cursor[1]);
        object_render(&local->har_player2[CURSOR_INDEX(local, 1)]);

        // render HAR name (Har1 VS. Har2)
        str vs_text;
        str_from_format(&vs_text, "%s VS. %s", har_get_name(CURSOR_INDEX(local, 0)),
                        har_get_name(CURSOR_INDEX(local, 1)));
        text_render(&tconf_black, TEXT_DEFAULT, 80, 107, 150, 8, str_c(&vs_text));
        str_free(&vs_text);
    } else {
        // 'choose your Robot'
        text_render(&tconf_green, TEXT_DEFAULT, 160, 97, 160, 8, lang_get(186));

        // render HAR name
        text_render(&tconf_black, TEXT_DEFAULT, 120, 107, 60, 8, har_get_name(CURSOR_INDEX(local, 0)));
    }
}

void melee_render(scene *scene) {
    melee_local *local = scene_get_userdata(scene);
    game_player *player2 = game_state_get_player(scene->gs, 1);

    if(local->page == PILOT_SELECT) {
        render_pilot_select(local, player2->selectable);
    } else {
        render_har_select(local, player2->selectable);
    }

    text_settings tconf_black;
    text_defaults(&tconf_black);
    tconf_black.font = FONT_SMALL;
    tconf_black.cforeground = TEXT_BLACK;
    tconf_black.shadow = TEXT_SHADOW_TOP | TEXT_SHADOW_LEFT;
    tconf_black.cshadow = TEXT_SHADOW_BLACK;

    if(player2->selectable) {
        int text_x = 8;
        chr_score *s1 = game_player_get_score(game_state_get_player(scene->gs, 0));
        chr_score *s2 = game_player_get_score(game_state_get_player(scene->gs, 1));
        str wins_text_a, wins_text_b;
        str_from_format(&wins_text_a, "Wins: %d", s1->wins);
        str_from_format(&wins_text_b, "Wins: %d", s2->wins);
        text_render(&tconf_black, TEXT_DEFAULT, text_x, 107, 50, 8, str_c(&wins_text_a));
        text_x = 312 - text_width(&tconf_black, str_c(&wins_text_b));
        text_render(&tconf_black, TEXT_DEFAULT, text_x, 107, 50, 8, str_c(&wins_text_b));
        str_free(&wins_text_a);
        str_free(&wins_text_b);
    }
}

static void load_pilot_portraits(scene *scene, melee_local *local) {
    sprite *current;
    portrait *target;
    animation *pilots_enabled = &bk_get_info(scene->bk_data, 3)->ani;
    for(int i = 0; i < 10; i++) {
        target = &local->pilot_portraits[i];

        // Copy the face image in full color (shown when selected)
        current = animation_get_sprite(pilots_enabled, i);
        target->x = current->pos.x;
        target->y = current->pos.y;
        surface_create_from(&target->enabled, current->data);

        // Copy the face image in dimmed color (shown when not selected)
        surface_create_from(&target->disabled, &target->enabled);
        surface_compress_index_blocks(&target->disabled, 0x60, 0xA0, 64, 16);
        surface_compress_index_blocks(&target->disabled, 0xA0, 0xD0, 8, 3);
        surface_compress_index_blocks(&target->disabled, 0xD0, 0xE0, 16, 3);
        surface_compress_index_blocks(&target->disabled, 0xE0, 0xF0, 8, 2);
        surface_compress_remap(&target->disabled, 0xF0, 0xF7, 0xB6, 3);
    }
}

static void load_har_portraits(scene *scene, melee_local *local) {
    sprite *current;
    portrait *target;
    int row, col;
    animation *har_portraits = &bk_get_info(scene->bk_data, 1)->ani;
    for(int i = 0; i < 10; i++) {
        row = i / 5;
        col = i % 5;
        target = &local->har_portraits[i];

        // Copy the HAR image in full color (shown when selected)
        current = animation_get_sprite(har_portraits, 0);
        target->x = current->pos.x + 62 * col;
        target->y = current->pos.y + 42 * row;
        surface_create_from_surface(&target->enabled, 51, 36, 62 * col, 42 * row, current->data);
        surface_set_transparency(&target->enabled, 0xD0);

        // Copy the enabled image, and compress the colors to grayscale
        surface_create_from(&target->disabled, &target->enabled);
        surface_convert_to_grayscale(&target->disabled, video_get_pal_ref(), 0xD0, 0xDF);
    }
}

static void load_hars(scene *scene, melee_local *local, bool player2_is_selectable) {
    animation *ani;
    for(int i = 0; i < 10; i++) {
        ani = &bk_get_info(scene->bk_data, 18 + i)->ani;
        object_create(&local->har_player1[i], scene->gs, vec2i_create(110, 95), vec2f_create(0, 0));
        object_set_animation(&local->har_player1[i], ani);
        object_select_sprite(&local->har_player1[i], 0);
        object_set_repeat(&local->har_player1[i], 1);

        if(player2_is_selectable) {
            ani = &bk_get_info(scene->bk_data, 18 + i)->ani;
            object_create(&local->har_player2[i], scene->gs, vec2i_create(210, 95), vec2f_create(0, 0));
            object_set_animation(&local->har_player2[i], ani);
            object_select_sprite(&local->har_player2[i], 0);
            object_set_repeat(&local->har_player2[i], 1);
            object_set_direction(&local->har_player2[i], OBJECT_FACE_LEFT);
            object_set_pal_offset(&local->har_player2[i], 48);
            object_set_pal_limit(&local->har_player2[i], 96);
        }
    }
}

int melee_create(scene *scene) {
    // Init local data
    melee_local *local = omf_calloc(1, sizeof(melee_local));
    scene_set_userdata(scene, local);

    local->cursor[1].row = 0;
    local->cursor[1].column = 4;

    game_player *player1 = game_state_get_player(scene->gs, 0);
    game_player *player2 = game_state_get_player(scene->gs, 1);

    controller *player1_ctrl = game_player_get_ctrl(player1);
    controller *player2_ctrl = game_player_get_ctrl(player2);

    palette *mpal = video_get_base_palette();
    palette_set_player_color(mpal, 0, 8, 0);
    palette_set_player_color(mpal, 0, 8, 1);
    palette_set_player_color(mpal, 0, 8, 2);
    video_force_pal_refresh();

    menu_background2_create(&local->bg_player_stats, 90, 61);
    menu_background2_create(&local->bg_player_bio, 160, 43);

    // Create a black surface for the highlight box. We modify the palette in renderer.
    unsigned char *black = omf_calloc(1, 51 * 36);
    surface_create_from_data(&local->select_hilight, 51, 36, black);
    surface_set_transparency(&local->select_hilight, -1);
    omf_free(black);

    // set up the magic controller hooks
    if(player1_ctrl && player2_ctrl) {
        if(player1_ctrl->type == CTRL_TYPE_NETWORK) {
            DEBUG("installing controller hook on controller 2");
            controller_add_hook(player2_ctrl, player1_ctrl, player1_ctrl->controller_hook);
        }

        if(player2_ctrl->type == CTRL_TYPE_NETWORK) {
            DEBUG("installing controller hook on controller 1");
            controller_add_hook(player1_ctrl, player2_ctrl, player2_ctrl->controller_hook);
        }
    }

    // Load HAR and Pilot face portraits and har sprites for the selection grid
    load_pilot_portraits(scene, local);
    load_har_portraits(scene, local);
    load_hars(scene, local, player2->selectable);

    // Load the big faces on the top corners
    animation *pilot_big_portraits = &bk_get_info(scene->bk_data, 4)->ani;
    object_create_static(&local->big_portrait_1, scene->gs);
    object_set_animation(&local->big_portrait_1, pilot_big_portraits);
    object_select_sprite(&local->big_portrait_1, 0);
    if(player2->selectable) {
        object_create(&local->big_portrait_2, scene->gs, vec2i_create(320, 0), vec2f_create(0, 0));
        object_set_animation(&local->big_portrait_2, pilot_big_portraits);
        object_select_sprite(&local->big_portrait_2, 4);
        object_set_direction(&local->big_portrait_2, OBJECT_FACE_LEFT);
    }

    // This contains the big logo and the frames
    animation *misc_stuff = &bk_get_info(scene->bk_data, 5)->ani;
    object_create_static(&local->player2_placeholder, scene->gs);
    object_set_animation(&local->player2_placeholder, misc_stuff);
    if(player2->selectable) {
        object_select_sprite(&local->player2_placeholder, 0);
    } else {
        object_select_sprite(&local->player2_placeholder, 1);
    }

    for(int i = 0; i < 2; i++) {
        local->bar_power[i] = progressbar_create(PROGRESSBAR_THEME_MELEE, PROGRESSBAR_LEFT, 50);
        local->bar_agility[i] = progressbar_create(PROGRESSBAR_THEME_MELEE, PROGRESSBAR_LEFT, 50);
        local->bar_endurance[i] = progressbar_create(PROGRESSBAR_THEME_MELEE, PROGRESSBAR_LEFT, 50);
    }
    component_layout(local->bar_power[0], 74, 12, 20 * 4, 8);
    component_layout(local->bar_agility[0], 74, 30, 20 * 4, 8);
    component_layout(local->bar_endurance[0], 74, 48, 20 * 4, 8);
    component_layout(local->bar_power[1], 320 - 66 - local->bg_player_stats.w, 12, 20 * 4, 8);
    component_layout(local->bar_agility[1], 320 - 66 - local->bg_player_stats.w, 30, 20 * 4, 8);
    component_layout(local->bar_endurance[1], 320 - 66 - local->bg_player_stats.w, 48, 20 * 4, 8);

    refresh_pilot_stats(local);
    set_cursor_colors(0);

    // initialize nova selection cheat
    memset(local->har_selected, 0, sizeof(local->har_selected));
    memset(local->katana_down_count, 0, sizeof(local->katana_down_count));

    // Set callbacks
    scene_set_input_poll_cb(scene, melee_input_tick);
    scene_set_render_cb(scene, melee_render);
    scene_set_free_cb(scene, melee_free);
    scene_set_dynamic_tick_cb(scene, melee_tick);

    // Play correct music
    audio_play_music(PSM_MENU);

    // All done
    return 0;
}
