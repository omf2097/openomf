#include <stdio.h>
#include <stdlib.h>

#include "game/common_defines.h"
#include "game/gui/frame.h"
#include "game/gui/text_render.h"
#include "game/gui/textinput.h"
#include "game/scenes/scoreboard.h"
#include "game/utils/formatting.h"
#include "game/utils/settings.h"
#include "resources/scores.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "video/surface.h"
#include "video/vga_state.h"

#define MAX_PAGES (NUMBER_OF_ROUND_TYPES - 1)
#define TEXT_COLOR_HEADER TEXT_BLINKY_GREEN
#define TEXT_COLOR_SCORES 0x7F
#define CURSOR_STR "\x7f"

typedef struct scoreboard_local_t {
    scoreboard data;
    score_entry pending_data;
    int has_pending_data;
    int page;
    component *ti;
    guiframe *frame;
} scoreboard_local;

void scoreboard_free(scene *scene) {
    scoreboard_local *local = scene_get_userdata(scene);
    omf_free(local);
    scene_set_userdata(scene, local);
}

void handle_scoreboard_save(scoreboard_local *local) {
    int slot = 0;

    const char *name = textinput_value(local->ti);
    if(!strlen(name)) {
        return;
    }
    strncpy(local->pending_data.name, name, sizeof(local->pending_data.name));
    for(int i = 0; i < 20; i++) {
        unsigned int ex_score = local->data.entries[local->page][i].score;
        unsigned int my_score = local->pending_data.score;
        if(ex_score < my_score) {
            slot = i;
            break;
        }
    }

    // Move next entries forward by one slot
    memmove(&local->data.entries[local->page][slot + 1], &local->data.entries[local->page][slot],
            sizeof(score_entry) * (20 - slot - 1));

    // Copy new entry to the right spot
    memcpy(&local->data.entries[local->page][slot], &local->pending_data, sizeof(score_entry));

    // Write to file
    scores_write(&local->data);
}

int scoreboard_event(scene *scene, SDL_Event *event) {
    scoreboard_local *local = scene_get_userdata(scene);

    // If we are in writing mode, try handling text input
    if(local->has_pending_data) {
        return guiframe_event(local->frame, event);
    }
    return 1;
}

void scoreboard_tick(scene *scene, int paused) {
    scoreboard_local *local = scene_get_userdata(scene);
    if(local->has_pending_data) {
        guiframe_tick(local->frame);
    }
}

void scoreboard_input_tick(scene *scene) {
    scoreboard_local *local = scene_get_userdata(scene);
    game_player *player1 = game_state_get_player(scene->gs, 0);
    ctrl_event *p1 = NULL, *i;
    controller_poll(player1->ctrl, &p1);
    i = p1;
    if(i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                // If there is pending data, and name has been given, save
                if(local->has_pending_data && (i->event_data.action == ACT_KICK || i->event_data.action == ACT_PUNCH)) {

                    handle_scoreboard_save(local);
                    local->has_pending_data = 0;

                    // Normal exit routine
                    // Only allow if there is no pending data.
                } else if(!local->has_pending_data &&
                          (i->event_data.action == ACT_ESC || i->event_data.action == ACT_KICK ||
                           i->event_data.action == ACT_PUNCH)) {

                    game_state_set_next(scene->gs, scene->gs->next_next_id);

                    // If left or right button is pressed, change page
                    // but only if we are not in input mode.
                } else if(!local->has_pending_data && i->event_data.action == ACT_LEFT) {
                    local->page = (local->page > 0) ? local->page - 1 : 0;
                } else if(!local->has_pending_data && i->event_data.action == ACT_RIGHT) {
                    local->page = (local->page < MAX_PAGES) ? local->page + 1 : MAX_PAGES;
                } else if(local->has_pending_data) {
                    guiframe_action(local->frame, i->event_data.action);
                }
            }
        } while((i = i->next));
    }
    controller_free_chain(p1);
}

