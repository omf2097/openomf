#include "game/gui/dialog.h"
#include "game/gui/gui_frame.h"
#include "game/protos/scene.h"
#include "game/utils/serial.h"
#include "game/utils/version.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/log.h"
#include "utils/miscmath.h"
#include "video/video.h"

#include "game/utils/nat.h"
#include "game/utils/settings.h"

#include "game/gui/gui.h"

#include <stdio.h>

// FIXME: No idea what these should be
#define TEXT_BLACK 1

#define YELL_COLOR 3
#define JOIN_COLOR 7
#define WHISPER_COLOR 6
#define LEAVE_COLOR 5
#define ANNOUNCEMENT_COLOR 48

#define VERSION_BUF_SIZE 30
// increment this when the protocol with the lobby server changes
#define PROTOCOL_VERSION 0

// GUI colors specific to palette used by lobby
#define TEXT_PRIMARY_COLOR 6
#define TEXT_SECONDARY_COLOR 6
#define TEXT_DISABLED_COLOR 4
#define TEXT_ACTIVE_COLOR 5
#define TEXT_INACTIVE_COLOR 3
#define TEXT_SHADOW_COLOR 6
#define DIALOG_BORDER_COLOR 0xFE

enum
{
    LOBBY_STARTING,
    LOBBY_CONNECTING,
    LOBBY_YELL,
    LOBBY_MAIN,
    LOBBY_CHALLENGE,
    LOBBY_WHISPER,
    LOBBY_REFRESH,
    LOBBY_EXIT,
    LOBBY_ACTION_COUNT
};

enum
{
    PACKET_JOIN = 1,
    PACKET_YELL,
    PACKET_WHISPER,
    PACKET_CHALLENGE,
    PACKET_DISCONNECT,
    PACKET_PRESENCE,
    PACKET_CONNECTED,
    PACKET_REFRESH,
    PACKET_ANNOUNCEMENT,
    PACKET_RELAY,
};

enum
{
    JOIN_SUCCESS = 0,
    JOIN_ERROR_NAME_USED,
    JOIN_ERROR_NAME_INVALID,
    JOIN_ERROR_UNSUPPORTED_PROTOCOL,
};

enum
{
    TITLE_PLAYER = 0,
    TITLE_ACTION,
    TITLE_WIN_LOSS,
    TITLE_VERSION,
    TITLE_USER_OF,
    TITLE_COUNT,
};

enum
{
    CHALLENGE_OFFER = 0,
    CHALLENGE_ACCEPT,
    CHALLENGE_REJECT,
    CHALLENGE_CANCEL,
    CHALLENGE_DONE,
    CHALLENGE_ERROR,
};

enum
{
    ROLE_CHALLENGER,
    ROLE_CHALLENGEE,
};

enum
{
    PRESENCE_UNKNOWN = 1,
    PRESENCE_STARTING,
    PRESENCE_AVAILABLE,
    PRESENCE_PRACTICING,
    PRESENCE_CHALLENGING,
    PRESENCE_PONDERING,
    PRESENCE_FIGHTING,
    PRESENCE_WATCHING,
    PRESENCE_COUNT,
};

typedef struct lobby_user {
    char name[16];
    char version[VERSION_BUF_SIZE];
    ENetAddress address;
    uint16_t port;     // port the server sees this user connecting from
    uint16_t ext_port; // port this user claims will route inbound to them (or 0)
    uint32_t id;
    uint8_t wins;
    uint8_t losses;
    uint8_t status;

    text *name_text;
    text *wins_text;
    text *version_text;
} lobby_user;

typedef struct lobby_local {
    ENetHost *client;
    ENetPeer *peer;
    ENetPeer *opponent_peer;
    // our enet connection id
    uint32_t id;
    // list of log messages (chat/join/etc)
    list log;
    // list of online users (includes ourself)
    list users;
    // what submenu we're in (STARTING/MAIN/YELL, etc)
    uint8_t mode;
    // when challening a peer, tracks how many connection attempts we've made
    // each side will make several attempts, depending on whether the 'external port' has been provided
    uint8_t connection_count;
    // the index of the currently selected user in the user list
    uint8_t active_user;
    // have we created the controllers needed to start a match
    bool controllers_created;
    // the user we are challenging, or is challenging us
    lobby_user *opponent;

    dialog *dialog;

    menu *joinmenu;

    text *presences[PRESENCE_COUNT];
    text *titles[5];

    gui_frame *frame;
    uint8_t role;
    // how many attempts we've made to get a working NAT address
    uint8_t nat_tries;
    // track if the client has failed to initialize/connect to the lobby
    bool disconnected;
    nat_ctx *nat;
    // the name of the user
    char name[16];
    // holds various helptext labels
    char helptext[80];
} lobby_local;

typedef struct log_event {
    text *message;
} log_event;

void lobby_free(scene *scene) {
    lobby_local *local = scene_get_userdata(scene);
    gui_frame_free(local->frame);

    // Free user records
    iterator it;
    lobby_user *user;
    list_iter_begin(&local->users, &it);
    foreach(it, user) {
        text_free(&user->name_text);
        text_free(&user->version_text);
        text_free(&user->wins_text);
    }
    list_free(&local->users);

    // Free chat messages
    log_event *log_msg;
    list_iter_begin(&local->log, &it);
    foreach(it, log_msg) {
        text_free(&log_msg->message);
    }
    list_free(&local->log);

    if(local->client) {
        enet_host_destroy(local->client);
    }
    if(local->dialog) {
        dialog_free(local->dialog);
        omf_free(local->dialog);
    }
    for(int i = 0; i < TITLE_COUNT; i++) {
        text_free(&local->titles[i]);
    }
    for(int i = 0; i < PRESENCE_COUNT; i++) {
        text_free(&local->presences[i]);
    }

    omf_free(local);
    scene_set_userdata(scene, local);
}

static int lobby_event(scene *scene, SDL_Event *e) {
    lobby_local *local = scene_get_userdata(scene);
    return gui_frame_event(local->frame, e);
}

void lobby_show_dialog(scene *scene, int dialog_style, char *dialog_text, dialog_clicked_cb callback) {
    lobby_local *local = scene_get_userdata(scene);
    if(local->dialog) {
        dialog_free(local->dialog);
        omf_free(local->dialog);
    }
    local->dialog = omf_calloc(1, sizeof(dialog));
    dialog_create(local->dialog, dialog_style, dialog_text, 72, 60);
    local->dialog->userdata = scene;
    local->dialog->clicked = callback;

    dialog_show(local->dialog, 1);
}

static void update_active_user_text(lobby_local *local) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%d of %d", local->active_user + 1, list_size(&local->users));
    text_set_from_c(local->titles[TITLE_USER_OF], buf);
}

void lobby_input_tick(scene *scene) {
    lobby_local *local = scene_get_userdata(scene);
    ctrl_event *p1 = NULL, *i;
    game_state_menu_poll(scene->gs, &p1);

    i = p1;
    if(i) {
        do {
            if(local->dialog && dialog_is_visible(local->dialog)) {
                dialog_event(local->dialog, p1->event_data.action);
            } else if(i->type == EVENT_TYPE_ACTION && i->event_data.action == ACT_DOWN) {
                local->active_user++;
                if(local->active_user >= list_size(&local->users)) {
                    local->active_user = 0;
                }
                update_active_user_text(local);
            } else if(i->type == EVENT_TYPE_ACTION && i->event_data.action == ACT_UP) {
                local->active_user--;
                if(local->active_user >= list_size(&local->users)) {
                    local->active_user = list_size(&local->users) - 1;
                }
                update_active_user_text(local);
            } else {
                gui_frame_action(local->frame, p1->event_data.action);
            }
        } while((i = i->next));
    }
    controller_free_chain(p1);
}

