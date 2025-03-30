#include <stdio.h>
#include <stdlib.h>

#include "game/common_defines.h"
#include "game/gui/gui_frame.h"
#include "game/gui/text_render.h"
#include "game/gui/textinput.h"
#include "game/scenes/scoreboard.h"
#include "game/utils/formatting.h"
#include "game/utils/settings.h"
#include "resources/scores.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/log.h"
#include "video/surface.h"
#include "video/vga_state.h"

#define MAX_PAGES (NUMBER_OF_ROUND_TYPES - 1)
#define CURSOR_STR "\x7f"
#define SCORE_COUNT 20

static const char *SCORE_ROW_FMT = "%-18.16s%-9s%-9s%11s";

#define TEXT_PRIMARY_COLOR 0xFD
#define TEXT_SECONDARY_COLOR 0xFE
#define TEXT_DISABLED_COLOR 0xC0
#define TEXT_ACTIVE_COLOR 0xFF
#define TEXT_INACTIVE_COLOR 0xFE
#define TEXT_SHADOW_COLOR 0xC0

typedef struct scoreboard_local {
    scoreboard data;
    score_entry pending_data;
    bool has_pending_data;
    int page;
    int new_score_slot;
    component *ti;
    gui_frame *frame;

    text *title;
    text *subtitle;
    text *scores[SCORE_COUNT];
} scoreboard_local;

static void set_title(scoreboard_local *local) {
    char row[128];
    snprintf(row, sizeof(row), "SCOREBOARD - %s", round_get_name(local->page));
    text_set_from_c(local->title, row);
}

static void set_subtitle(scoreboard_local *local) {
    char row[128];
    snprintf(row, sizeof(row), SCORE_ROW_FMT, "PLAYER NAME", "ROBOT", "PILOT", "SCORE");
    text_set_from_c(local->subtitle, row);
}

static void set_scores(scoreboard_local *local) {
    char score_text[15];
    char row[128];
    unsigned int score, har_id, pilot_id;
    char *player_name;
    int entry = 0;

    for(int r = 0; r < SCORE_COUNT; r++) {
        score = local->data.entries[local->page][entry].score;
        row[0] = 0;

        if(local->has_pending_data && r == local->new_score_slot) {
            har_id = local->pending_data.har_id;
            pilot_id = local->pending_data.pilot_id;
            score_format(local->pending_data.score, score_text, sizeof(score_text));
            snprintf(row, sizeof(row), SCORE_ROW_FMT, "", har_get_name(har_id), pilot_get_name(pilot_id), score_text);
        } else {
            har_id = local->data.entries[local->page][entry].har_id;
            pilot_id = local->data.entries[local->page][entry].pilot_id;
            player_name = local->data.entries[local->page][entry].name;
            if(score > 0) {
                score_format(score, score_text, sizeof(score_text));
                snprintf(row, sizeof(row), SCORE_ROW_FMT, player_name, har_get_name(har_id), pilot_get_name(pilot_id),
                         score_text);
            }
            entry++;
        }
        text_set_from_c(local->scores[r], row);
    }
}

/**
 * Refresh title and scores for rendering purposes.
 */
static void refresh(scoreboard_local *local) {
    set_title(local);
    set_scores(local);
}

static void scoreboard_free(scene *scene) {
    scoreboard_local *local = scene_get_userdata(scene);
    text_free(&local->title);
    text_free(&local->subtitle);
    for(int i = 0; i < SCORE_COUNT; i++) {
        text_free(&local->scores[i]);
    }
    if(local->frame) {
        gui_frame_free(local->frame);
    }
    omf_free(local);
    scene_set_userdata(scene, NULL);
}

static void handle_scoreboard_save(scoreboard_local *local) {
    int slot = local->new_score_slot;
    const char *name = textinput_value(local->ti);
    if(!strlen(name)) {
        return;
    }
    strncpy_or_truncate(local->pending_data.name, name, sizeof(local->pending_data.name));

    // Move next entries forward by one slot
    memmove(&local->data.entries[local->page][slot + 1], &local->data.entries[local->page][slot],
            sizeof(score_entry) * (20 - slot - 1));

    // Copy new entry to the right spot
    memcpy(&local->data.entries[local->page][slot], &local->pending_data, sizeof(score_entry));

    // Write to file
    scores_write(&local->data);
}

static int scoreboard_event(scene *scene, SDL_Event *event) {
    scoreboard_local *local = scene_get_userdata(scene);

    // If we are in writing mode, try handling text input
    if(local->has_pending_data) {
        return gui_frame_event(local->frame, event);
    }
    return 1;
}

static void scoreboard_tick(scene *scene, int paused) {
    scoreboard_local *local = scene_get_userdata(scene);
    if(local->has_pending_data) {
        gui_frame_tick(local->frame);
    }
}

static bool pressed_ok(ctrl_event *event) {
    return (event->event_data.action == ACT_KICK || event->event_data.action == ACT_PUNCH);
}

static void process_event(scene *scene, scoreboard_local *local, ctrl_event *event) {
    if(event->type != EVENT_TYPE_ACTION) {
        return;
    }
    if(local->has_pending_data) {
        if(pressed_ok(event)) {
            handle_scoreboard_save(local);
            local->has_pending_data = false;
            set_scores(local);
        } else {
            gui_frame_action(local->frame, event->event_data.action);
        }
    } else {
        if(event->event_data.action == ACT_ESC || pressed_ok(event)) {
            game_state_set_next(scene->gs, scene->gs->next_next_id);
        } else if(event->event_data.action == ACT_LEFT) {
            local->page = (local->page > 0) ? local->page - 1 : 0;
            refresh(local);
        } else if(event->event_data.action == ACT_RIGHT) {
            local->page = (local->page < MAX_PAGES) ? local->page + 1 : MAX_PAGES;
            refresh(local);
        }
    }
}

