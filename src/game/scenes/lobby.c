#include "game/gui/frame.h"
#include "game/protos/scene.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "video/video.h"

#include "game/gui/gui.h"

// FIXME: No idea what these should be
#define TEXT_COLOR 0xFE
#define BLUE_TEXT_COLOR 0xFE
#define TEXT_BLACK 1

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
    return guiframe_event(local->frame, e);
}

void lobby_input_tick(scene *scene) {
    lobby_local *local = scene_get_userdata(scene);
    game_player *player1 = game_state_get_player(scene->gs, 0);
    ctrl_event *p1 = NULL, *i;
    controller_poll(player1->ctrl, &p1);

    i = p1;
    if(i) {
        do {
            /*if(i->type == EVENT_TYPE_ACTION) {
                if(i->event_data.action == ACT_ESC) {
                    game_state_set_next(scene->gs, SCENE_MENU);
                }
            }*/

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

    guiframe_render(local->frame);
}

void lobby_challenge(component *c, void *userdata) {
}

void lobby_do_yell(component *c, void *userdata) {
    // menu *m = sizer_get_obj(c->parent);
    DEBUG("yelled %s", textinput_value(c));
    textinput_clear(c);
    // TODO get the message and send/log it from the textinput component 'c'
}

component *lobby_yell_create(scene *s) {
    // Text config
    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_NET1;
    tconf.halign = TEXT_LEFT;
    tconf.cforeground = 6;
    tconf.cselected = 5;
    tconf.cdisabled = 4;
    tconf.cinactive = 3;

    text_settings help_text;
    text_defaults(&help_text);
    help_text.font = FONT_NET2;
    help_text.halign = TEXT_LEFT;
    help_text.cforeground = 56;

    component *menu = menu_create(11);

    menu_set_help_pos(menu, 10, 155, 500, 10);
    menu_set_help_text_settings(menu, &help_text);
    menu_set_horizontal(menu, true);
    menu_set_background(menu, false);
    menu_attach(menu, label_create(&tconf, "Yell:"));
    component *yell_input =
        textinput_create(&tconf, "Yell:", "Yell a message to everybody in the challenge arena.", "");
    menu_attach(menu, yell_input);
    textinput_enable_background(yell_input, 0);
    textinput_set_done_cb(yell_input, lobby_do_yell, s);

    return menu;
}

void lobby_do_whisper(component *c, void *userdata) {
    menu *m = sizer_get_obj(c->parent);
    DEBUG("whispered %s", textinput_value(c));
    // TODO get the message and send/log it from the textinput component 'c'
    m->finished = 1;
}

component *lobby_whisper_create(scene *s) {
    // Text config
    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_NET1;
    tconf.halign = TEXT_LEFT;
    tconf.cforeground = 6;
    tconf.cselected = 5;
    tconf.cdisabled = 4;
    tconf.cinactive = 3;

    text_settings help_text;
    text_defaults(&help_text);
    help_text.font = FONT_NET2;
    help_text.halign = TEXT_LEFT;
    help_text.cforeground = 56;

    component *menu = menu_create(11);

    menu_set_help_pos(menu, 10, 155, 500, 10);
    menu_set_help_text_settings(menu, &help_text);
    menu_set_horizontal(menu, true);
    menu_set_background(menu, false);
    menu_attach(menu, label_create(&tconf, "Whisper:"));
    component *whisper_input =
        textinput_create(&tconf, "Whisper:", "Whisper a message to %s. Press enter when done, esc to abort.", "");
    menu_attach(menu, whisper_input);
    textinput_enable_background(whisper_input, 0);
    textinput_set_done_cb(whisper_input, lobby_do_whisper, s);

    return menu;
}

void lobby_whisper(component *c, void *userdata) {
    scene *s = userdata;
    menu_set_submenu(c->parent, lobby_whisper_create(s));
}

void lobby_yell(component *c, void *userdata) {
    scene *s = userdata;
    menu_set_submenu(c->parent, lobby_yell_create(s));
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

void lobby_entered_name(component *c, void *userdata) {
    scene *scene = userdata;
    char buf[128];
    lobby_local *local = scene_get_userdata(scene);
    strncpy(local->name, textinput_value(c), sizeof(local->name));
    snprintf(buf, sizeof(buf), "%s has entered the Arena", local->name);
    list_append(&local->log, buf, strlen(buf) + 1);
    menu *m = sizer_get_obj(c->parent);
    m->finished = 1;
}

component *lobby_exit_create(scene *s) {
    // Text config
    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_NET1;
    tconf.halign = TEXT_LEFT;
    tconf.cforeground = 6;
    tconf.cselected = 5;
    tconf.cdisabled = 4;
    tconf.cinactive = 3;

    component *menu = menu_create(11);
    menu_set_horizontal(menu, true);
    menu_set_background(menu, false);
    menu_attach(menu, label_create(&tconf, "Exit the Challenge Arena?"));
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
    local->mode = LOBBY_CHALLENGE;
    list_create(&local->log);

    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_NET1;
    tconf.halign = TEXT_LEFT;
    tconf.cforeground = 6;
    tconf.cselected = 5;
    tconf.cdisabled = 4;
    tconf.cinactive = 3;

    component *menu = menu_create(11);
    menu_set_horizontal(menu, true);
    menu_set_background(menu, false);

    menu_set_help_pos(menu, 10, 155, 500, 10);
    text_settings help_text;
    text_defaults(&help_text);
    help_text.font = FONT_NET2;
    help_text.halign = TEXT_LEFT;
    help_text.cforeground = 56;

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

    component *name_menu = menu_create(11);
    menu_set_horizontal(name_menu, true);
    menu_set_background(name_menu, false);
    menu_attach(name_menu, label_create(&tconf, "Enter your name:"));
    // TODO pull the last used name from settings
    component *name_input = textinput_create(&tconf, "", "", "");
    textinput_enable_background(name_input, 0);
    textinput_set_done_cb(name_input, lobby_entered_name, scene);
    menu_attach(name_menu, name_input);

    local->frame = guiframe_create(9, 132, 300, 12);
    guiframe_set_root(local->frame, menu);
    guiframe_layout(local->frame);

    menu_set_submenu(menu, name_menu);

    scene_set_input_poll_cb(scene, lobby_input_tick);

    scene_set_dynamic_tick_cb(scene, lobby_tick);

    scene_set_render_overlay_cb(scene, lobby_render_overlay);
    scene_set_event_cb(scene, lobby_event);

    return 0;
}