static text *create_big_text(int w, int h, const char *str) {
    text *t = text_create_with_font_and_size(FONT_NET1, w, h);
    text_set_color(t, 3);
    text_set_from_c(t, str);
    return t;
}

static text *create_small_text(int w, int h, const char *str) {
    text *t = text_create_with_font_and_size(FONT_NET2, w, h);
    text_set_color(t, 56);
    text_set_from_c(t, str);
    return t;
}

static void update_lobby_user_texts(lobby_user *user, bool create) {
    char wins[8];
    snprintf(wins, sizeof(wins), "%d/%d", user->wins, user->losses);
    if(create) {
        user->name_text = create_big_text(90, 8, user->name);
        user->wins_text = create_small_text(50, 6, wins);
        user->version_text = create_small_text(140, 6, user->version);
        text_set_color(user->name_text, 36);
        text_set_color(user->wins_text, 56);
        text_set_color(user->version_text, 56);
    } else {
        text_set_from_c(user->name_text, user->name);
        text_set_from_c(user->wins_text, wins);
        text_set_from_c(user->version_text, user->version);
    }
}

void lobby_render_overlay(scene *scene) {
    lobby_local *local = scene_get_userdata(scene);

    text_settings font_big;
    text_defaults(&font_big);
    font_big.font = FONT_NET1;
    font_big.cforeground = 3;

    text_settings font_small;
    text_defaults(&font_small);
    font_small.font = FONT_NET2;
    font_small.cforeground = 56;

    if(local->mode > LOBBY_YELL) {
        text_draw(local->titles[TITLE_PLAYER], 16, 7);
        text_draw(local->titles[TITLE_ACTION], 117, 7);
        text_draw(local->titles[TITLE_WIN_LOSS], 200, 8);
        text_draw(local->titles[TITLE_VERSION], 240, 8);
        text_draw(local->titles[TITLE_USER_OF], 284, 8);

        iterator it;
        lobby_user *user;
        list_iter_begin(&local->users, &it);
        int i = 0;
        while((user = iter_next(&it)) && i < 8) {
            text_set_color(user->name_text, (i == local->active_user) ? 7 : 36);
            text_draw(user->name_text, 16, 18 + (10 * i));

            if(user->status > 0 && user->status < PRESENCE_COUNT) {
                text_draw(local->presences[user->status], 117, 18 + (10 * i));
            } else {
                text_draw(local->presences[PRESENCE_UNKNOWN], 117, 18 + (10 * i));
            }

            text_draw(user->wins_text, 200, 18 + (10 * i));
            text_draw(user->version_text, 240, 18 + (10 * i));
            i++;
        }

        int left = 32;
        list_iter_end(&local->log, &it);
        log_event *log_msg;
        while((log_msg = iter_prev(&it)) && left > 0) {
            left -= text_get_layout_height(log_msg->message);
            text_draw(log_msg->message, 10, 168 + left);
        }
    } else if(local->mode == LOBBY_YELL) {
        iterator it;
        int left = 140;
        list_iter_end(&local->log, &it);
        log_event *log_msg;
        while((log_msg = iter_prev(&it)) && left > 0) {
            left -= text_get_layout_height(log_msg->message);
            text_draw(log_msg->message, 10, left);
        }
    }

    gui_frame_render(local->frame);

    if(local->dialog && dialog_is_visible(local->dialog)) {
        dialog_render(local->dialog);
    }
}

void lobby_dialog_cancel_challenge(dialog *dlg, dialog_result result) {
    dialog_show(dlg, 0);
    scene *s = dlg->userdata;
    lobby_local *local = scene_get_userdata(s);
    serial ser;
    serial_create(&ser);
    serial_write_int8(&ser, PACKET_CHALLENGE << 4 | CHALLENGE_CANCEL);

    ENetPacket *packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
    serial_free(&ser);
    enet_peer_send(local->peer, 0, packet);

    if(local->opponent_peer) {
        enet_peer_reset(local->opponent_peer);
    }

    local->opponent_peer = NULL;
    local->opponent = NULL;
}

void lobby_do_challenge(component *c, void *userdata) {
    scene *s = userdata;
    lobby_local *local = scene_get_userdata(s);
    lobby_user *user = list_get(&local->users, local->active_user);
    local->opponent = user;
    char buf[80];

    snprintf(buf, sizeof(buf), "Challenging %s...", user->name);
    local->role = ROLE_CHALLENGER;

    lobby_show_dialog(s, DIALOG_STYLE_CANCEL, buf, lobby_dialog_cancel_challenge);

    serial ser;
    serial_create(&ser);
    serial_write_int8(&ser, PACKET_CHALLENGE << 4);
    serial_write_int32(&ser, user->id);
    local->opponent = user;

    ENetPacket *packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
    serial_free(&ser);

    enet_peer_send(local->peer, 0, packet);
}

void lobby_cancel_challenge(component *c, void *userdata) {
    menu *m = sizer_get_obj(c->parent);
    scene *s = userdata;
    lobby_local *local = scene_get_userdata(s);
    m->finished = 1;
    local->mode = LOBBY_MAIN;
}

component *lobby_challenge_create(scene *s) {
    lobby_local *local = scene_get_userdata(s);

    component *menu = menu_create();
    menu_set_horizontal(menu, true);
    menu_set_background(menu, false);
    menu_set_padding(menu, 0);

    lobby_user *user = list_get(&local->users, local->active_user);
    snprintf(local->helptext, sizeof(local->helptext), "Challenge %s?", user->name);
    component *challenge_label = label_create(local->helptext);
    component *yes_button = button_create("Yes", NULL, false, false, lobby_do_challenge, s);
    component *no_button = button_create("No", NULL, false, false, lobby_cancel_challenge, s);

    label_set_text_shadow(challenge_label, GLYPH_SHADOW_BOTTOM, 9);
    button_set_text_shadow(yes_button, GLYPH_SHADOW_BOTTOM, 9);
    button_set_text_shadow(no_button, GLYPH_SHADOW_BOTTOM, 9);

    menu_attach(menu, challenge_label);
    menu_attach(menu, yes_button);
    menu_attach(menu, no_button);

    return menu;
}

void lobby_challenge(component *c, void *userdata) {
    scene *s = userdata;
    menu_set_submenu(c->parent, lobby_challenge_create(s));
}

void lobby_do_yell(component *c, void *userdata) {
    scene *scene = userdata;
    lobby_local *local = scene_get_userdata(scene);

    // menu *m = sizer_get_obj(c->parent);
    const char *yell = textinput_value(c);

    if(strlen(yell) > 0) {
        log_debug("yelled %s", textinput_value(c));

        serial ser;
        serial_create(&ser);
        serial_write_int8(&ser, PACKET_YELL << 4);
        serial_write(&ser, yell, strlen(yell));

        ENetPacket *packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
        serial_free(&ser);

        enet_peer_send(local->peer, 0, packet);

        textinput_clear(c);
    }
}

