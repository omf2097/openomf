#include <math.h>
#include <stdlib.h>

#include "audio/audio.h"
#include "formats/pilot.h"
#include "game/game_state.h"
#include "game/gui/menu_background.h"
#include "game/gui/progressbar.h"
#include "game/gui/text/text.h"
#include "game/protos/object.h"
#include "game/protos/scene.h"
#include "game/scenes/melee.h"
#include "resources/animation.h"
#include "resources/bk.h"
#include "resources/languages.h"
#include "resources/pilots.h"
#include "resources/sprite.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/log.h"
#include "utils/random.h"
#include "video/vga_state.h"
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

enum
{
    STAT_POWER,
    STAT_AGILITY,
    STAT_ENDURANCE,
    STAT_COUNT
};

typedef struct {
    cursor_data cursor[2];
    selection_page page;

    object big_portrait_1;
    object big_portrait_2;
    object player2_placeholder;
    object unselected_pilot_portraits;
    object unselected_har_portraits;

    portrait pilot_portraits[10];
    portrait har_portraits[10];

    object har[2];

    component *bar_stat[2][STAT_COUNT];

    int pilot_id_a;
    int pilot_id_b;

    surface bg_player_stats;
    surface bg_player_bio;
    surface select_hilight;

    unsigned int ticks;
    unsigned int tickbase[2];

    text *wins[2];
    text *har_title;
    text *titles[2];
    text *player_name[2];
    text *player_bio[2];
    text *player_stats[3];

    // nova selection cheat
    unsigned char cheat_selected[2][10];
    unsigned char katana_down_count[2];
    // pilot stat cheat. wrap around on both rows and select every pilot, then hold KICK to adjust stats.
    unsigned char cheat_pilot_stats[2];
    unsigned char cheat_pilot_stats_stat[2]; // which stat is selected?

    bool network_game;
} melee_local;

void handle_action(scene *scene, int player, int action);

void melee_free(scene *scene) {
    melee_local *local = scene_get_userdata(scene);
    game_player *player2 = game_state_get_player(scene->gs, 1);

    surface_free(&local->bg_player_stats);
    surface_free(&local->bg_player_bio);
    surface_free(&local->select_hilight);
    for(int i = 0; i < 2; i++) {
        for(int j = 0; j < STAT_COUNT; j++) {
            component_free(local->bar_stat[i][j]);
        }
    }

    object_free(&local->har[0]);
    object_free(&local->har[1]);

    for(int i = 0; i < 10; i++) {
        surface_free(&local->har_portraits[i].enabled);
        surface_free(&local->har_portraits[i].disabled);
        surface_free(&local->pilot_portraits[i].enabled);
        surface_free(&local->pilot_portraits[i].disabled);
    }

    for(int i = 0; i < 2; i++) {
        text_free(&local->player_bio[i]);
        text_free(&local->player_name[i]);
        text_free(&local->titles[i]);
        text_free(&local->wins[i]);
    }
    for(int i = 0; i < 3; i++) {
        text_free(&local->player_stats[i]);
    }
    text_free(&local->har_title);

    object_free(&local->player2_placeholder);
    object_free(&local->unselected_pilot_portraits);
    object_free(&local->unselected_har_portraits);
    object_free(&local->big_portrait_1);
    if(player2->selectable) {
        object_free(&local->big_portrait_2);
    }
    omf_free(local);
    scene_set_userdata(scene, local);
}

static int ticks_to_blinky(int ticks) {
    float rate = ((float)ticks) / 25.0f;
    int offset = roundf((cosf(rate) + 1.0f) * 64.0f);
    return offset;
}

static void set_cursor_colors(int a_ticks, int b_ticks, bool a_done, bool b_done) {
    int base = 120;
    int a_offset = ticks_to_blinky(a_ticks);
    int b_offset = ticks_to_blinky(b_ticks);

    vga_color red_cursor_color = {base + (a_done ? 64 : a_offset), 0, 0};
    vga_color blue_cursor_color = {0, 0, base + (b_done ? 64 : b_offset)};
    vga_color violet_cursor_color = {base + a_offset, 0, base + a_offset};

    vga_state_set_base_palette_index(RED_CURSOR_INDEX, &red_cursor_color);
    vga_state_set_base_palette_index(BLUE_CURSOR_INDEX, &blue_cursor_color);
    vga_state_set_base_palette_index(VIOLET_CURSOR_INDEX, &violet_cursor_color);
}