static void scoreboard_input_tick(scene *scene) {
    scoreboard_local *local = scene_get_userdata(scene);
    ctrl_event *p1 = NULL, *i;
    game_state_menu_poll(scene->gs, &p1);
    i = p1;
    if(i) {
        do {
            process_event(scene, local, i);
        } while((i = i->next) != NULL);
    }
    controller_free_chain(p1);
}

static void scoreboard_render_overlay(scene *scene) {
    scoreboard_local *local = scene_get_userdata(scene);
    text_draw(local->title, 0, 5);
    text_draw(local->subtitle, 20, 20);
    for(int i = 0; i < SCORE_COUNT; i++) {
        text_draw(local->scores[i], 20, 30 + i * 8);
    }
    if(local->has_pending_data) {
        gui_frame_render(local->frame);
    }
}

/**
 * Attempts to check for any score that is pending from a finished single player game
 */
static bool found_pending_score(scene *scene) {
    return (game_state_get_player(scene->gs, 1)->ctrl != NULL &&
            game_state_get_player(scene->gs, 1)->ctrl->type == CTRL_TYPE_AI &&
            game_state_get_player(scene->gs, 0)->score.score > 0);
}

/**
 * Check if score is high enough to be put on high score list
 */
static bool score_fits_scoreboard(scoreboard_local *local, unsigned int score) {
    for(int i = 0; i < SCORE_COUNT; i++) {
        if(score > local->data.entries[local->page][i].score) {
            return true;
        }
    }
    return false;
}

int scoreboard_create(scene *scene) {
    // Init local data
    scoreboard_local *local = omf_calloc(1, sizeof(scoreboard_local));
    local->page = settings_get()->gameplay.rounds;
    local->has_pending_data = false;
    local->new_score_slot = -1;

    // Darken the colors for the background a bit.
    vga_state_mul_base_palette(0, 0xEF, 0.25f);

    // Arena menu theme
    gui_theme theme;
    gui_theme_defaults(&theme);
    theme.dialog.border_color = TEXT_MEDIUM_GREEN;
    theme.text.primary_color = TEXT_PRIMARY_COLOR;
    theme.text.secondary_color = TEXT_SECONDARY_COLOR;
    theme.text.disabled_color = TEXT_DISABLED_COLOR;
    theme.text.active_color = TEXT_ACTIVE_COLOR;
    theme.text.inactive_color = TEXT_INACTIVE_COLOR;
    theme.text.shadow_color = TEXT_SHADOW_COLOR;

    // Load scores
    if(scores_read(&local->data) == 1) {
        scores_clear(&local->data);
        log_debug("No score data found; using empty score array.");
    }

    // Check for pending score
    if(found_pending_score(scene)) {
        game_player *player = game_state_get_player(scene->gs, 0);
        unsigned int score = player->score.score;
        if(score_fits_scoreboard(local, score)) {
            local->has_pending_data = true;
            local->pending_data.score = score;
            local->pending_data.har_id = player->pilot->har_id;
            local->pending_data.pilot_id = player->pilot->pilot_id;
            local->pending_data.name[0] = 0;
        }

        // Wipe old score data, whether it was written on scoreboard or not.
        chr_score_reset(game_player_get_score(player), 1);
    }

    // If we need to add new data to the scoreboard, try to find a slot for it.
    if(local->has_pending_data) {
        for(int r = 0; r < SCORE_COUNT; r++) {
            unsigned int score = local->data.entries[local->page][r].score;
            if(score < local->pending_data.score) {
                local->new_score_slot = r;
                log_debug("Found slot %d for new high score!", r);
                break;
            }
        }
    }

    if(local->new_score_slot > -1) {
        local->frame = gui_frame_create(&theme, 20, 30 + local->new_score_slot * 8, 96, 10);
        local->ti = textinput_create(15, "", "");
        textinput_set_font(local->ti, FONT_SMALL);
        textinput_enable_background(local->ti, false);
        textinput_set_horizontal_align(local->ti, TEXT_ALIGN_LEFT);
        gui_frame_set_root(local->frame, local->ti);
        gui_frame_layout(local->frame);
        component_select(local->ti, 1);
    } else {
        log_debug("New high score does not fit into the top 20 -- skipping input.");
        local->has_pending_data = false;
    }

    // Set up the top page title -- this will change when browsing left or right.
    local->title = text_create_with_font_and_size(FONT_BIG, 320, 6);
    text_set_color(local->title, TEXT_PRIMARY_COLOR);
    text_set_horizontal_align(local->title, TEXT_ALIGN_CENTER);
    set_title(local);

    // Set up the score column header row
    local->subtitle = text_create_with_font_and_size(FONT_SMALL, 290, 6);
    text_set_color(local->subtitle, TEXT_PRIMARY_COLOR);
    set_subtitle(local);

    // Set up score rows and load values for them.
    for(int i = 0; i < SCORE_COUNT; i++) {
        local->scores[i] = text_create_with_font_and_size(FONT_SMALL, 290, 6);
        text_set_color(local->scores[i], TEXT_PRIMARY_COLOR);
    }
    set_scores(local);

    // Set callbacks
    scene_set_userdata(scene, local);
    scene_set_event_cb(scene, scoreboard_event);
    scene_set_input_poll_cb(scene, scoreboard_input_tick);
    scene_set_render_overlay_cb(scene, scoreboard_render_overlay);
    scene_set_free_cb(scene, scoreboard_free);
    scene_set_static_tick_cb(scene, scoreboard_tick);

    // All done
    return 0;
}