component *lobby_yell_create(scene *s) {
    component *menu = menu_create();

    menu_set_help_pos(menu, 10, 155, 500, 10);
    menu_set_help_text_settings(menu, FONT_NET2, TEXT_ALIGN_LEFT, 56);
    menu_set_horizontal(menu, true);
    menu_set_background(menu, false);

    component *yell_label = label_create("Yell:");
    label_set_text_shadow(yell_label, GLYPH_SHADOW_BOTTOM, 9);
    menu_attach(menu, yell_label);
    component *yell_input =
        textinput_create(32,
                         "Yell a message to everybody in the challenge arena.\n\n\n\n\nTo whisper to one player, type "
                         "their name, a ':', and your message.\nPress 'esc' to return to the challenge arena menu.",
                         "");
    textinput_set_text_shadow(yell_input, GLYPH_SHADOW_BOTTOM, 9);
    textinput_set_font(yell_input, FONT_NET1);
    textinput_set_horizontal_align(yell_input, TEXT_ALIGN_LEFT);
    menu_attach(menu, yell_input);
    textinput_enable_background(yell_input, false);
    textinput_set_done_cb(yell_input, lobby_do_yell, s);

    return menu;
}

static text *create_log_message(const char *status, vga_index color) {
    text *t = text_create_with_font_and_size(FONT_NET1, 300, 200);
    text_set_color(t, color);
    text_generate_layout(t);
    text_set_from_c(t, status);
    text_generate_layout(t);
    return t;
}

void lobby_do_whisper(component *c, void *userdata) {
    menu *m = sizer_get_obj(c->parent);
    scene *s = userdata;

    const char *whisper = textinput_value(c);

    if(strlen(whisper) > 0) {
        log_debug("whispered %s", whisper);

        lobby_local *local = scene_get_userdata(s);
        lobby_user *user = list_get(&local->users, local->active_user);
        log_debug("active_user is %d", local->active_user);
        log_debug("whispered %s", textinput_value(c));
        serial ser;
        serial_create(&ser);
        serial_write_int8(&ser, PACKET_WHISPER << 4);
        serial_write_int32(&ser, user->id);
        serial_write(&ser, whisper, strlen(whisper));

        ENetPacket *packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
        serial_free(&ser);

        enet_peer_send(local->peer, 0, packet);

        str tmp;
        str_from_format(&tmp, "%s: %s", local->nat, whisper);
        log_event log = {create_log_message(str_c(&tmp), WHISPER_COLOR)};
        list_append(&local->log, &log, sizeof(log));
        str_free(&tmp);

        m->finished = 1;
        local->mode = LOBBY_MAIN;
        textinput_clear(c);
    }
}

component *lobby_whisper_create(scene *s) {
    lobby_local *local = scene_get_userdata(s);

    component *menu = menu_create();

    menu_set_help_pos(menu, 10, 155, 500, 10);
    menu_set_help_text_settings(menu, FONT_NET2, TEXT_ALIGN_LEFT, 56);
    menu_set_horizontal(menu, true);
    menu_set_background(menu, false);

    component *whisper_label = label_create("Whisper:");
    label_set_text_shadow(whisper_label, GLYPH_SHADOW_BOTTOM, 9);
    menu_attach(menu, whisper_label);
    lobby_user *user = list_get(&local->users, local->active_user);
    snprintf(local->helptext, sizeof(local->helptext), "Whisper a message to %s. Press enter when done, esc to abort.",
             user->name);
    component *whisper_input = textinput_create(32, local->helptext, "");
    textinput_set_text_shadow(whisper_input, GLYPH_SHADOW_BOTTOM, 9);
    textinput_set_horizontal_align(whisper_input, TEXT_ALIGN_LEFT);
    textinput_set_font(whisper_input, FONT_NET1);
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
    lobby_local *local = scene_get_userdata(s);
    local->mode = LOBBY_YELL;
    menu_set_submenu(c->parent, lobby_yell_create(s));
}

void lobby_refresh(component *c, void *userdata) {
    scene *s = userdata;
    lobby_local *local = scene_get_userdata(s);

    serial ser;
    serial_create(&ser);
    serial_write_int8(&ser, (uint8_t)(PACKET_REFRESH << 4));

    ENetPacket *packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
    serial_free(&ser);

    enet_peer_send(local->peer, 0, packet);
}

void lobby_do_exit(component *c, void *userdata) {
    scene *scene = userdata;
    lobby_local *local = scene_get_userdata(scene);
    nat_free((nat_ctx *)local->peer->data);
    omf_free(local->peer->data);

    game_state_set_next(scene->gs, SCENE_MENU);
}

void lobby_refuse_exit(component *c, void *userdata) {
    menu *m = sizer_get_obj(c->parent);
    scene *s = userdata;
    lobby_local *local = scene_get_userdata(s);
    m->finished = 1;
    local->mode = LOBBY_MAIN;
}

void lobby_entered_name(component *c, void *userdata) {
    if(strlen(textinput_value(c))) {
        scene *scene = userdata;
        lobby_local *local = scene_get_userdata(scene);

        strncpy_or_truncate(local->name, textinput_value(c), sizeof(local->name));

        char version[VERSION_BUF_SIZE];
        // TODO support git version when not on a tag
        snprintf(version, sizeof(version), "%s", get_version_string());
        serial ser;
        serial_create(&ser);
        serial_write_int8(&ser, PACKET_JOIN << 4 | (PROTOCOL_VERSION & 0x0f));
        // if we mapped an external port, send it to the server
        if(local->nat->type != NAT_TYPE_NONE) {
            serial_write_int16(&ser, local->nat->ext_port ? local->nat->ext_port : local->client->address.port);
        } else {
            serial_write_int16(&ser, local->client->address.port);
        }
        serial_write_int8(&ser, strlen(version));
        serial_write(&ser, version, strlen(version));
        const char *name = textinput_value(c);
        serial_write(&ser, name, strlen(name));

        omf_free(settings_get()->net.net_username);
        settings_get()->net.net_username = omf_strdup(name);

        ENetPacket *packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
        serial_free(&ser);

        enet_peer_send(local->peer, 0, packet);

        local->joinmenu = sizer_get_obj(c->parent);
    }
}

void lobby_dialog_cancel_connect(dialog *dlg, dialog_result result) {
    dialog_show(dlg, 0);
    scene *s = dlg->userdata;
    lobby_local *local = scene_get_userdata(s);
    serial ser;
    serial_create(&ser);
    serial_write_int8(&ser, PACKET_CHALLENGE << 4 | CHALLENGE_CANCEL);

    ENetPacket *packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
    serial_free(&ser);
    enet_peer_send(local->peer, 0, packet);
    if(local->opponent_peer) {
        enet_peer_reset(local->opponent_peer);
    }
    local->opponent_peer = NULL;
}

void lobby_try_connect(void *scenedata, void *userdata) {
    scene *s = scenedata;
    lobby_local *local = scene_get_userdata(s);
    if(local->opponent && !local->opponent_peer) {
        log_info("doing scheduled outbound connection to %d.%d.%d.%d port %d", local->opponent->address.host & 0xFF,
                 (local->opponent->address.host >> 8) & 0xFF, (local->opponent->address.host >> 16) & 0xF,
                 (local->opponent->address.host >> 24) & 0xFF, local->opponent->address.port);
        local->opponent_peer = enet_host_connect(local->client, &local->opponent->address, 3, 0);
        if(local->opponent_peer) {
            enet_peer_timeout(local->opponent_peer, 4, 1000, 1000);
        }
    }
}

