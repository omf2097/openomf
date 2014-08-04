#include <stdio.h>
#include <stdlib.h>

#include "video/video.h"
#include "video/surface.h"
#include "resources/ids.h"
#include "resources/scores.h"
#include "utils/log.h"
#include "game/common_defines.h"
#include "game/text/text.h"
#include "game/utils/formatting.h"
#include "game/utils/settings.h"
#include "game/scenes/scoreboard.h"

#define MAX_PAGES (NUMBER_OF_ROUND_TYPES-1)
#define TEXT_COLOR_HEADER color_create(80, 220, 80, 0xFF)
#define TEXT_COLOR_SCORES color_create(0xFF, 0xFF, 0xFF, 0xFF)
#define CURSOR_STR "\x7f"

typedef struct scoreboard_local_t {
    surface black_surface;
    scoreboard data;
    score_entry pending_data;
    int has_pending_data;
    int page;
} scoreboard_local;

void scoreboard_free(scene *scene) {
    scoreboard_local *local = scene_get_userdata(scene);
    surface_free(&local->black_surface);
    free(local);
}

void handle_scoreboard_save(scoreboard_local *local) {
    int slot = 0;
    for(int i = 0; i < 20; i++) {
        unsigned int ex_score = local->data.entries[local->page][i].score;
        unsigned int my_score = local->pending_data.score;
        if(ex_score < my_score) {
            slot = i;
            break;
        }
    }

    // Move next entries forward by one slot
    memmove(
        &local->data.entries[local->page][slot+1],
        &local->data.entries[local->page][slot],
        sizeof(score_entry) * (20 - slot - 1));

    // Copy new entry to the right spot
    memcpy(
        &local->data.entries[local->page][slot],
        &local->pending_data,
        sizeof(score_entry));

    // Write to file
    scores_write(&local->data);
}

int scoreboard_event(scene *scene, SDL_Event *event) {
    scoreboard_local *local = scene_get_userdata(scene);

    // If we are in writing mode, try handling text input
    if(local->has_pending_data && event->type == SDL_KEYDOWN) {
        unsigned char code = event->key.keysym.sym;
        unsigned char len = strlen(local->pending_data.name);
        unsigned char scancode = event->key.keysym.scancode;
        if(scancode == SDL_SCANCODE_BACKSPACE || scancode == SDL_SCANCODE_DELETE) {
            if(len > 0) {
                local->pending_data.name[len-1] = 0;
            }
            return 1;
        } else if(code >= 32 && code <= 126) {
            if(len < sizeof(local->pending_data.name)-1) {
                local->pending_data.name[len+1] = 0;
                local->pending_data.name[len] = code;
            }
            return 1;
        }
    }
    return 1;
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
                if(local->has_pending_data
                        && strlen(local->pending_data.name) > 0
                        && (i->event_data.action == ACT_KICK || i->event_data.action == ACT_PUNCH)) {

                    handle_scoreboard_save(local);
                    local->has_pending_data = 0;

                // If there is no data, and confirm is clicked, don't save
                } else if (local->has_pending_data == 1
                        && strlen(local->pending_data.name) == 0
                        && (i->event_data.action == ACT_KICK || i->event_data.action == ACT_PUNCH)) {

                    local->has_pending_data = 0;

                // Normal exit routine
                // Only allow if there is no pending data.
                } else if(!local->has_pending_data && 
                    (i->event_data.action == ACT_ESC ||
                     i->event_data.action == ACT_KICK ||
                     i->event_data.action == ACT_PUNCH)) {

                    game_state_set_next(scene->gs, scene->gs->next_next_id);

                // If left or right button is pressed, change page
                // but only if we are not in input mode.
                } else if(!local->has_pending_data && i->event_data.action == ACT_LEFT) {
                    local->page = (local->page > 0) ? local->page-1 : 0;
                } else if(!local->has_pending_data && i->event_data.action == ACT_RIGHT) {
                    local->page = (local->page < MAX_PAGES) ? local->page+1 : MAX_PAGES;
                }
            } 
        } while((i = i->next));
    }
    controller_free_chain(p1);
}

void scoreboard_render_overlay(scene *scene) {
    scoreboard_local *local = scene_get_userdata(scene);
    video_render_sprite_size(&local->black_surface, 0, 0, 320, 200);
    char row[128];
    char score_text[15];
    char temp_name[17];
    const char* score_row_format = "%-18s%-9s%-9s%11s";

    // Header text
    sprintf(row, "SCOREBOARD - %s", round_get_name(local->page));
    int title_x = 62 + (local->page == 0 ? 8 : 0);
    font_render(&font_large, row, title_x, 5, TEXT_COLOR_HEADER);

    // Column names
    sprintf(row, score_row_format, "PLAYER NAME", "ROBOT", "PILOT", "SCORE");
    font_render(&font_small, row, 20, 20, TEXT_COLOR_HEADER);

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
            sprintf(temp_name, "%s%s", local->pending_data.name, CURSOR_STR);
            score_format(local->pending_data.score, score_text);
            sprintf(row,
                score_row_format,
                temp_name,
                har_get_name(local->pending_data.har_id),
                pilot_get_name(local->pending_data.pilot_id),
                score_text);
            found_slot = 1;
        } else {
            har_id = local->data.entries[local->page][entry].har_id;
            pilot_id = local->data.entries[local->page][entry].pilot_id;
            player_name = local->data.entries[local->page][entry].name;
            if(score > 0) {
                score_format(score, score_text);
                sprintf(row,
                    score_row_format,
                    player_name,
                    har_get_name(har_id),
                    pilot_get_name(pilot_id),
                    score_text);
            }
            entry++;
        }
        font_render(&font_small, row, 20, 30 + r*8, TEXT_COLOR_SCORES);
    }
}

// Attempts to check for any score that is pending from a finished single player game
int found_pending_score(scene *scene) {
    if(game_state_get_player(scene->gs, 1)->ctrl != NULL
        && game_state_get_player(scene->gs, 1)->ctrl->type == CTRL_TYPE_AI
        && game_state_get_player(scene->gs, 0)->score.score > 0) {
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
    scoreboard_local *local = malloc(sizeof(scoreboard_local));
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
            local->pending_data.har_id = player->har_id;
            local->pending_data.pilot_id = player->pilot_id;
            local->pending_data.name[0] = 0;
        }

        // Wipe old score data, whether it was written on scoreboard or not.
        chr_score_reset(game_player_get_score(player), 1);
    }

    // Create a surface that has an appropriate alpha for darkening the screen a bit
    surface_create(&local->black_surface, SURFACE_TYPE_RGBA, 32, 32);
    surface_fill(&local->black_surface, color_create(0,0,0,200));

    // Set callbacks
    scene_set_userdata(scene, local);
    scene_set_event_cb(scene, scoreboard_event);
    scene_set_input_poll_cb(scene, scoreboard_input_tick);
    scene_set_render_overlay_cb(scene, scoreboard_render_overlay);
    scene_set_free_cb(scene, scoreboard_free);
    video_select_renderer(VIDEO_RENDERER_HW);

    // All done
    return 0;
}
