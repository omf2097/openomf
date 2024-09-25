#include "game/protos/scene.h"
#include "utils/allocator.h"
#include "video/video.h"

#define TEXT_BRIGHT color_create(240, 250, 250, 255)
#define TEXT_COLOR color_create(186, 250, 250, 255)
#define BLUE_TEXT_COLOR color_create(60, 182, 255, 255)

#define TEXT_BLACK color_create(0, 0, 0, 255)

enum
{
    LOBBY_CHALLENGE,
    LOBBY_WHISPER,
    LOBBY_YELL,
    LOBBY_REFRESH,
    LOBBY_EXIT,
    LOBBY_ACTION_COUNT
};

typedef struct lobby_local_t {
    char name[40];
    char msg[128];
    list log;
    bool named;
    uint8_t mode;
} lobby_local;

static int lobby_event(scene *scene, SDL_Event *e) {
    lobby_local *local = scene_get_userdata(scene);
    if(!local->named) {
        // Handle selection
        if(e->type == SDL_TEXTINPUT) {
            strncat(local->name, e->text.text, sizeof(local->name) - strlen(local->name));
            return 0;
        } else if(e->type == SDL_KEYDOWN) {
            size_t len = strlen(local->name);
            const unsigned char *state = SDL_GetKeyboardState(NULL);
            if(state[SDL_SCANCODE_BACKSPACE] || state[SDL_SCANCODE_DELETE]) {
                if(len > 0) {
                    local->name[len - 1] = '\0';
                }
            } else if(state[SDL_SCANCODE_LEFT]) {
                // TODO move cursor to the left
            } else if(state[SDL_SCANCODE_RIGHT]) {
                // TODO move cursor to the right
            } else if(state[SDL_SCANCODE_V] && state[SDL_SCANCODE_LCTRL]) {
                if(SDL_HasClipboardText()) {
                    strncat(local->name, SDL_GetClipboardText(), sizeof(local->name) - strlen(local->name));
                }
            } else if(state[SDL_SCANCODE_RETURN] && strlen(local->name) > 0) {
                local->named = true;
            } else if(state[SDL_SCANCODE_ESCAPE]) {
                game_state_set_next(scene->gs, SCENE_MENU);
            }
            return 0;
        }
    } else {
        if(e->type == SDL_TEXTINPUT && (local->mode == LOBBY_WHISPER || local->mode == LOBBY_YELL)) {
            strncat(local->msg, e->text.text, sizeof(local->msg) - strlen(local->msg));
            return 0;
        } else if(e->type == SDL_KEYDOWN) {
            const unsigned char *state = SDL_GetKeyboardState(NULL);
            if((state[SDL_SCANCODE_BACKSPACE] || state[SDL_SCANCODE_DELETE]) &&
               (local->mode == LOBBY_WHISPER || local->mode == LOBBY_YELL)) {
                size_t len = strlen(local->msg);
                if(len > 0) {
                    local->msg[len - 1] = '\0';
                }
            } else if(state[SDL_SCANCODE_RIGHT]) {
                local->mode++;
                if(local->mode >= LOBBY_ACTION_COUNT) {
                    local->mode = LOBBY_CHALLENGE;
                }
            } else if(state[SDL_SCANCODE_LEFT]) {
                local->mode--;
                if(local->mode >= LOBBY_ACTION_COUNT) {
                    local->mode = LOBBY_EXIT;
                }
            }
        }
    }

    return 1;
}

void lobby_input_tick(scene *scene) {
    game_player *player1 = game_state_get_player(scene->gs, 0);
    ctrl_event *p1 = NULL, *i;
    controller_poll(player1->ctrl, &p1);

    i = p1;
    if(i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                if(i->event_data.action == ACT_ESC) {
                    game_state_set_next(scene->gs, SCENE_MENU);
                }
            }
        } while((i = i->next));
    }
    controller_free_chain(p1);
}