void lobby_dialog_accept_challenge(dialog *dlg, dialog_result result) {
    dialog_show(dlg, 0);

    scene *s = dlg->userdata;
    lobby_local *local = scene_get_userdata(s);
    serial ser;
    serial_create(&ser);
    uint8_t flag = result == DIALOG_RESULT_YES_OK ? CHALLENGE_ACCEPT : CHALLENGE_REJECT;
    serial_write_int8(&ser, (PACKET_CHALLENGE << 4) | flag);

    ENetPacket *packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
    serial_free(&ser);
    enet_peer_send(local->peer, 0, packet);

    if(result == DIALOG_RESULT_YES_OK) {
        lobby_show_dialog(s, DIALOG_STYLE_CANCEL, "Establishing connection...", lobby_dialog_cancel_connect);
        local->connection_count = 0;
        local->role = ROLE_CHALLENGEE;
        ticktimer_add(&s->tick_timer, 500, lobby_try_connect, NULL);
    }
}

component *lobby_exit_create(scene *s) {
    component *menu = menu_create();
    menu_set_horizontal(menu, true);
    menu_set_background(menu, false);
    menu_set_padding(menu, 8);

    component *exit_label = label_create("Exit the Challenge Arena?");
    component *yes_button = button_create("Yes", NULL, false, false, lobby_do_exit, s);
    component *no_button = button_create("No", NULL, false, false, lobby_refuse_exit, s);

    label_set_text_shadow(exit_label, GLYPH_SHADOW_BOTTOM, 9);
    button_set_text_shadow(yes_button, GLYPH_SHADOW_BOTTOM, 9);
    button_set_text_shadow(no_button, GLYPH_SHADOW_BOTTOM, 9);

    menu_attach(menu, exit_label);
    menu_attach(menu, yes_button);
    menu_attach(menu, no_button);

    return menu;
}

void lobby_exit(component *c, void *userdata) {
    scene *s = userdata;
    menu_set_submenu(c->parent, lobby_exit_create(s));
}

void lobby_dialog_close(dialog *dlg, dialog_result result) {
    dialog_show(dlg, 0);
}

void lobby_dialog_close_exit(dialog *dlg, dialog_result result) {
    scene *s = dlg->userdata;
    dialog_show(dlg, 0);
    game_state_set_next(s->gs, SCENE_MENU);
}

void lobby_dialog_nat_cancel(dialog *dlg, dialog_result result) {
    scene *s = dlg->userdata;
    lobby_local *local = scene_get_userdata(s);
    local->nat_tries = 10;
}

