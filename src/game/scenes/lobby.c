#include "game/gui/frame.h"
#include "game/protos/scene.h"
#include "utils/allocator.h"
#include "video/video.h"

#include "game/gui/gui.h"

// FIXME: No idea what these should be
#define TEXT_COLOR 0xFE
#define BLUE_TEXT_COLOR 0xFE


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

    guiframe *frame;
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
                char buf[128];
                snprintf(buf, sizeof(buf), "%s has entered the Arena", local->name);
                list_append(&local->log, buf, strlen(buf) + 1);
                local->named = true;
            } else if(state[SDL_SCANCODE_ESCAPE]) {
                game_state_set_next(scene->gs, SCENE_MENU);
            }
            return 0;
        }
    } else {
        return guiframe_event(local->frame, e);
        /*if(e->type == SDL_TEXTINPUT && (local->mode == LOBBY_WHISPER || local->mode == LOBBY_YELL)) {
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
        }*/
    }

    return 1;
}

void lobby_input_tick(scene *scene) {
    lobby_local *local = scene_get_userdata(scene);
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

            guiframe_action(local->frame, p1->event_data.action);
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
        guiframe_render(local->frame);
    }
}

void lobby_challenge(component *c, void *userdata) {
}

void lobby_whisper(component *c, void *userdata) {
}

void lobby_yell(component *c, void *userdata) {
}

void lobby_refresh(component *c, void *userdata) {
}

void lobby_do_exit(component *c, void *userdata) {
    scene *scene = userdata;
    game_state_set_next(scene->gs, SCENE_MENU);
}

void lobby_refuse_exit(component *c, void *userdata) {
    menu *m = sizer_get_obj(c->parent);
    m->finished = 1;
}

component *lobby_exit_create(scene *s) {
    // Text config
    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_NET1;
    tconf.halign = TEXT_LEFT;
    tconf.cforeground = COLOR_DARK_GREEN;

    component *menu = menu_create(11);
    menu_set_horizontal(menu, true);
    menu_set_background(menu, false);
    menu_attach(menu, textbutton_create(&tconf, "Exit the Challenge Arena?", NULL, COM_DISABLED, NULL, NULL));
    menu_attach(menu, textbutton_create(&tconf, "Yes", NULL, COM_ENABLED, lobby_do_exit, s));
    menu_attach(menu, textbutton_create(&tconf, "No", NULL, COM_ENABLED, lobby_refuse_exit, NULL));

    return menu;
}

void lobby_exit(component *c, void *userdata) {
    scene *s = userdata;
    menu_set_submenu(c->parent, lobby_exit_create(s));
}

void lobby_tick(scene *scene, int paused) {
    lobby_local *local = scene_get_userdata(scene);
    guiframe_tick(local->frame);
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
    list_create(&local->log);

    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_NET1;
    tconf.halign = TEXT_LEFT;
    tconf.cforeground = BLUE_TEXT_COLOR;

    component *menu = menu_create(11);
    menu_set_horizontal(menu, true);
    menu_set_background(menu, false);

    menu_set_help_pos(menu, 10, 155, 500, 10);
    text_settings help_text;
    text_defaults(&help_text);
    help_text.font = FONT_NET2;
    help_text.halign = TEXT_LEFT;
    menu_set_help_text_settings(menu, &help_text);
    menu_attach(menu, textbutton_create(&tconf, "Challenge",
                                        "Challenge this player to a fight. Challenge yourself for 1-player game.",
                                        COM_ENABLED, lobby_challenge, scene));
    menu_attach(menu, textbutton_create(&tconf, "Whisper", "Whisper a message to this player.", COM_ENABLED,
                                        lobby_whisper, scene));
    menu_attach(menu,
                textbutton_create(&tconf, "Yell", "Chat with everybody in the arena.", COM_ENABLED, lobby_yell, scene));
    menu_attach(menu,
                textbutton_create(&tconf, "Refresh", "Refresh the player list.", COM_ENABLED, lobby_refresh, scene));
    menu_attach(menu, textbutton_create(&tconf, "Exit", "Exit and disconnect.", COM_ENABLED, lobby_exit, scene));

    local->frame = guiframe_create(9, 132, 300, 12);
    guiframe_set_root(local->frame, menu);
    guiframe_layout(local->frame);

    scene_set_input_poll_cb(scene, lobby_input_tick);

    scene_set_dynamic_tick_cb(scene, lobby_tick);

    scene_set_render_overlay_cb(scene, lobby_render_overlay);
    scene_set_event_cb(scene, lobby_event);

    return 0;
}