void lobby_render_overlay(scene *scene) {
    lobby_local *local = scene_get_userdata(scene);

    char buf[100];

    snprintf(buf, sizeof(buf), "Player");
    font_render(&font_net1, buf, 16, 7, TEXT_COLOR);

    snprintf(buf, sizeof(buf), "Action");
    font_render(&font_net1, buf, 117, 7, TEXT_COLOR);

    snprintf(buf, sizeof(buf), "Wn/Loss");
    font_render(&font_net2, buf, 200, 8, TEXT_COLOR);

    snprintf(buf, sizeof(buf), "Version");
    font_render(&font_net2, buf, 240, 8, TEXT_COLOR);

    snprintf(buf, sizeof(buf), "1 of 0");
    font_render(&font_net2, buf, 284, 8, TEXT_COLOR);

    if(!local->named) {
        snprintf(buf, sizeof(buf), "Enter your name:");
        font_render_shadowed_colored(&font_net1, buf, 9, 140, BLUE_TEXT_COLOR, TEXT_SHADOW_RIGHT | TEXT_SHADOW_BOTTOM,
                                     TEXT_BLACK);
        snprintf(buf, sizeof(buf), "%s%c", local->name, 127);
        font_render_shadowed(&font_net1, buf, 130, 140, TEXT_COLOR, TEXT_SHADOW_RIGHT | TEXT_SHADOW_BOTTOM);
    } else {
        color c = (local->mode == LOBBY_CHALLENGE && scene->gs->int_tick % 2 == 0) ? TEXT_BRIGHT : BLUE_TEXT_COLOR;
        snprintf(buf, sizeof(buf), "Challenge");
        font_render_shadowed_colored(&font_net1, buf, 9, 140, c, TEXT_SHADOW_RIGHT | TEXT_SHADOW_BOTTOM, TEXT_BLACK);
        c = (local->mode == LOBBY_WHISPER && scene->gs->int_tick % 2 == 0) ? TEXT_BRIGHT : BLUE_TEXT_COLOR;
        snprintf(buf, sizeof(buf), "Whisper");
        font_render_shadowed_colored(&font_net1, buf, 85, 140, c, TEXT_SHADOW_RIGHT | TEXT_SHADOW_BOTTOM, TEXT_BLACK);
        c = (local->mode == LOBBY_YELL && scene->gs->int_tick % 2 == 0) ? TEXT_BRIGHT : BLUE_TEXT_COLOR;
        snprintf(buf, sizeof(buf), "Yell");
        font_render_shadowed_colored(&font_net1, buf, 143, 140, c, TEXT_SHADOW_RIGHT | TEXT_SHADOW_BOTTOM, TEXT_BLACK);
        c = (local->mode == LOBBY_REFRESH && scene->gs->int_tick % 2 == 0) ? TEXT_BRIGHT : BLUE_TEXT_COLOR;
        snprintf(buf, sizeof(buf), "Refresh");
        font_render_shadowed_colored(&font_net1, buf, 178, 140, c, TEXT_SHADOW_RIGHT | TEXT_SHADOW_BOTTOM, TEXT_BLACK);

        c = (local->mode == LOBBY_EXIT && scene->gs->int_tick % 2 == 0) ? TEXT_BRIGHT : BLUE_TEXT_COLOR;
        snprintf(buf, sizeof(buf), "Exit");
        font_render_shadowed_colored(&font_net1, buf, 240, 140, c, TEXT_SHADOW_RIGHT | TEXT_SHADOW_BOTTOM, TEXT_BLACK);
    }
}

int lobby_create(scene *scene) {

    lobby_local *local;

    // Load up settings
    // setting = settings_get();

    fight_stats *fight_stats = &scene->gs->fight_stats;
    memset(fight_stats, 0, sizeof(*fight_stats));

    // Initialize local struct
    local = omf_calloc(1, sizeof(lobby_local));
    scene_set_userdata(scene, local);

    local->name[0] = 0;
    local->msg[0] = 0;
    local->named = false;
    local->mode = LOBBY_CHALLENGE;

    // scene_set_input_poll_cb(scene, lobby_input_tick);

    scene_set_render_overlay_cb(scene, lobby_render_overlay);
    scene_set_event_cb(scene, lobby_event);

    video_render_bg_separately(false);
    return 0;
}