void lobby_tick(scene *scene, int paused) {
    lobby_local *local = scene_get_userdata(scene);

    game_state *gs = scene->gs;
    ENetEvent event;
    serial ser;

    if(gs->this_wait_ticks) {
        // wait for the cross fade to finish
        // so the dialogs are visible
        return;
    }

    if(local->disconnected) {
        return;
    }

    // local->client goes NULL when entering a match, so avoid this code in that case
    if(!local->client && local->controllers_created == false) {
        ENetAddress address;
        address.host = ENET_HOST_ANY;
        address.port = settings_get()->net.net_listen_port_start;

        log_info("attempting to bind to port %d", address.port);

        if(address.port == 0) {
            address.port = rand_int(65535 - 1024) + 1024;
        }

        // Set up host
        local->controllers_created = false;
        int randtries = 0;

        int end_port = settings_get()->net.net_listen_port_end;
        if(!end_port) {
            end_port = 65535;
        }
        local->client = enet_host_create(&address, 2, 3, 0, 0);
        while(local->client == NULL) {
            log_info("requested port %d unavailable, trying ports %d to %d", address.port,
                     settings_get()->net.net_listen_port_start, end_port);
            if(settings_get()->net.net_listen_port_start == 0) {
                address.port = rand_int(65535 - 1024) + 1024;
                randtries++;
                if(randtries > 10) {
                    log_info("Failed to initialize ENet server, could not allocate random port");

                    lobby_show_dialog(scene, DIALOG_STYLE_OK,
                                      "Failed to initialize ENet server; could not allocate random port.",
                                      lobby_dialog_close_exit);

                    local->disconnected = true;
                    return;
                }
            } else {
                address.port++;
                if(address.port > end_port) {
                    log_info("Failed to initialize ENet server, port range exhausted");

                    lobby_show_dialog(scene, DIALOG_STYLE_OK, "Failed to initialize ENet server; port range exhausted.",
                                      lobby_dialog_close_exit);

                    local->disconnected = true;
                    return;
                }
                randtries++;
                if(randtries > 10) {
                    log_info("Failed to initialize ENet server, could not allocate port between %d and %d after 10 "
                             "tries",
                             settings_get()->net.net_listen_port_start, end_port);

                    lobby_show_dialog(
                        scene, DIALOG_STYLE_OK,
                        "Failed to initialize ENet server; could not allocate random port after 10 attempts.",
                        lobby_dialog_close_exit);

                    local->disconnected = true;
                    return;
                }
            }
            local->client = enet_host_create(&address, 2, 3, 0, 0);
        }

        log_info("bound to port %d", address.port);

        enet_socket_set_option(local->client->socket, ENET_SOCKOPT_REUSEADDR, 1);

        return;
    }

    if(!local->nat) {
        local->nat = omf_calloc(1, sizeof(nat_ctx));
        nat_create(local->nat);
        local->nat_tries = 0;
        if(local->nat->type != NAT_TYPE_NONE) {
            lobby_show_dialog(scene, DIALOG_STYLE_CANCEL, "Attempting NAT traversal...", lobby_dialog_nat_cancel);
        }
    }

    if(local->client && local->nat_tries < 10 && local->nat->type != NAT_TYPE_NONE) {
        uint16_t ext_port;
        if(settings_get()->net.net_ext_port_start == 0) {
            ext_port = rand_int(65535 - 1024) + 1024;
        } else {
            ext_port = settings_get()->net.net_ext_port_start + local->nat_tries;
        }

        if(!nat_create_mapping(local->nat, local->client->address.port, ext_port)) {
            local->nat_tries++;
            return;
        }
    }

    if(local->nat_tries < 11) {
        // nat has either finished or failed
        // increment this so far it won't trip again
        local->nat_tries = 12;

        lobby_show_dialog(scene, DIALOG_STYLE_CANCEL, "Connecting to lobby...", lobby_dialog_close_exit);
    }

    if(!local->peer) {
        ENetAddress lobby_address;
        enet_address_set_host(&lobby_address, settings_get()->net.net_lobby_address);
        // enet_address_set_host(&address, "127.0.0.1");
        lobby_address.port = 2098;
        log_debug("server address is %s", settings_get()->net.net_lobby_address);
        /* Initiate the connection, allocating the two channels 0, 1 and 2. */
        local->peer = enet_host_connect(local->client, &lobby_address, 3, 0);
        if(local->peer == NULL) {
            lobby_show_dialog(scene, DIALOG_STYLE_OK, "No available peers for initiating an ENet connection.",
                              lobby_dialog_close_exit);
            local->disconnected = true;
            return;
        }
        enet_peer_ping_interval(local->peer, 100);
    }

    while(local->client && !local->controllers_created && enet_host_service(local->client, &event, 0) > 0) {
        switch(event.type) {
            case ENET_EVENT_TYPE_NONE:
                break;
            case ENET_EVENT_TYPE_CONNECT:
                // check both address and port in case everything is running on localhost
                if(event.peer->address.host == local->peer->address.host &&
                   event.peer->address.port == local->peer->address.port) {
                    log_debug("Connection to server succeeded.");

                    log_debug("local peer connect id %d", local->peer->connectID);
                    log_debug("remote peer connect id %d", event.peer->connectID);

                    event.peer->data = local->nat;

                    // close any active dialogs
                    dialog_show(local->dialog, 0);
                    dialog_free(local->dialog);
                    omf_free(local->dialog);

                    break;
                }
                log_debug("A new client connected from %x:%u.", event.peer->address.host, event.peer->address.port);

                /* Store any relevant client information here. */
                event.peer->data = NULL;
                log_debug("new peer was %d, server peer was %d, opponent peer was %d", event.peer, local->peer,
                          local->opponent_peer);

                if(local->opponent_peer && event.peer->address.host == local->opponent->address.host) {
                    log_debug("connected to peer outbound!");

                    // TODO probably need a more specific cancel callback here
                    lobby_show_dialog(scene, DIALOG_STYLE_CANCEL, "Connected, synchronizing clocks...", NULL);
                    local->opponent_peer = event.peer;
                    serial_create(&ser);
                    serial_write_int8(&ser, PACKET_JOIN << 4);

                    ENetPacket *packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
                    serial_free(&ser);

                    enet_peer_send(local->opponent_peer, 0, packet);

                    // signal the server we're connected
                    serial_create(&ser);
                    serial_write_int8(&ser, PACKET_CONNECTED << 4);
                    packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(local->peer, 0, packet);
                    serial_free(&ser);

                    controller *net_ctrl;
                    game_player *p1 = game_state_get_player(gs, 0);
                    game_player *p2 = game_state_get_player(gs, 1);
                    gs->net_mode = NET_MODE_LOBBY;

                    // force the speed to 3
                    game_state_set_speed(gs, 10);

                    p1->pilot->har_id = HAR_JAGUAR;
                    p1->pilot->pilot_id = 0;
                    p1->pilot->name[0] = '\0';
                    p2->pilot->har_id = HAR_JAGUAR;
                    p2->pilot->pilot_id = 0;
                    p2->pilot->name[0] = '\0';

                    net_ctrl = omf_calloc(1, sizeof(controller));
                    controller_init(net_ctrl, gs);

                    game_player *challengee;

                    int player_id = 0;
                    if(local->role == ROLE_CHALLENGER) {
                        // we did the connecting and we're the challenger
                        // so we are player 1
                        challengee = p2;
                    } else {
                        player_id = 1;
                        challengee = p1;
                    }

                    net_ctrl->har_obj_id = challengee->har_obj_id;

                    // Challenger -- Network
                    net_controller_create(net_ctrl, local->client, event.peer, local->peer,
                                          local->role == ROLE_CHALLENGER ? ROLE_SERVER : ROLE_CLIENT);
                    game_player_set_ctrl(challengee, net_ctrl);

                    // Challengee -- local
                    settings_keyboard *k = &settings_get()->keys;
                    if(k->ctrl_type1 == CTRL_TYPE_KEYBOARD) {
                        _setup_keyboard(scene->gs, player_id, 0);
                    } else if(k->ctrl_type1 == CTRL_TYPE_GAMEPAD) {
                        _setup_joystick(scene->gs, player_id, k->joy_name1, k->joy_offset1);
                    }

                    game_player_set_selectable(challengee, 1);

                    chr_score_set_difficulty(game_player_get_score(game_state_get_player(gs, 0)),
                                             AI_DIFFICULTY_CHAMPION);
                    chr_score_set_difficulty(game_player_get_score(game_state_get_player(gs, 1)),
                                             AI_DIFFICULTY_CHAMPION);

                    local->controllers_created = true;
                }

                break;
            case ENET_EVENT_TYPE_RECEIVE:

                serial_create_from(&ser, (const char *)event.packet->data, event.packet->dataLength);
                uint8_t control_byte = serial_read_int8(&ser);
                log_debug("A packet of length %u with control byte %d was received on channel %u.",
                          event.packet->dataLength, control_byte, event.channelID);
                switch(control_byte >> 4) {
                    case PACKET_PRESENCE: {
                        lobby_user user;
                        memset(&user, 0, sizeof(lobby_user));
                        user.id = serial_read_uint32(&ser);
                        user.address.host = serial_read_uint32(&ser);
                        user.port = serial_read_uint16(&ser);
                        user.ext_port = serial_read_uint16(&ser);
                        if(user.ext_port != 0) {
                            user.address.port = user.ext_port;
                        } else {
                            user.address.port = user.port;
                        }
                        user.wins = serial_read_int8(&ser);
                        user.losses = serial_read_int8(&ser);
                        user.status = serial_read_int8(&ser);
                        uint8_t version_len = serial_read_int8(&ser);
                        if(version_len < sizeof(user.version)) {
                            serial_read(&ser, user.version, version_len);
                            user.version[version_len] = 0;
                            uint8_t name_len = ser.wpos - ser.rpos;
                            if(name_len > 0 && name_len < 16) {
                                serial_read(&ser, user.name, name_len);
                                user.name[name_len] = 0;
                                // check if this user already exists and update it if it does
                                iterator it;
                                list_iter_begin(&local->users, &it);
                                lobby_user *u;
                                bool found = false;
                                foreach(it, u) {
                                    if(u->id == user.id) {
                                        found = true;
                                        u->wins = user.wins;
                                        u->losses = user.losses;
                                        u->status = user.status;
                                        u->address.host = user.address.host;
                                        u->port = user.port;
                                        u->ext_port = user.ext_port;
                                        u->address.port = user.address.port;
                                        memcpy(u->version, user.version, sizeof(u->version));
                                        update_lobby_user_texts(u, false);
                                        break;
                                    }
                                }
                                if(!found) {
                                    update_lobby_user_texts(&user, true);
                                    list_append(&local->users, &user, sizeof(lobby_user));
                                    if(control_byte & 0x8) {
                                        str tmp;
                                        str_from_format(&tmp, "%s has entered the Arena", user.name);
                                        log_event log = {create_log_message(str_c(&tmp), JOIN_COLOR)};
                                        list_append(&local->log, &log, sizeof(log));
                                        str_free(&tmp);
                                    }
                                }
                                update_active_user_text(local);
                            }
                        }
                    } break;
                    case PACKET_JOIN:
                        if(event.peer == local->peer) {
                            switch(control_byte & 0xf) {
                                case JOIN_SUCCESS:
                                    local->id = serial_read_uint32(&ser);
                                    log_debug("successfully joined lobby and assigned ID %d", local->id);
                                    if(local->joinmenu) {
                                        local->joinmenu->finished = 1;
                                        local->joinmenu = NULL;
                                    }
                                    local->mode = LOBBY_MAIN;
                                    break;
                                case JOIN_ERROR_NAME_USED:
                                    lobby_show_dialog(scene, DIALOG_STYLE_OK, "Username already in use.",
                                                      lobby_dialog_close);
                                    break;
                                case JOIN_ERROR_NAME_INVALID:
                                    lobby_show_dialog(scene, DIALOG_STYLE_OK, "Username invalid.", lobby_dialog_close);
                                    break;
                                case JOIN_ERROR_UNSUPPORTED_PROTOCOL:
                                    lobby_show_dialog(scene, DIALOG_STYLE_OK,
                                                      "Lobby server does not support this protocol version.",
                                                      lobby_dialog_close_exit);
                                    break;
                                default: {
                                    char buf[80];
                                    snprintf(buf, sizeof(buf), "Unknown join error %d", control_byte & 0xf);
                                    lobby_show_dialog(scene, DIALOG_STYLE_OK, buf, lobby_dialog_close_exit);
                                } break;
                            }
                        } else if(!local->opponent_peer && event.peer->address.host == local->opponent->address.host) {
                            log_debug("connected to peer inbound!");
                            local->opponent_peer = event.peer;

                            // TODO probably need a more specific cancel callback here
                            lobby_show_dialog(scene, DIALOG_STYLE_CANCEL, "Connected, synchronizing clocks...", NULL);

                            // signal the server we're connected
                            serial reply_ser;
                            serial_create(&reply_ser);
                            serial_write_int8(&reply_ser, PACKET_CONNECTED << 4);
                            ENetPacket *packet =
                                enet_packet_create(reply_ser.data, serial_len(&reply_ser), ENET_PACKET_FLAG_RELIABLE);
                            enet_peer_send(local->peer, 0, packet);
                            serial_free(&reply_ser);

                            controller *net_ctrl;
                            game_player *p1 = game_state_get_player(gs, 0);
                            game_player *p2 = game_state_get_player(gs, 1);
                            gs->net_mode = NET_MODE_LOBBY;

                            // force the speed to 3
                            game_state_set_speed(gs, 10);

                            p1->pilot->har_id = HAR_JAGUAR;
                            p1->pilot->pilot_id = 0;
                            p1->pilot->name[0] = '\0';
                            p2->pilot->har_id = HAR_JAGUAR;
                            p2->pilot->pilot_id = 0;
                            p2->pilot->name[0] = '\0';

                            net_ctrl = omf_calloc(1, sizeof(controller));
                            controller_init(net_ctrl, gs);

                            game_player *challengee;

                            int player_id = 0;
                            if(local->role == ROLE_CHALLENGER) {
                                // we were connected TO but we are the challenger
                                // so we are player 1
                                challengee = p2;
                            } else {
                                player_id = 1;
                                challengee = p1;
                            }

                            net_ctrl->har_obj_id = challengee->har_obj_id;

                            // Challengee -- Network
                            net_controller_create(net_ctrl, local->client, event.peer, local->peer,
                                                  local->role == ROLE_CHALLENGER ? ROLE_SERVER : ROLE_CLIENT);
                            game_player_set_ctrl(challengee, net_ctrl);

                            // Challenger -- local
                            settings_keyboard *k = &settings_get()->keys;
                            if(k->ctrl_type1 == CTRL_TYPE_KEYBOARD) {
                                _setup_keyboard(scene->gs, player_id, 0);
                            } else if(k->ctrl_type1 == CTRL_TYPE_GAMEPAD) {
                                _setup_joystick(scene->gs, player_id, k->joy_name1, k->joy_offset1);
                            }

                            game_player_set_selectable(challengee, 1);

                            chr_score_set_difficulty(game_player_get_score(game_state_get_player(gs, 0)),
                                                     AI_DIFFICULTY_CHAMPION);
                            chr_score_set_difficulty(game_player_get_score(game_state_get_player(gs, 1)),
                                                     AI_DIFFICULTY_CHAMPION);

                            local->controllers_created = true;

                        } else {
                            log_debug("opponent peer %d, host %d %d", local->opponent_peer, event.peer->address.host,
                                      local->opponent->address.host);
                        }
                        break;
                    case PACKET_YELL: {
                        char tmp[150];
                        strncpy_or_truncate(tmp, (char *)event.packet->data + 1, sizeof(tmp));
                        log_event log = {create_log_message(tmp, YELL_COLOR)};
                        list_append(&local->log, &log, sizeof(log));
                    } break;
                    case PACKET_WHISPER: {
                        char tmp[150];
                        strncpy_or_truncate(tmp, (char *)event.packet->data + 1, sizeof(tmp));
                        log_event log = {create_log_message(tmp, WHISPER_COLOR)};
                        list_append(&local->log, &log, sizeof(log));
                    } break;
                    case PACKET_ANNOUNCEMENT: {
                        char tmp[150];
                        strncpy_or_truncate(tmp, (char *)event.packet->data + 1, sizeof(tmp));
                        log_event log = {create_log_message(tmp, ANNOUNCEMENT_COLOR)};
                        list_append(&local->log, &log, sizeof(log));
                    } break;
                    case PACKET_RELAY: {
                        lobby_show_dialog(scene, DIALOG_STYLE_CANCEL, "Connected via relay, synchronizing clocks...",
                                          NULL);
                        // lobby and opponent peer are now the same
                        local->opponent_peer = local->peer;
                        serial_create(&ser);
                        serial_write_int8(&ser, PACKET_JOIN << 4);

                        ENetPacket *packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
                        serial_free(&ser);

                        enet_peer_send(local->opponent_peer, 0, packet);

                        // signal the server we're connected
                        serial_create(&ser);
                        serial_write_int8(&ser, PACKET_CONNECTED << 4);
                        packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
                        enet_peer_send(local->peer, 0, packet);
                        serial_free(&ser);

                        controller *net_ctrl;
                        game_player *p1 = game_state_get_player(gs, 0);
                        game_player *p2 = game_state_get_player(gs, 1);
                        gs->net_mode = NET_MODE_LOBBY;

                        // force the speed to 3
                        game_state_set_speed(gs, 10);

                        p1->pilot->har_id = HAR_JAGUAR;
                        p1->pilot->pilot_id = 0;
                        p1->pilot->name[0] = '\0';
                        p2->pilot->har_id = HAR_JAGUAR;
                        p2->pilot->pilot_id = 0;
                        p2->pilot->name[0] = '\0';

                        net_ctrl = omf_calloc(1, sizeof(controller));
                        controller_init(net_ctrl, gs);

                        game_player *challengee;

                        int player_id = 0;
                        if(local->role == ROLE_CHALLENGER) {
                            // we did the connecting and we're the challenger
                            // so we are player 1
                            challengee = p2;
                        } else {
                            player_id = 1;
                            challengee = p1;
                        }

                        net_ctrl->har_obj_id = challengee->har_obj_id;

                        // Challenger -- Network
                        net_controller_create(net_ctrl, local->client, event.peer, local->peer,
                                              local->role == ROLE_CHALLENGER ? ROLE_SERVER : ROLE_CLIENT);
                        game_player_set_ctrl(challengee, net_ctrl);

                        // Challengee -- local
                        settings_keyboard *k = &settings_get()->keys;
                        if(k->ctrl_type1 == CTRL_TYPE_KEYBOARD) {
                            _setup_keyboard(scene->gs, player_id, 0);
                        } else if(k->ctrl_type1 == CTRL_TYPE_GAMEPAD) {
                            _setup_joystick(scene->gs, player_id, k->joy_name1, k->joy_offset1);
                        }

                        game_player_set_selectable(challengee, 1);

                        chr_score_set_difficulty(game_player_get_score(game_state_get_player(gs, 0)),
                                                 AI_DIFFICULTY_CHAMPION);
                        chr_score_set_difficulty(game_player_get_score(game_state_get_player(gs, 1)),
                                                 AI_DIFFICULTY_CHAMPION);

                        local->controllers_created = true;

                    } break;
                    case PACKET_DISCONNECT: {
                        uint32_t connect_id = serial_read_uint32(&ser);
                        iterator it;
                        list_iter_begin(&local->users, &it);
                        lobby_user *user;
                        foreach(it, user) {
                            if(user->id == connect_id) {
                                str tmp;
                                str_from_format(&tmp, "%s has left the Arena", user->name);
                                log_event log = {create_log_message(str_c(&tmp), LEAVE_COLOR)};
                                list_append(&local->log, &log, sizeof(log));
                                str_free(&tmp);
                                if(local->opponent == user) {
                                    local->opponent = NULL;
                                    // any existing dialogs were for this user
                                    if(local->dialog) {
                                        dialog_free(local->dialog);
                                        omf_free(local->dialog);
                                    }
                                    // TODO do we need to pop a dialog saying "user disconnected"?
                                }
                                text_free(&user->wins_text);
                                text_free(&user->version_text);
                                text_free(&user->name_text);
                                list_delete(&local->users, &it);
                                break;
                            }
                        }
                    } break;
                    case PACKET_CHALLENGE: {
                        switch(control_byte & 0xf) {
                            case CHALLENGE_OFFER: {
                                uint32_t connect_id = serial_read_uint32(&ser);

                                log_debug("got challenge from %d, we are %d", connect_id, local->id);
                                iterator it;
                                list_iter_begin(&local->users, &it);
                                lobby_user *user;
                                bool found = false;
                                foreach(it, user) {
                                    if(user->id == connect_id) {
                                        found = true;
                                        break;
                                    }
                                }

                                if(found) {
                                    local->opponent = user;
                                    char buf[80];
                                    local->role = ROLE_CHALLENGEE;
                                    snprintf(buf, sizeof(buf), "Accept challenge from %s?", user->name);
                                    lobby_show_dialog(scene, DIALOG_STYLE_YES_NO, buf, lobby_dialog_accept_challenge);
                                } else {
                                    log_debug("unable to find user with id %d", connect_id);
                                }
                            } break;
                            case CHALLENGE_ACCEPT:
                                // peer accepted, try to connect to them
                                lobby_show_dialog(scene, DIALOG_STYLE_CANCEL, "Establishing connection...",
                                                  lobby_dialog_cancel_connect);

                                // try to connect immediately
                                local->opponent_peer =
                                    enet_host_connect(local->client, &local->opponent->address, 3, 0);

                                log_debug("doing immediate outbound connection to %d.%d.%d.%d port %d",
                                          local->opponent->address.host & 0xFF,
                                          (local->opponent->address.host >> 8) & 0xFF,
                                          (local->opponent->address.host >> 16) & 0xF,
                                          (local->opponent->address.host >> 24) & 0xFF, local->opponent->address.port);
                                if(local->opponent_peer) {
                                    enet_peer_timeout(local->opponent_peer, 4, 1000, 1000);
                                }
                                local->connection_count = 0;
                                break;
                            case CHALLENGE_REJECT:
                                // peer rejected our challenge
                                lobby_show_dialog(scene, DIALOG_STYLE_OK, "Challenge rejected.", lobby_dialog_close);
                                break;
                            case CHALLENGE_CANCEL:
                                // peer cancelled their challenge
                                lobby_show_dialog(scene, DIALOG_STYLE_OK, "Challenge cancelled by peer.",
                                                  lobby_dialog_close);
                                if(local->opponent_peer) {
                                    enet_peer_reset(local->opponent_peer);
                                }
                                local->opponent_peer = NULL;
                                local->opponent = NULL;
                                break;
                            case CHALLENGE_ERROR: {
                                uint8_t error_len = ser.wpos - ser.rpos;
                                char buf[200];
                                serial_read(&ser, buf, min2(sizeof(buf) - 1, error_len));
                                buf[error_len] = 0;
                                lobby_show_dialog(scene, DIALOG_STYLE_OK, buf, lobby_dialog_close);
                            } break;
                        }
                    } break;
                    default:
                        log_debug("unknown packet of type %d received", event.packet->data[0] >> 4);
                        break;
                }
                serial_free(&ser);
                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy(event.packet);

                break;
            case ENET_EVENT_TYPE_DISCONNECT:

                if(event.peer == local->opponent_peer) {
                    local->connection_count++;
                    log_debug("outbound peer connection failed");
                    enet_peer_reset(local->opponent_peer);
                    local->opponent_peer = NULL;

                    if(local->connection_count < 2) {
                        // signal the server we failed to connect first time
                        serial_create(&ser);
                        serial_write_int8(&ser, PACKET_CONNECTED << 4 | 1);
                        ENetPacket *packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
                        enet_peer_send(local->peer, 0, packet);
                        serial_free(&ser);

                        if(local->opponent->address.port != local->opponent->port && local->opponent->ext_port != 0) {
                            // the user's claimed port didn't work, try the one the server saw
                            local->opponent->address.port = local->opponent->port;
                            // reset the counter
                            local->connection_count = 0;
                        }

                        ticktimer_add(&scene->tick_timer, 150, lobby_try_connect, NULL);
                    } else {
                        // signal the server we failed to connect second time
                        serial_create(&ser);
                        serial_write_int8(&ser, PACKET_CONNECTED << 4 | 2);
                        ENetPacket *packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
                        enet_peer_send(local->peer, 0, packet);
                        serial_free(&ser);
                    }
                }

                /* Reset the peer's client information. */

                if(event.peer && local->peer == event.peer && event.peer->data) {
                    nat_free((nat_ctx *)event.peer->data);
                    omf_free(event.peer->data);
                    game_state_set_next(gs, SCENE_MENU);
                }

                // event.peer->data = NULL;
        }
    }
    local->active_user = min2(local->active_user, list_size(&local->users) - 1);
    update_active_user_text(local);
    gui_frame_tick(local->frame);

    component *c = gui_frame_get_root(local->frame);
    if((c = menu_get_submenu(c)) && menu_is_finished(c)) {
        local->mode = LOBBY_MAIN;
    }

    game_player *p1 = game_state_get_player(gs, 0);
    controller *c1 = game_player_get_ctrl(p1);
    if(c1->type == CTRL_TYPE_NETWORK && net_controller_ready(c1)) {
        log_debug("network peer is ready, tick offset is %d and rtt is %d", net_controller_tick_offset(c1), c1->rtt);
        local->client = NULL;
        game_state_set_next(gs, SCENE_MELEE);
    }

    game_player *p2 = game_state_get_player(gs, 1);
    controller *c2 = game_player_get_ctrl(p2);
    if(c2->type == CTRL_TYPE_NETWORK && net_controller_ready(c2) == 1) {
        log_debug("network peer is ready, tick offset is %d and rtt is %d", net_controller_tick_offset(c2), c2->rtt);
        local->client = NULL;
        game_state_set_next(gs, SCENE_MELEE);
    }
}