static void load_pilot_portraits_palette(scene *scene) {
    vga_palette *bk_pal = bk_get_palette(scene->bk_data, 0);
    // copy and dim for unselected pilot portraits
    for(uint8_t idx = 0x01; idx < 0x60; idx++) {
        uint8_t src_idx = idx + 0xA0;
        vga_color src = bk_pal->colors[src_idx];
        src.r /= 2;
        src.g /= 2;
        src.b /= 2;
        bk_pal->colors[idx] = src;
    }
    vga_state_set_base_palette_from_range(bk_pal, 0x00, 0x00, 0x60);
}

static bool check_pilot_stat_cheat(scene *scene, int player_id) {
    melee_local *local = scene_get_userdata(scene);

    // if we haven't enabled the cheat yet, check the rules
    if(local->cheat_pilot_stats[player_id] != 0xFF) {
        if(local->cheat_pilot_stats[player_id] != 3)
            return false; // haven't wrapped around both rows
        if(memchr(local->cheat_selected[player_id], 0, sizeof(local->cheat_selected[player_id])) != NULL)
            return false; // not all pilots have been selected

        // mark cheat as enabled
        local->cheat_pilot_stats[player_id] = 0xFF;

        // select POWER stat.
        local->cheat_pilot_stats_stat[player_id] = 0;
    }

    // disable KICK debouncing
    game_player *player = game_state_get_player(scene->gs, player_id);
    game_player_get_ctrl(player)->last &= ~ACT_KICK;

    return true;
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
                handle_action(scene, 0, i->event_data.action);
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
                handle_action(scene, 1, i->event_data.action);
            } else if(i->type == EVENT_TYPE_CLOSE) {
                game_state_set_next(scene->gs, SCENE_MENU);
                return;
            }
        } while((i = i->next) != NULL);
    }

    if(local->page == HAR_SELECT && local->ticks % 10 == 1) {
        object_dynamic_tick(&local->har[0]);
        if(player2->selectable) {
            object_dynamic_tick(&local->har[1]);
        }
    }

    // Tick cursor colors
    set_cursor_colors(local->ticks - local->tickbase[0], local->ticks - local->tickbase[1], CURSOR_A_DONE(local),
                      CURSOR_B_DONE(local));
    local->ticks++;
}

static uint8_t *get_pilot_stat(sd_pilot *p, int stat) {
    switch(stat) {
        default:
            assert(0);
        case STAT_POWER:
            return &p->power;
        case STAT_AGILITY:
            return &p->agility;
        case STAT_ENDURANCE:
            return &p->endurance;
    }
}

static void refresh_pilot_stats(scene *scene, int player_id) {
    melee_local *local = scene_get_userdata(scene);
    game_player *player = game_state_get_player(scene->gs, player_id);
    for(int stat = 0; stat < STAT_COUNT; stat++) {
        progressbar_set_progress(local->bar_stat[player_id][stat],
                                 (*get_pilot_stat(player->pilot, stat) * 100) / MAX_STAT, false);
    }
}

void update_har(scene *scene, int player) {
    melee_local *local = scene_get_userdata(scene);
    if(local->page == HAR_SELECT) {
        game_player *player2 = game_state_get_player(scene->gs, 1);
        animation *ani;
        object *har = &local->har[player];

        ani = &bk_get_info(scene->bk_data, 18 + CURSOR_INDEX(local, player))->ani;
        object_set_animation(har, ani);
        object_select_sprite(har, 0);
        object_set_repeat(har, 1);
        if(player2->selectable) {
            str tmp;
            str_from_format(&tmp, "%s VS. %s", har_get_name(CURSOR_INDEX(local, 0)),
                            har_get_name(CURSOR_INDEX(local, 1)));
            text_set_from_str(local->har_title, &tmp);
            str_free(&tmp);
        } else {
            text_set_from_c(local->har_title, har_get_name(CURSOR_INDEX(local, 0)));
        }
    }
}