void scoreboard_render_overlay(scene *scene) {
    scoreboard_local *local = scene_get_userdata(scene);
    char row[128];
    char score_text[15];
    const char *score_row_format = "%-18s%-9s%-9s%11s";

    text_settings big_text;
    text_defaults(&big_text);
    big_text.font = FONT_BIG;

    // Header text
    snprintf(row, sizeof(row), "SCOREBOARD - %s", round_get_name(local->page));
    int title_x = 62 + (local->page == 0 ? 8 : 0);
    text_render(&big_text, TEXT_DEFAULT, title_x, 5, 200, 6, row);

    text_settings small_text;
    text_defaults(&small_text);
    small_text.font = FONT_SMALL;

    // Column names
    snprintf(row, sizeof(row), score_row_format, "PLAYER NAME", "ROBOT", "PILOT", "SCORE");
    text_render(&small_text, TEXT_DEFAULT, 20, 20, 290, 6, row);

    // Scores information
    unsigned int score, har_id, pilot_id;
    char *player_name;
    int entry = 0;
    int found_slot = 0;
    for(int r = 0; r < 20; r++) {
        score = local->data.entries[local->page][entry].score;
        row[0] = 0;

        // If this slot is the slot where the new, pending score data should be written,
        // show pending data and text input field. Otherwise just show next line of
        // original saved score data.
        if(local->has_pending_data && score < local->pending_data.score && !found_slot) {
            found_slot = 1;
            score_format(local->pending_data.score, score_text, sizeof(score_text));
            snprintf(row, sizeof(row), score_row_format, "", har_get_name(local->pending_data.har_id),
                     pilot_get_name(local->pending_data.pilot_id), score_text);
            guiframe_render(local->frame);
        } else {
            har_id = local->data.entries[local->page][entry].har_id;
            pilot_id = local->data.entries[local->page][entry].pilot_id;
            player_name = local->data.entries[local->page][entry].name;
            if(score > 0) {
                score_format(score, score_text, sizeof(score_text));
                snprintf(row, sizeof(row), score_row_format, player_name, har_get_name(har_id),
                         pilot_get_name(pilot_id), score_text);
            }
            entry++;
        }
        text_render(&small_text, TEXT_DEFAULT, 20, 30 + r * 8, 290, 6, row);
    }
}

// Attempts to check for any score that is pending from a finished single player game
int found_pending_score(scene *scene) {
    if(game_state_get_player(scene->gs, 1)->ctrl != NULL &&
       game_state_get_player(scene->gs, 1)->ctrl->type == CTRL_TYPE_AI &&
       game_state_get_player(scene->gs, 0)->score.score > 0) {
        return 1;
    }
    return 0;
}

// Check if score is high enough to be put on high score list
int score_fits_scoreboard(scoreboard_local *local, unsigned int score) {
    for(int i = 0; i < 20; i++) {
        if(score > local->data.entries[local->page][i].score) {
            return 1;
        }
    }
    return 0;
}

int scoreboard_create(scene *scene) {
    // Init local data
    scoreboard_local *local = omf_calloc(1, sizeof(scoreboard_local));
    local->page = settings_get()->gameplay.rounds;

    // Load scores
    if(scores_read(&local->data) == 1) {
        scores_clear(&local->data);
        DEBUG("No score data found; using empty score array.");
    }

    // Check for pending score
    local->has_pending_data = 0;
    if(found_pending_score(scene)) {
        game_player *player = game_state_get_player(scene->gs, 0);
        unsigned int score = player->score.score;
        if(score_fits_scoreboard(local, score)) {
            local->has_pending_data = 1;
            local->pending_data.score = score;
            local->pending_data.har_id = player->pilot->har_id;
            local->pending_data.pilot_id = player->pilot->pilot_id;
            local->pending_data.name[0] = 0;
        }

        // Wipe old score data, whether it was written on scoreboard or not.
        chr_score_reset(game_player_get_score(player), 1);
    }

    // Darken the colors for the background a bit.
    vga_state_mul_base_palette(0, 0xEF, 0.6);

    if(local->has_pending_data) {
        text_settings small_text;
        text_defaults(&small_text);
        small_text.font = FONT_SMALL;

        int found_slot = 0;
        unsigned int score;
        int entry = 0;
        for(int r = 0; r < 20 && !found_slot; r++) {
            score = local->data.entries[local->page][entry].score;
            if(local->has_pending_data && score < local->pending_data.score && !found_slot) {
                found_slot = 1;
                local->frame = guiframe_create(20, 30 + r * 8, 20, 10);
                local->ti = textinput_create(&small_text, 16, "", "");
                textinput_enable_background(local->ti, 0);
                guiframe_set_root(local->frame, local->ti);
                guiframe_layout(local->frame);
                component_select(local->ti, 1);
            } else {
                entry++;
            }
        }
    }

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