int lobby_create(scene *scene) {

    lobby_local *local;

    // force the match to use reasonable defaults
    game_state_match_settings_defaults(scene->gs);

    fight_stats *fight_stats = &scene->gs->fight_stats;
    memset(fight_stats, 0, sizeof(*fight_stats));

    // Initialize local struct
    local = omf_calloc(1, sizeof(lobby_local));
    scene_set_userdata(scene, local);

    local->name[0] = 0;
    local->mode = LOBBY_STARTING;
    list_create(&local->log);

    local->nat_tries = 0;
    local->disconnected = false;

    // Create lobby theme
    gui_theme theme;
    gui_theme_defaults(&theme);
    theme.text.font = FONT_NET1;
    theme.dialog.border_color = DIALOG_BORDER_COLOR;
    theme.text.primary_color = TEXT_PRIMARY_COLOR;
    theme.text.secondary_color = TEXT_SECONDARY_COLOR;
    theme.text.disabled_color = TEXT_DISABLED_COLOR;
    theme.text.active_color = TEXT_ACTIVE_COLOR;
    theme.text.inactive_color = TEXT_INACTIVE_COLOR;
    theme.text.shadow_color = TEXT_SHADOW_COLOR;

    component *menu = menu_create();
    menu_set_horizontal(menu, true);
    menu_set_background(menu, false);
    menu_set_padding(menu, 6);

    menu_set_help_pos(menu, 10, 155, 500, 10);

    menu_set_help_text_settings(menu, FONT_NET2, TEXT_ALIGN_LEFT, 56);
    component *challenge_button =
        button_create("Challenge", "Challenge this player to a fight. Challenge yourself for 1-player game.", false,
                      false, lobby_challenge, scene);
    component *whisper_button =
        button_create("Whisper", "Whisper a message to this player.", false, false, lobby_whisper, scene);
    component *yell_button =
        button_create("Yell", "Chat with everybody in the arena.", false, false, lobby_yell, scene);
    component *refresh_button =
        button_create("Refresh", "Refresh the player list.", false, false, lobby_refresh, scene);
    component *exit_button = button_create("Exit", "Exit and disconnect.", false, false, lobby_exit, scene);

    button_set_text_shadow(challenge_button, GLYPH_SHADOW_BOTTOM, 9);
    button_set_text_shadow(whisper_button, GLYPH_SHADOW_BOTTOM, 9);
    button_set_text_shadow(yell_button, GLYPH_SHADOW_BOTTOM, 9);
    button_set_text_shadow(refresh_button, GLYPH_SHADOW_BOTTOM, 9);
    button_set_text_shadow(exit_button, GLYPH_SHADOW_BOTTOM, 9);

    menu_attach(menu, challenge_button);
    menu_attach(menu, whisper_button);
    menu_attach(menu, yell_button);
    menu_attach(menu, refresh_button);
    menu_attach(menu, exit_button);

    int winner = -1;
    // check if there's already a net controller provisioned
    // and harvest the information from it, if possible
    if(game_state_get_player(scene->gs, 0)->ctrl->type == CTRL_TYPE_NETWORK) {
        local->peer = net_controller_get_lobby_connection(game_state_get_player(scene->gs, 0)->ctrl);
        local->client = net_controller_get_host(game_state_get_player(scene->gs, 0)->ctrl);
        local->nat = local->peer->data;
        winner = net_controller_get_winner(game_state_get_player(scene->gs, 0)->ctrl);
        local->mode = LOBBY_MAIN;
        local->nat_tries = 12;
    } else if(game_state_get_player(scene->gs, 1)->ctrl->type == CTRL_TYPE_NETWORK) {
        local->peer = net_controller_get_lobby_connection(game_state_get_player(scene->gs, 1)->ctrl);
        local->client = net_controller_get_host(game_state_get_player(scene->gs, 1)->ctrl);
        local->nat = local->peer->data;
        winner = net_controller_get_winner(game_state_get_player(scene->gs, 1)->ctrl);
        local->mode = LOBBY_MAIN;
        local->nat_tries = 12;
    }

    // Cleanups and resets
    for(int i = 0; i < 2; i++) {
        // destroy any leftover controllers
        game_player *player = game_state_get_player(scene->gs, i);
        game_player_set_ctrl(player, NULL);
    }
    reconfigure_controller(scene->gs);

    // Title texts
    local->titles[0] = create_big_text(50, 8, "Player");
    local->titles[1] = create_big_text(50, 8, "Action");
    local->titles[2] = create_small_text(50, 6, "Wn/Loss");
    local->titles[3] = create_small_text(50, 6, "Version");
    local->titles[4] = create_small_text(40, 6, "0 of 0");

    // Presence texts. These should match the enums.
    const char *presences[] = {"",          "unknown",  "starting", "available", "practicing", "challenging",
                               "pondering", "fighting", "watching"};
    for(int i = 0; i < PRESENCE_COUNT; i++) {
        local->presences[i] = create_small_text(70, 6, presences[i]);
        text_set_color(local->presences[i], 40);
    }

    local->frame = gui_frame_create(&theme, 9, 132, 300, 8);
    gui_frame_set_root(local->frame, menu);
    gui_frame_layout(local->frame);

    if(local->mode == LOBBY_STARTING) {

        component *name_menu = menu_create();
        menu_set_horizontal(name_menu, true);
        menu_set_background(name_menu, false);
        menu_set_padding(name_menu, 0);

        component *enter_name_label = label_create("Enter your name:");
        label_set_text_shadow(enter_name_label, GLYPH_SHADOW_BOTTOM, 9);
        menu_attach(name_menu, enter_name_label);
        // pull the last used name from settings
        component *name_input = textinput_create(14, "", settings_get()->net.net_username);
        textinput_set_text_shadow(name_input, GLYPH_SHADOW_BOTTOM, 9);
        textinput_set_font(name_input, FONT_NET1);
        textinput_enable_background(name_input, false);
        textinput_set_horizontal_align(name_input, TEXT_ALIGN_LEFT);
        textinput_set_done_cb(name_input, lobby_entered_name, scene);
        menu_attach(name_menu, name_input);

        menu_set_submenu(menu, name_menu);

        lobby_show_dialog(scene, DIALOG_STYLE_CANCEL, "Establishing network socket...", lobby_dialog_close_exit);

    } else {
        serial ser;
        serial_create(&ser);

        if(winner >= 0) {
            serial_write_int8(&ser, PACKET_CHALLENGE << 4 | CHALLENGE_DONE);
            serial_write_int8(&ser, (uint8_t)winner);
            ENetPacket *packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(local->peer, 0, packet);
            serial_free(&ser);
        }

        // send the server a REFRESH command so we can get the userlist, our username, etc
        serial_write_int8(&ser, (uint8_t)(PACKET_REFRESH << 4));

        ENetPacket *packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
        serial_free(&ser);

        enet_peer_send(local->peer, 0, packet);
    }

    scene_set_input_poll_cb(scene, lobby_input_tick);

    scene_set_dynamic_tick_cb(scene, lobby_tick);

    scene_set_free_cb(scene, lobby_free);

    scene_set_render_overlay_cb(scene, lobby_render_overlay);
    scene_set_event_cb(scene, lobby_event);

    return 0;
}