static void reset_cursor_blinky(melee_local *local, int player) {
    if(CURSORS_MATCH(local) || player == 0) {
        local->tickbase[0] = local->ticks;
    }
    if(CURSORS_MATCH(local) || player == 1) {
        local->tickbase[1] = local->ticks;
    }
}

static void load_pilot_stats(scene *scene, int player_id) {
    game_player *player = game_state_get_player(scene->gs, player_id);
    melee_local *local = scene_get_userdata(scene);

    int pilot_id = player_id == 0 ? local->pilot_id_a : local->pilot_id_b;

    pilot p_a;
    pilot_get_info(&p_a, pilot_id);

    if(player->selectable) {
        object *big_portrait = player_id == 0 ? &local->big_portrait_1 : &local->big_portrait_2;
        object_select_sprite(big_portrait, pilot_id);

        text_set_from_c(local->player_bio[player_id], lang_get(135 + pilot_id));
        text_set_from_c(local->player_name[player_id], lang_get(20 + pilot_id));
        object_select_sprite(big_portrait, pilot_id);
    }

    player->pilot->endurance = p_a.endurance;
    player->pilot->power = p_a.power;
    player->pilot->agility = p_a.agility;
    player->pilot->sex = p_a.sex;
    refresh_pilot_stats(scene, player_id);
}

static void load_pilot_colors(scene *scene, int player_id) {
    game_player *player = game_state_get_player(scene->gs, player_id);
    melee_local *local = scene_get_userdata(scene);

    int pilot_id = player_id == 0 ? local->pilot_id_a : local->pilot_id_b;

    pilot p_a;
    pilot_get_info(&p_a, pilot_id);

    // update the player palette
    sd_pilot_set_player_color(player->pilot, PRIMARY, p_a.color_1);
    sd_pilot_set_player_color(player->pilot, SECONDARY, p_a.color_2);
    sd_pilot_set_player_color(player->pilot, TERTIARY, p_a.color_3);
    palette_load_player_colors(&player->pilot->palette, player_id);
}

