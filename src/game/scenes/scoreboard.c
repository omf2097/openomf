#include <stdio.h>

#include "video/video.h"
#include "video/surface.h"
#include "resources/ids.h"
#include "resources/scores.h"
#include "utils/log.h"
#include "game/common_defines.h"
#include "game/text/text.h"
#include "game/utils/formatting.h"
#include "game/scenes/scoreboard.h"

#define MAX_PAGES (NUMBER_OF_ROUND_TYPES-1)
#define TEXT_COLOR_HEADER color_create(80, 220, 80, 0xFF)
#define TEXT_COLOR_SCORES color_create(0xFF, 0xFF, 0xFF, 0xFF)

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

int scoreboard_event(scene *scene, SDL_Event *event) {
    scoreboard_local *local = scene_get_userdata(scene);
    game_player *player1 = game_state_get_player(scene->gs, 0);
    ctrl_event *p1 = NULL, *i;
    controller_event(player1->ctrl, event, &p1);
    i = p1;
    if(i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                if(i->event_data.action == ACT_ESC ||
                    i->event_data.action == ACT_KICK ||
                    i->event_data.action == ACT_PUNCH) {

                    game_state_set_next(scene->gs, SCENE_MENU);
                } else if(i->event_data.action == ACT_LEFT) {
                    local->page = (local->page > 0) ? local->page-1 : 0;
                } else if(i->event_data.action == ACT_RIGHT) {
                    local->page = (local->page < MAX_PAGES) ? local->page+1 : MAX_PAGES;
                }
            } 
        } while((i = i->next));
    }
    controller_free_chain(p1);
    return 1;
}

void scoreboard_render_overlay(scene *scene) {
    scoreboard_local *local = scene_get_userdata(scene);
    video_render_sprite_size(&local->black_surface, 0, 0, 320, 200);
    char row[128];
    char score_text[15];

    // Header text
    sprintf(row, "SCOREBOARD - %s", round_types[local->page]);
    int title_x = 62 + (local->page == 0 ? 8 : 0);
    font_render(&font_large, row, title_x, 5, TEXT_COLOR_HEADER);

    // Column names
    sprintf(row, "%-18s%-9s%-9s%11s", "PLAYER NAME", "ROBOT", "PILOT", "SCORE");
    font_render(&font_small, row, 20, 20, TEXT_COLOR_HEADER);

    // Scores information
    unsigned int score, har_id, pilot_id;
    char *player_name;
    for(int r = 0; r < 20; r++) {
        score = local->data.entries[local->page][r].score;
        har_id = local->data.entries[local->page][r].har_id;
        pilot_id = local->data.entries[local->page][r].pilot_id;
        player_name = local->data.entries[local->page][r].name;
        if(score > 0) {
            score_format(score, score_text);
            sprintf(row, "%-18s%-9s%-9s%11s",
                player_name,
                har_names[har_id],
                pilot_names[pilot_id],
                score_text);
            font_render(&font_small, row, 20, 30 + r*8, TEXT_COLOR_SCORES);
        }
    }
}

int found_pending_score(scene *scene) {
    if(game_state_get_player(scene->gs, 1)->ctrl != NULL
        && game_state_get_player(scene->gs, 1)->ctrl->type == CTRL_TYPE_AI
        && game_state_get_player(scene->gs, 1)->score.score > 0) {
        return 1;
    }
    return 0;
}

int scoreboard_create(scene *scene) {
    // Init local data
    scoreboard_local *local = malloc(sizeof(scoreboard_local));
    local->page = 0;

    // Load scores
    if(scores_read(&local->data) == 1) {
        scores_clear(&local->data);
        DEBUG("No score data found; using empty score array.");
    }

    // Check for pending score
    if(found_pending_score(scene)) {
        local->has_pending_data = 1;
        local->pending_data.score = game_state_get_player(scene->gs, 1)->score.score;
        local->pending_data.har_id = 0; // TODO: FIX
        local->pending_data.pilot_id = 0; // TODO: FIX
        local->pending_data.name[0] = 0;
    }

    // Create a surface that has an appropriate alpha for darkening the screen a bit
    surface_create(&local->black_surface, SURFACE_TYPE_RGBA, 32, 32);
    surface_fill(&local->black_surface, color_create(0,0,0,200));

    // Set callbacks
    scene_set_userdata(scene, local);
    scene_set_event_cb(scene, scoreboard_event);
    scene_set_render_overlay_cb(scene, scoreboard_render_overlay);
    scene_set_free_cb(scene, scoreboard_free);
    video_select_renderer(VIDEO_RENDERER_HW);

    // All done
    return 0;
}