void handle_action(scene *scene, int player, int action) {
    game_player *player1 = game_state_get_player(scene->gs, 0);
    game_player *player2 = game_state_get_player(scene->gs, 1);
    melee_local *local = scene_get_userdata(scene);
    int *row = &local->cursor[player].row;
    int *column = &local->cursor[player].column;
    bool *done = &local->cursor[player].done;

    if(*done) {
        return;
    }

    int old_row = *row;
    int old_column = *column;

    switch(action) {
        case ACT_STOP:
            progressbar_set_highlight(local->bar_stat[player][local->cheat_pilot_stats_stat[player]], 0);
            break;
        case ACT_LEFT:
            (*column)--;
            if(*column < 0) {
                *column = 4;
                if(local->page == PILOT_SELECT) {
                    local->cheat_pilot_stats[player] |= *row + 1;
                }
            }
            reset_cursor_blinky(local, player);
            break;
        case ACT_RIGHT:
            (*column)++;
            if(*column > 4) {
                *column = 0;
                if(local->page == PILOT_SELECT) {
                    local->cheat_pilot_stats[player] |= *row + 1;
                }
            }
            reset_cursor_blinky(local, player);
            break;
        case ACT_UP:
            if(*row == 1) {
                *row = 0;
            }
            reset_cursor_blinky(local, player);
            break;
        case ACT_DOWN:
            if(*row == 0) {
                *row = 1;
            }
            reset_cursor_blinky(local, player);
            // nova selection cheat
            if(*row == 1 && *column == 0) {
                local->katana_down_count[player]++;
                if(local->katana_down_count[player] > 11) {
                    local->katana_down_count[player] = 11;
                }
            }
            break;
        case ACT_KICK | ACT_UP:
            if(local->page == PILOT_SELECT && check_pilot_stat_cheat(scene, player) &&
               local->cheat_pilot_stats_stat[player] > 0) {
                progressbar_set_highlight(local->bar_stat[player][local->cheat_pilot_stats_stat[player]], 0);
                local->cheat_pilot_stats_stat[player]--;
            }
            break;
        case ACT_KICK | ACT_DOWN:
            if(local->page == PILOT_SELECT && check_pilot_stat_cheat(scene, player) &&
               local->cheat_pilot_stats_stat[player] < 2) {
                progressbar_set_highlight(local->bar_stat[player][local->cheat_pilot_stats_stat[player]], 0);
                local->cheat_pilot_stats_stat[player]++;
            }
            break;
        case ACT_KICK | ACT_LEFT:
            if(local->page == PILOT_SELECT && check_pilot_stat_cheat(scene, player)) {
                sd_pilot *pilot = game_state_get_player(scene->gs, player)->pilot;
                int stat = local->cheat_pilot_stats_stat[player];
                uint8_t *stat_val = get_pilot_stat(pilot, stat);
                // reallocate points to other stats from selected stat
                for(int other = 0; other < STAT_COUNT; other++) {
                    uint8_t *other_val = get_pilot_stat(pilot, other);
                    if(other == stat || *other_val >= MAX_STAT || *stat_val <= 0)
                        continue;
                    (*other_val)++;
                    (*stat_val)--;
                }
                refresh_pilot_stats(scene, player);
            }
            break;
        case ACT_KICK | ACT_RIGHT:
            if(local->page == PILOT_SELECT && check_pilot_stat_cheat(scene, player)) {
                sd_pilot *pilot = game_state_get_player(scene->gs, player)->pilot;
                int stat = local->cheat_pilot_stats_stat[player];
                uint8_t *stat_val = get_pilot_stat(pilot, stat);
                // reallocate points from other stats into selected stat
                for(int other = 0; other < STAT_COUNT; other++) {
                    uint8_t *other_val = get_pilot_stat(pilot, other);
                    if(other == stat || *other_val <= 0 || *stat_val >= MAX_STAT)
                        continue;
                    (*other_val)--;
                    (*stat_val)++;
                }
                refresh_pilot_stats(scene, player);
            }
            break;
        case ACT_KICK:
            if(local->page == PILOT_SELECT && check_pilot_stat_cheat(scene, player)) {
                progressbar_set_highlight(local->bar_stat[player][local->cheat_pilot_stats_stat[player]], 1);
                break;
            }
            // [[fallthrough]]
        case ACT_PUNCH:
            *done = 1;
            audio_play_sound(20, 0.5f, 0.0f, 0);
            if(CURSOR_A_DONE(local) && (CURSOR_B_DONE(local) || !player2->selectable)) {
                local->cursor[0].done = 0;
                local->cursor[1].done = 0;
                if(local->page == PILOT_SELECT) {
                    local->page = HAR_SELECT;
                    update_har(scene, 0);
                    update_har(scene, 1);
                    local->pilot_id_a = CURSOR_INDEX(local, 0);
                    local->pilot_id_b = CURSOR_INDEX(local, 1);

                    // prepare for nova selection cheat
                    memset(local->cheat_selected, 0, sizeof(local->cheat_selected));
                    local->cheat_selected[0][local->pilot_id_a] = 1;
                    local->cheat_selected[1][local->pilot_id_b] = 1;

                    load_pilot_colors(scene, 0);

                    if(player2->selectable) {
                        load_pilot_colors(scene, 1);
                    }
                } else {
                    int nova_activated[2] = {1, 1};
                    for(int i = 0; i < 2; i++) {
                        nova_activated[i] = local->katana_down_count[i] >= 11 &&
                                            !memchr(local->cheat_selected[i], 0, sizeof(local->cheat_selected[i]));
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

                        local->pilot_id_b = player2->pilot->pilot_id;
                        load_pilot_stats(scene, player);
                        load_pilot_colors(scene, 1);
                    }
                    if(!local->network_game) {
                        strncpy_or_truncate(player1->pilot->name, lang_get(player1->pilot->pilot_id + 20),
                                            sizeof(player1->pilot->name));
                        // TODO: lang: remove (the need for) newline stripping
                        // 1player name strings end in a newline...
                        if(player1->pilot->name[strlen(player1->pilot->name) - 1] == '\n') {
                            player1->pilot->name[strlen(player1->pilot->name) - 1] = 0;
                        }
                        strncpy_or_truncate(player2->pilot->name, lang_get(player2->pilot->pilot_id + 20),
                                            sizeof(player2->pilot->name));
                        // TODO: lang: remove (the need for) newline stripping
                        // 1player name strings end in a newline...
                        if(player2->pilot->name[strlen(player2->pilot->name) - 1] == '\n') {
                            player2->pilot->name[strlen(player2->pilot->name) - 1] = 0;
                        }
                    }
                    game_state_set_next(scene->gs, SCENE_VS);
                }
            }
            break;
    }

    if(old_row != *row || old_column != *column) {
        float panning = (float)(*column) * (2.0f / 5.0f) - 0.5f;
        audio_play_sound(19, 0.5f, panning, 0);
        if(local->page == PILOT_SELECT) {
            if(player == 0) {
                local->pilot_id_a = CURSOR_INDEX(local, player);
            } else {
                local->pilot_id_b = CURSOR_INDEX(local, player);
            }
            load_pilot_stats(scene, player);
        } else {
            update_har(scene, player);
        }
    }

#if 0
    if(local->page == PILOT_SELECT) {
    }

    refresh_pilot_stats(local);
#endif

    local->cheat_selected[player][5 * (*row) + *column] = 1;
}

// Move cursors to select Pilot or HARs specified by A and B.
static void restore_cursors_to(melee_local *local, unsigned a, unsigned b) {
    // note: a/b can be HAR_NOVA (10) when returning from VS,
    // which the original game handles surprisingly elegantly.
    // If we try to naively do that, our pointers all explode.
    if(a == HAR_NOVA) {
        a = HAR_FLAIL;
    }
    if(b == HAR_NOVA) {
        b = HAR_FLAIL;
    }
    local->cursor[0].column = a % 5;
    local->cursor[0].row = a / 5;
    local->cursor[0].done = 0;
    local->cursor[1].column = b % 5;
    local->cursor[1].row = b / 5;
    local->cursor[1].done = 0;
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
                handle_action(scene, 0, i->event_data.action);
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
                handle_action(scene, 1, i->event_data.action);
            } else if(i->type == EVENT_TYPE_CLOSE) {
                game_state_set_next(scene->gs, SCENE_MENU);
            }
        } while((i = i->next) != NULL);
    }
    controller_free_chain(p2);

    ctrl_event *menu_ev = NULL;
    game_state_menu_poll(scene->gs, &menu_ev);

    for(i = menu_ev; i; i = i->next) {
        if(i->type == EVENT_TYPE_ACTION && i->event_data.action == ACT_ESC) {
            audio_play_sound(20, 0.5f, 0.0f, 0);
            if(local->page == HAR_SELECT) {
                // restore the player selection
                restore_cursors_to(local, local->pilot_id_a, local->pilot_id_b);
                local->page = PILOT_SELECT;
                load_pilot_portraits_palette(scene);
            } else {
                game_state_set_next(scene->gs, SCENE_MENU);
            }
        }
    }
    controller_free_chain(menu_ev);
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

static void render_enabled_portrait(const portrait *portraits, cursor_data *cursor, int player) {
    const portrait *p = &portraits[5 * cursor->row + cursor->column];
    if(player < 0) {
        video_draw(&p->enabled, p->x, p->y);
    } else {
        video_draw_offset(&p->enabled, p->x, p->y, player * 48, (player + 1) * 48);
    }
}

static text *create_green_text(int w, int h, const char *str) {
    text *t = text_create_with_font_and_size(FONT_SMALL, w, h);
    text_set_color(t, TEXT_GREEN);
    text_set_shadow_style(t, GLYPH_SHADOW_RIGHT | GLYPH_SHADOW_BOTTOM);
    text_set_shadow_color(t, TEXT_SHADOW_GREEN);
    text_set_horizontal_align(t, TEXT_ALIGN_CENTER);
    text_set_vertical_align(t, TEXT_ALIGN_MIDDLE);
    text_set_from_c(t, str);
    return t;
}

static text *create_black_text(int w, int h, const char *str) {
    text *t = text_create_with_font_and_size(FONT_SMALL, w, h);
    text_set_color(t, TEXT_BLACK);
    text_set_shadow_style(t, GLYPH_SHADOW_TOP | GLYPH_SHADOW_LEFT);
    text_set_shadow_color(t, TEXT_SHADOW_BLACK);
    text_set_horizontal_align(t, TEXT_ALIGN_CENTER);
    text_set_vertical_align(t, TEXT_ALIGN_MIDDLE);
    text_set_from_c(t, str);
    return t;
}

static void render_pilot_select(melee_local *local, bool player2_is_selectable) {
    video_draw(&local->bg_player_stats, 70, 0);
    video_draw(&local->bg_player_bio, 0, 62);

    text_draw(local->player_name[0], 0, 52);
    text_draw(local->player_bio[0], 4, 66);
    text_draw(local->player_stats[0], 74, 4);
    text_draw(local->player_stats[1], 74, 22);
    text_draw(local->player_stats[2], 74, 40);

    for(int stat = 0; stat < STAT_COUNT; stat++) {
        component_render(local->bar_stat[0][stat]);
    }

    object_render(&local->player2_placeholder);

    if(player2_is_selectable) {
        video_draw(&local->bg_player_stats, 320 - 70 - local->bg_player_stats.w, 0);
        video_draw(&local->bg_player_bio, 320 - local->bg_player_bio.w, 62);

        text_draw(local->player_name[1], 320 - 66, 52);
        text_draw(local->player_bio[1], 320 - local->bg_player_bio.w + 4, 66);
        text_draw(local->player_stats[0], 320 - 66 - local->bg_player_stats.w, 4);
        text_draw(local->player_stats[1], 320 - 66 - local->bg_player_stats.w, 22);
        text_draw(local->player_stats[2], 320 - 66 - local->bg_player_stats.w, 40);

        for(int stat = 0; stat < STAT_COUNT; stat++) {
            component_render(local->bar_stat[1][stat]);
        }
    } else {
        text_draw(local->titles[0], 160, 97);
    }

    object_render(&local->unselected_pilot_portraits);
    render_highlights(local, player2_is_selectable);
    render_enabled_portrait(local->pilot_portraits, &local->cursor[0], -1);
    object_render(&local->big_portrait_1);
    if(player2_is_selectable) {
        render_enabled_portrait(local->pilot_portraits, &local->cursor[1], -1);
        object_render(&local->big_portrait_2);
    }
}

static void render_har_select(melee_local *local, bool player2_is_selectable) {
    object_render(&local->player2_placeholder);

    // render the stupid unselected HAR portraits before anything
    // so we can render anything else on top of them
    object_render(&local->unselected_har_portraits);
    render_highlights(local, player2_is_selectable);

    // currently selected player
    object_render(&local->big_portrait_1);

    // currently selected HAR
    render_enabled_portrait(local->har_portraits, &local->cursor[0], 0);
    object_render(&local->har[0]);

    // player 1 name
    text_draw(local->player_name[0], 0, 52);

    if(player2_is_selectable) {
        // player 2 name
        text_draw(local->player_name[1], 320 - 66, 52);

        // currently selected player
        object_render(&local->big_portrait_2);

        // currently selected HAR
        render_enabled_portrait(local->har_portraits, &local->cursor[1], 1);
        object_render(&local->har[1]);

        // render HAR name (Har1 VS. Har2)
        text_draw(local->har_title, 0, 107);
    } else {
        // 'choose your Robot'
        text_draw(local->titles[1], 160, 97);

        // render HAR name
        text_draw(local->har_title, 0, 107);
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

    if(player2->selectable) {
        text_draw(local->wins[0], 8, 107);
        text_draw(local->wins[1], 160, 107);
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

    animation *har_portraits = &bk_get_info(scene->bk_data, 0)->ani;
    object_create_static(&local->unselected_pilot_portraits, scene->gs);
    object_set_animation(&local->unselected_pilot_portraits, har_portraits);
    object_select_sprite(&local->unselected_pilot_portraits, 0);
}

static void load_har_portraits(scene *scene, melee_local *local) {
    portrait *target;
    int row, col;
    animation *har_portraits = &bk_get_info(scene->bk_data, 1)->ani;
    sprite *sheet = animation_get_sprite(har_portraits, 0);

    for(int i = 0; i < 10; i++) {
        row = i / 5;
        col = i % 5;
        target = &local->har_portraits[i];

        // Copy the HAR image in full color (shown when selected)
        target->x = sheet->pos.x + 62 * col;
        target->y = sheet->pos.y + 42 * row;
        surface_create_from_surface(&target->enabled, 51, 36, 62 * col, 42 * row, sheet->data);
        surface_set_transparency(&target->enabled, 0xD0);
    }

    // convert BK's sheet sprite to grayscale and use it for the unselected_har_portraits
    surface_convert_har_to_grayscale(sheet->data, 8);
    object_create_static(&local->unselected_har_portraits, scene->gs);
    object_set_animation(&local->unselected_har_portraits, har_portraits);
    object_select_sprite(&local->unselected_har_portraits, 0);
}

static void load_hars(scene *scene, melee_local *local, bool player2_is_selectable) {
    object_create(&local->har[0], scene->gs, vec2i_create(110, 95), vec2f_create(0, 0));
    update_har(scene, 0);
    if(player2_is_selectable) {
        object_create(&local->har[1], scene->gs, vec2i_create(210, 95), vec2f_create(0, 0));
        update_har(scene, 1);
        object_set_direction(&local->har[1], OBJECT_FACE_LEFT);
        object_set_pal_offset(&local->har[1], 48);
        object_set_pal_limit(&local->har[1], 96);
    }
}

int melee_event_cb(scene *scene, SDL_Event *e) {
    melee_local *local = scene_get_userdata(scene);
    if(local->page == HAR_SELECT && e->type == SDL_KEYDOWN && SDLK_1 <= e->key.keysym.sym &&
       e->key.keysym.sym <= SDLK_6) {
        // color cheat:
        // press 1-3 to change pilot1 colors
        // press 4-6 to change pilot2 colors in 2 player
        int idx = e->key.keysym.sym - SDLK_1;
        int pal_id = idx % 3;
        int player_id = idx / 3;

        game_player *player = game_state_get_player(scene->gs, player_id);
        uint8_t color = sd_pilot_get_player_color(player->pilot, pal_id);
        color = (color + 1) % 16;
        sd_pilot_set_player_color(player->pilot, pal_id, color);
        palette_load_player_colors(&player->pilot->palette, player_id);
    }
    return 1;
}

int melee_create(scene *scene) {
    // Init local data
    melee_local *local = omf_calloc(1, sizeof(melee_local));
    scene_set_userdata(scene, local);
    scene->event = melee_event_cb;

    game_player *player1 = game_state_get_player(scene->gs, 0);
    game_player *player2 = game_state_get_player(scene->gs, 1);

    local->network_game = false;
    if(player1->ctrl && player2->ctrl) {
        if(player1->ctrl->type == CTRL_TYPE_NETWORK || player2->ctrl->type == CTRL_TYPE_NETWORK) {
            local->network_game = true;
        }
    }

    // if we already have a pilot name, we're coming back from VS.
    if(!local->network_game && player1->pilot->name[0] != '\0' &&
       (player2->pilot->name[0] != '\0' || !player2->selectable)) {
        local->page = HAR_SELECT;
        local->pilot_id_a = player1->pilot->pilot_id;
        local->pilot_id_b = player2->pilot->pilot_id;
        restore_cursors_to(local, player1->pilot->har_id, player2->pilot->har_id);

        palette_load_player_colors(&player1->pilot->palette, 0);
        palette_load_player_colors(&player2->pilot->palette, 1);
    } else {
        local->page = PILOT_SELECT;
        local->pilot_id_a = PILOT_CRYSTAL;
        local->pilot_id_b = PILOT_SHIRRO;
        restore_cursors_to(local, local->pilot_id_a, local->pilot_id_b);

        load_pilot_portraits_palette(scene);
    }

    controller *player1_ctrl = game_player_get_ctrl(player1);
    controller *player2_ctrl = game_player_get_ctrl(player2);

    menu_background_create(&local->bg_player_stats, 90, 61, MenuBackgroundMeleeVs);
    menu_background_create(&local->bg_player_bio, 160, 43, MenuBackgroundMeleeVs);

    // Player bio boxes and names for both players
    for(int i = 0; i < 2; i++) {
        local->player_bio[i] = create_green_text(156, 34, lang_get(135 + CURSOR_INDEX(local, i)));
        local->player_name[i] = create_black_text(66, 6, lang_get(20 + CURSOR_INDEX(local, i)));
        text_set_margin(local->player_bio[i], (text_margin){2, 2, 0, 0});
    }
    // Stats texts (POWER, AGILITY, ENDURANCE)
    for(int i = 0; i < 3; i++) {
        local->player_stats[i] = create_green_text(85, 6, lang_get(216 + i));
    }

    // Page titles. These are static.
    local->titles[0] = create_green_text(160, 6, lang_get(187)); // 'choose your pilot'
    local->titles[1] = create_green_text(160, 6, lang_get(186)); // 'choose your robot'

    // This is used to either show the selected HAR name or both in the "X VS. Y" format.
    local->har_title = create_black_text(320, 6, har_get_name(CURSOR_INDEX(local, 0)));

    // Create a black surface for the highlight box. We modify the palette in renderer.
    unsigned char *black = omf_calloc(1, 51 * 36);
    surface_create_from_data(&local->select_hilight, 51, 36, black);
    surface_set_transparency(&local->select_hilight, -1);
    omf_free(black);

    // set up the magic controller hooks
    if(player1_ctrl && player2_ctrl) {
        if(player1_ctrl->type == CTRL_TYPE_NETWORK) {
            log_debug("installing controller hook on controller 2");
            controller_clear_hooks(player2_ctrl);
            controller_add_hook(player2_ctrl, player1_ctrl, player1_ctrl->controller_hook);
            controller_add_hook(scene->gs->menu_ctrl, player1_ctrl, menu_controller_hook);
        }

        if(player2_ctrl->type == CTRL_TYPE_NETWORK) {
            log_debug("installing controller hook on controller 1");
            controller_clear_hooks(player1_ctrl);
            controller_add_hook(player1_ctrl, player2_ctrl, player2_ctrl->controller_hook);
            controller_add_hook(scene->gs->menu_ctrl, player2_ctrl, menu_controller_hook);
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
        chr_score *s1 = game_player_get_score(game_state_get_player(scene->gs, 0));
        chr_score *s2 = game_player_get_score(game_state_get_player(scene->gs, 1));

        char tmp[64];
        snprintf(tmp, sizeof(tmp), "Wins: %d", s1->wins);
        local->wins[0] = create_black_text(160 - 8, 6, tmp);
        text_set_horizontal_align(local->wins[0], TEXT_ALIGN_LEFT);
        snprintf(tmp, sizeof(tmp), "Wins: %d", s2->wins);
        local->wins[1] = create_black_text(160 - 8, 6, tmp);
        text_set_horizontal_align(local->wins[1], TEXT_ALIGN_RIGHT);

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
        int x = i == 0 ? (74) : (320 - 66 - local->bg_player_stats.w);
        for(int stat = 0; stat < STAT_COUNT; stat++) {
            int y = 12 + stat * 18;
            local->bar_stat[i][stat] = progressbar_create(PROGRESSBAR_THEME_MELEE, PROGRESSBAR_LEFT, 50);
            component_layout(local->bar_stat[i][stat], x, y, 20 * 4, 8);
        }
    }

    load_pilot_stats(scene, 0);
    load_pilot_stats(scene, 1);
    // refresh_pilot_stats(local);
    set_cursor_colors(0, 0, false, false);

    // initialize cheats
    memset(local->cheat_selected, 0, sizeof(local->cheat_selected));
    memset(local->katana_down_count, 0, sizeof(local->katana_down_count));
    memset(local->cheat_pilot_stats, 0, sizeof(local->cheat_pilot_stats));

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
