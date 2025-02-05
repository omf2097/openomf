#include "game/gui/dialog.h"
#include "game/gui/frame.h"
#include "game/protos/scene.h"
#include "game/utils/serial.h"
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
#define TEXT_COLOR 0xFE
#define BLUE_TEXT_COLOR 0xFE
#define TEXT_BLACK 1

#define YELL_COLOR 3
#define JOIN_COLOR 7
#define WHISPER_COLOR 6
#define LEAVE_COLOR 5

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
};

enum
{
    CHALLENGE_FLAG_ACCEPT = 1 << 0,
    CHALLENGE_FLAG_REJECT = 1 << 1,
    CHALLENGE_FLAG_CANCEL = 1 << 2
};

typedef struct lobby_user_t {
    char name[16];
    char version[15];
    bool self;
    ENetAddress address;
    uint16_t port;     // port the server sees this user connecting from
    uint16_t ext_port; // port this user claims will route inbound to them (or 0)
    uint32_t id;
    uint8_t wins;
    uint8_t losses;
} lobby_user;

typedef struct lobby_local_t {
    char name[16];
    char helptext[80];
    uint32_t id;
    list log;
    list users;
    bool named;
    uint8_t mode;
    ENetHost *client;
    ENetPeer *peer;
    ENetPeer *opponent_peer;
    uint8_t connection_count;
    uint8_t active_user;
    lobby_user *opponent;
    bool controllers_created;

    dialog *dialog;

    menu *joinmenu;

    guiframe *frame;
} lobby_local;

typedef struct log_event_t {
    uint8_t color;
    char msg[51];
} log_event;

void lobby_free(scene *scene) {
    lobby_local *local = scene_get_userdata(scene);
    guiframe_free(local->frame);
    list_free(&local->users);
    list_free(&local->log);
    if(local->client) {
        enet_host_destroy(local->client);
    }
    if(local->dialog) {
        dialog_free(local->dialog);
        omf_free(local->dialog);
    }

    omf_free(local);
    scene_set_userdata(scene, local);
}

static int lobby_event(scene *scene, SDL_Event *e) {
    lobby_local *local = scene_get_userdata(scene);
    return guiframe_event(local->frame, e);
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
            } else if(i->type == EVENT_TYPE_ACTION && i->event_data.action == ACT_UP) {
                local->active_user--;
                if(local->active_user >= list_size(&local->users)) {
                    local->active_user = list_size(&local->users) - 1;
                }
            } else {
                guiframe_action(local->frame, p1->event_data.action);
            }
        } while((i = i->next));
    }
    controller_free_chain(p1);
}

void lobby_render_overlay(scene *scene) {
    lobby_local *local = scene_get_userdata(scene);

    char buf[100];
    int longest = 0;

    text_settings font_big;
    text_defaults(&font_big);
    font_big.font = FONT_NET1;
    font_big.cforeground = 3;

    text_settings font_small;
    text_defaults(&font_small);
    font_small.font = FONT_NET2;
    font_small.cforeground = 56;

    if(local->mode > LOBBY_YELL) {
        snprintf(buf, sizeof(buf), "Player");
        text_render(&font_big, TEXT_DEFAULT, 16, 7, 50, 8, buf);

        snprintf(buf, sizeof(buf), "Action");
        text_render(&font_big, TEXT_DEFAULT, 117, 7, 50, 8, buf);

        snprintf(buf, sizeof(buf), "Wn/Loss");
        text_render(&font_small, TEXT_DEFAULT, 200, 8, 50, 6, buf);

        snprintf(buf, sizeof(buf), "Version");
        text_render(&font_small, TEXT_DEFAULT, 240, 8, 50, 6, buf);

        snprintf(buf, sizeof(buf), "%d of %d", local->active_user + 1, list_size(&local->users));
        text_render(&font_small, TEXT_DEFAULT, 284, 8, 40, 6, buf);

        iterator it;
        lobby_user *user;
        list_iter_begin(&local->users, &it);
        int i = 0;
        while((user = list_iter_next(&it)) && i < 8) {
            if(i == local->active_user) {
                font_big.cforeground = 7;
            } else {
                font_big.cforeground = 8;
            }
            text_render(&font_big, TEXT_DEFAULT, 16, 18 + (10 * i), 90, 8, user->name);
            // TODO status
            font_small.cforeground = 40;
            text_render(&font_small, TEXT_DEFAULT, 117, 18 + (10 * i), 60, 6, "available");
            char wins[8];
            snprintf(wins, sizeof(wins), "%d/%d", user->wins, user->losses);
            font_small.cforeground = 56;
            text_render(&font_small, TEXT_DEFAULT, 200, 18 + (10 * i), 50, 6, wins);
            text_render(&font_small, TEXT_DEFAULT, 240, 18 + (10 * i), 50, 6, user->version);
            i++;
        }

        i = 0;
        list_iter_end(&local->log, &it);
        log_event *logmsg;
        while((logmsg = list_iter_prev(&it)) && i < 4) {
            font_big.cforeground = logmsg->color;
            text_render(&font_big, TEXT_DEFAULT, 10, 188 - (8 * i), 300, 8, logmsg->msg);
            i += text_find_line_count(&font_big, 300 / 8, 3, strlen(logmsg->msg), logmsg->msg, &longest);
        }
    } else if(local->mode == LOBBY_YELL) {
        iterator it;
        int i = 0;
        list_iter_end(&local->log, &it);
        log_event *logmsg;
        while((logmsg = list_iter_prev(&it)) && i < 13) {
            font_big.cforeground = logmsg->color;
            text_render(&font_big, TEXT_DEFAULT, 10, 120 - (8 * i), 300, 8, logmsg->msg);
            i += text_find_line_count(&font_big, 300 / 8, 3, strlen(logmsg->msg), logmsg->msg, &longest);
        }
    }

    guiframe_render(local->frame);

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
    serial_write_int8(&ser, PACKET_CHALLENGE << 4 | CHALLENGE_FLAG_CANCEL);

    ENetPacket *packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
    serial_free(&ser);
    enet_peer_send(local->peer, 0, packet);
}

void lobby_do_challenge(component *c, void *userdata) {
    scene *s = userdata;
    lobby_local *local = scene_get_userdata(s);
    if(local->dialog) {
        dialog_free(local->dialog);
        omf_free(local->dialog);
    }
    local->dialog = omf_calloc(1, sizeof(dialog));
    lobby_user *user = list_get(&local->users, local->active_user);
    local->opponent = user;
    char buf[80];

    snprintf(buf, sizeof(buf), "Challenging %s...", user->name);
    dialog_create(local->dialog, DIALOG_STYLE_CANCEL, buf, 72, 60);
    local->dialog->userdata = s;
    local->dialog->clicked = lobby_dialog_cancel_challenge;

    dialog_show(local->dialog, 1);

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
    menu_set_padding(menu, 0);

    lobby_user *user = list_get(&local->users, local->active_user);
    snprintf(local->helptext, sizeof(local->helptext), "Challenge %s?", user->name);
    menu_attach(menu, label_create(&tconf, local->helptext));
    menu_attach(menu, textbutton_create(&tconf, "Yes", NULL, COM_ENABLED, lobby_do_challenge, s));
    menu_attach(menu, textbutton_create(&tconf, "No", NULL, COM_ENABLED, lobby_cancel_challenge, s));

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
        DEBUG("yelled %s", textinput_value(c));

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
    menu_set_padding(menu, 0);

    menu_attach(menu, label_create(&tconf, "Yell:"));
    component *yell_input =
        textinput_create(&tconf, 36,
                         "Yell a message to everybody in the challenge arena.\n\n\n\n\nTo whisper to one player, type "
                         "their name, a ':', and your message.\nPress 'esc' to return to the challenge arena menu.",
                         "");
    menu_attach(menu, yell_input);
    textinput_enable_background(yell_input, 0);
    textinput_set_done_cb(yell_input, lobby_do_yell, s);

    return menu;
}

void lobby_do_whisper(component *c, void *userdata) {
    menu *m = sizer_get_obj(c->parent);
    scene *s = userdata;

    const char *whisper = textinput_value(c);

    if(strlen(whisper) > 0) {
        DEBUG("whispered %s", whisper);

        lobby_local *local = scene_get_userdata(s);
        lobby_user *user = list_get(&local->users, local->active_user);
        DEBUG("active_user is %d", local->active_user);
        DEBUG("whispered %s", textinput_value(c));
        serial ser;
        serial_create(&ser);
        serial_write_int8(&ser, PACKET_WHISPER << 4);
        serial_write_int32(&ser, user->id);
        serial_write(&ser, whisper, strlen(whisper));

        ENetPacket *packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
        serial_free(&ser);

        enet_peer_send(local->peer, 0, packet);

        log_event log;
        log.color = WHISPER_COLOR;
        snprintf(log.msg, sizeof(log.msg), "%s: %s", local->name, whisper);
        list_append(&local->log, &log, sizeof(log));

        m->finished = 1;
        local->mode = LOBBY_MAIN;
        textinput_clear(c);
    }
}

component *lobby_whisper_create(scene *s) {
    lobby_local *local = scene_get_userdata(s);
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
    menu_set_padding(menu, 0);

    menu_attach(menu, label_create(&tconf, "Whisper:"));
    lobby_user *user = list_get(&local->users, local->active_user);
    snprintf(local->helptext, sizeof(local->helptext), "Whisper a message to %s. Press enter when done, esc to abort.",
             user->name);
    component *whisper_input = textinput_create(&tconf, 36, local->helptext, "");
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

        nat_ctx *nat = omf_calloc(1, sizeof(nat_ctx));
        nat_create(nat);

        ENetAddress address;
        ENetAddress lobby_address;
        address.host = ENET_HOST_ANY;
        address.port = settings_get()->net.net_listen_port_start;

        DEBUG("attempting to bind to port %d", address.port);

        if(address.port == 0) {
            address.port = rand_int(65535 - 1024) + 1024;
        }

        // Set up host
        local->controllers_created = 0;
        int randtries = 0;

        int end_port = settings_get()->net.net_listen_port_end;
        if(!end_port) {
            end_port = 65535;
        }
        while(local->client == NULL) {
            local->client = enet_host_create(&address, 2, 2, 0, 0);
            if(local->client == NULL) {
                DEBUG("requested port %d unavailable, trying ports %d to %d", address.port,
                      settings_get()->net.net_listen_port_start, end_port);
                if(settings_get()->net.net_listen_port_start == 0) {
                    address.port = rand_int(65535 - 1024) + 1024;
                    randtries++;
                    if(randtries > 10) {
                        DEBUG("Failed to initialize ENet server, could not allocate random port");
                        return;
                    }
                } else {
                    address.port++;
                    if(address.port > end_port) {
                        DEBUG("Failed to initialize ENet server, port range exhausted");
                        return;
                    }
                    randtries++;
                    if(randtries > 10) {
                        DEBUG("Failed to initialize ENet server, could not allocate port between %d and %d after 10 "
                              "tries",
                              settings_get()->net.net_listen_port_start, end_port);
                        return;
                    }
                }
            }
        }

        DEBUG("bound to port %d", address.port);

        int ext_port = settings_get()->net.net_ext_port_start;
        if(ext_port == 0) {
            // try to use the internal port first
            ext_port = address.port;
        }
        randtries = 0;

        if(nat->type != NAT_TYPE_NONE) {
            while(!nat_create_mapping(nat, address.port, ext_port)) {
                if(settings_get()->net.net_ext_port_start == 0) {
                    ext_port = rand_int(65535 - 1024) + 1024;
                    randtries++;
                    if(randtries > 10) {
                        ext_port = 0;
                        break;
                    }
                } else {
                    ext_port++;
                    if(ext_port > settings_get()->net.net_ext_port_end) {
                        ext_port = 0;
                        break;
                    }
                }
            }
        }
        enet_socket_set_option(local->client->socket, ENET_SOCKOPT_REUSEADDR, 1);

        ENetEvent event;
        ENetPeer *peer = NULL;
        enet_address_set_host(&lobby_address, "lobby.openomf.org");
        // enet_address_set_host(&address, "127.0.0.1");
        lobby_address.port = 2098;
        DEBUG("server address is %d", lobby_address.host);
        /* Initiate the connection, allocating the two channels 0 and 1. */
        local->peer = enet_host_connect(local->client, &lobby_address, 2, 0);

        if(local->peer == NULL) {
            DEBUG("No available peers for initiating an ENet connection.\n");
        }
        /* Wait up to 5 seconds for the connection attempt to succeed. */
        else if(enet_host_service(local->client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
            DEBUG("Connection to server succeeded.");

            DEBUG("local peer connect id %d", local->peer->connectID);
            DEBUG("remote peer connect id %d", event.peer->connectID);

            event.peer->data = nat;
            strncpy_or_truncate(local->name, textinput_value(c), sizeof(local->name));

            char version[15];
            // TODO support git version when not on a tag
            snprintf(version, 14, "%d.%d.%d", V_MAJOR, V_MINOR, V_PATCH);
            version[14] = 0;
            serial ser;
            serial_create(&ser);
            serial_write_int8(&ser, PACKET_JOIN << 4);
            // if we mapped an external port, send it to the server
            if(nat->type != NAT_TYPE_NONE && ext_port) {
                serial_write_int16(&ser, nat->ext_port);
            } else {
                serial_write_int16(&ser, address.port);
            }
            serial_write_int8(&ser, strlen(version));
            serial_write(&ser, version, strlen(version));
            const char *name = textinput_value(c);
            serial_write(&ser, name, strlen(name));

            settings_get()->net.net_username = omf_strdup(name);

            ENetPacket *packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
            serial_free(&ser);

            enet_peer_send(local->peer, 0, packet);

            local->joinmenu = sizer_get_obj(c->parent);
            // menu *m = sizer_get_obj(c->parent);
            // m->finished = 1;
            // local->mode = LOBBY_MAIN;
        } else {
            /* Either the 5 seconds are up or a disconnect event was */
            /* received. Reset the peer in the event the 5 seconds   */
            /* had run out without any significant event.            */
            enet_peer_reset(peer);

            DEBUG("Connection to server failed.");
        }
    }
}

void lobby_dialog_cancel_connect(dialog *dlg, dialog_result result) {
    dialog_show(dlg, 0);
    scene *s = dlg->userdata;
    lobby_local *local = scene_get_userdata(s);
    serial ser;
    serial_create(&ser);
    serial_write_int8(&ser, PACKET_CHALLENGE << 4 | CHALLENGE_FLAG_CANCEL);

    ENetPacket *packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
    serial_free(&ser);
    enet_peer_send(local->peer, 0, packet);
    local->opponent_peer = NULL;
}

void lobby_try_connect(void *scenedata, void *userdata) {
    scene *s = scenedata;
    lobby_local *local = scene_get_userdata(s);
    if(!local->opponent_peer) {
        DEBUG("doing scheduled outbound connection to %d.%d.%d.%d port %d", local->opponent->address.host & 0xFF,
              (local->opponent->address.host >> 8) & 0xFF, (local->opponent->address.host >> 16) & 0xF,
              (local->opponent->address.host >> 24) & 0xFF, local->opponent->address.port);
        local->opponent_peer = enet_host_connect(local->client, &local->opponent->address, 2, 0);
        enet_peer_timeout(local->opponent_peer, 4, 1000, 1000);
    }
}

void lobby_dialog_accept_challenge(dialog *dlg, dialog_result result) {
    dialog_show(dlg, 0);

    scene *s = dlg->userdata;
    lobby_local *local = scene_get_userdata(s);
    serial ser;
    serial_create(&ser);
    uint8_t flag = result == DIALOG_RESULT_YES_OK ? CHALLENGE_FLAG_ACCEPT : CHALLENGE_FLAG_REJECT;
    serial_write_int8(&ser, (PACKET_CHALLENGE << 4) | flag);

    ENetPacket *packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
    serial_free(&ser);
    enet_peer_send(local->peer, 0, packet);

    if(result == DIALOG_RESULT_YES_OK) {
        dialog_free(local->dialog);
        dialog_create(local->dialog, DIALOG_STYLE_CANCEL, "Establishing connection...", 72, 60);

        local->connection_count = 0;
        ticktimer_add(&s->tick_timer, 500, lobby_try_connect, NULL);

        dialog_show(local->dialog, 1);
        local->dialog->userdata = s;
        local->dialog->clicked = lobby_dialog_cancel_connect;
    }
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
    menu_set_padding(menu, 0);

    menu_attach(menu, label_create(&tconf, "Exit the Challenge Arena?"));
    menu_attach(menu, textbutton_create(&tconf, "Yes", NULL, COM_ENABLED, lobby_do_exit, s));
    menu_attach(menu, textbutton_create(&tconf, "No", NULL, COM_ENABLED, lobby_refuse_exit, s));

    return menu;
}

void lobby_exit(component *c, void *userdata) {
    scene *s = userdata;
    menu_set_submenu(c->parent, lobby_exit_create(s));
}

void lobby_dialog_close(dialog *dlg, dialog_result result) {
    dialog_show(dlg, 0);
}

void lobby_tick(scene *scene, int paused) {
    lobby_local *local = scene_get_userdata(scene);

    game_state *gs = scene->gs;
    ENetEvent event;
    serial ser;
    while(local->client && !local->controllers_created && enet_host_service(local->client, &event, 0) > 0) {
        switch(event.type) {
            case ENET_EVENT_TYPE_NONE:
                break;
            case ENET_EVENT_TYPE_CONNECT:
                DEBUG("A new client connected from %x:%u.", event.peer->address.host, event.peer->address.port);

                /* Store any relevant client information here. */
                event.peer->data = NULL;
                DEBUG("new peer was %d, server peer was %d, opponent peer was %d", event.peer, local->peer,
                      local->opponent_peer);

                if(local->opponent_peer && event.peer->address.host == local->opponent->address.host) {
                    DEBUG("connected to peer outbound!");
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

                    controller *player1_ctrl, *player2_ctrl;
                    keyboard_keys *keys;
                    game_player *p1 = game_state_get_player(gs, 0);
                    game_player *p2 = game_state_get_player(gs, 1);
                    gs->net_mode = NET_MODE_LOBBY;

                    // force the speed to 3
                    game_state_set_speed(gs, 10);

                    p1->pilot->har_id = HAR_JAGUAR;
                    p1->pilot->pilot_id = 0;
                    p2->pilot->har_id = HAR_JAGUAR;
                    p2->pilot->pilot_id = 0;

                    player1_ctrl = omf_calloc(1, sizeof(controller));
                    controller_init(player1_ctrl, gs);
                    player1_ctrl->har_obj_id = p1->har_obj_id;
                    player2_ctrl = omf_calloc(1, sizeof(controller));
                    controller_init(player2_ctrl, gs);
                    player2_ctrl->har_obj_id = p2->har_obj_id;

                    // Player 1 controller -- Keyboard
                    settings_keyboard *k = &settings_get()->keys;
                    keys = omf_calloc(1, sizeof(keyboard_keys));
                    keys->jump_up = SDL_GetScancodeFromName(k->key1_jump_up);
                    keys->jump_right = SDL_GetScancodeFromName(k->key1_jump_right);
                    keys->walk_right = SDL_GetScancodeFromName(k->key1_walk_right);
                    keys->duck_forward = SDL_GetScancodeFromName(k->key1_duck_forward);
                    keys->duck = SDL_GetScancodeFromName(k->key1_duck);
                    keys->duck_back = SDL_GetScancodeFromName(k->key1_duck_back);
                    keys->walk_back = SDL_GetScancodeFromName(k->key1_walk_back);
                    keys->jump_left = SDL_GetScancodeFromName(k->key1_jump_left);
                    keys->punch = SDL_GetScancodeFromName(k->key1_punch);
                    keys->kick = SDL_GetScancodeFromName(k->key1_kick);
                    keyboard_create(player1_ctrl, keys, 0);
                    game_player_set_ctrl(p1, player1_ctrl);

                    // Player 2 controller -- Network
                    net_controller_create(player2_ctrl, local->client, event.peer, local->peer, ROLE_SERVER);
                    game_player_set_ctrl(p2, player2_ctrl);
                    game_player_set_selectable(p2, 1);

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
                DEBUG("A packet of length %u with control byte %d was received on channel %u.",
                      event.packet->dataLength, control_byte, event.channelID);
                switch(control_byte >> 4) {
                    case PACKET_PRESENCE: {
                        lobby_user user;
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
                        uint8_t version_len = serial_read_int8(&ser);
                        if(version_len < 15) {
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
                                while((u = list_iter_next(&it)) && !found) {
                                    if(u->id == user.id) {
                                        found = true;
                                        u->wins = user.wins;
                                        u->losses = user.losses;
                                        u->address.host = user.address.host;
                                        u->port = user.port;
                                        u->ext_port = user.ext_port;
                                        u->address.port = user.address.port;
                                        memcpy(user.version, u->version, sizeof(user.version));
                                    }
                                }

                                if(!found) {
                                    list_append(&local->users, &user, sizeof(lobby_user));
                                    if(control_byte & 0x8) {
                                        log_event log;
                                        log.color = JOIN_COLOR;
                                        snprintf(log.msg, sizeof(log.msg), "%s has entered the Arena", user.name),
                                            list_append(&local->log, &log, sizeof(log));
                                    }
                                }
                            }
                        }
                    } break;
                    case PACKET_JOIN:
                        if(event.peer == local->peer) {
                            switch(control_byte & 0xf) {
                                case 0:
                                    local->id = serial_read_uint32(&ser);
                                    DEBUG("successfully joined lobby and assigned ID %d", local->id);
                                    local->joinmenu->finished = 1;
                                    local->mode = LOBBY_MAIN;
                                    break;
                                default:
                                    // TODO something went wrong show a dialog box
                                    break;
                            }
                        } else if(!local->opponent_peer && event.peer->address.host == local->opponent->address.host) {
                            DEBUG("connected to peer inbound!");
                            local->opponent_peer = event.peer;

                            // signal the server we're connected
                            serial_create(&ser);
                            serial_write_int8(&ser, PACKET_CONNECTED << 4);
                            ENetPacket *packet =
                                enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
                            enet_peer_send(local->peer, 0, packet);
                            serial_free(&ser);

                            controller *player1_ctrl, *player2_ctrl;
                            keyboard_keys *keys;
                            game_player *p1 = game_state_get_player(gs, 0);
                            game_player *p2 = game_state_get_player(gs, 1);
                            gs->net_mode = NET_MODE_LOBBY;

                            // force the speed to 3
                            game_state_set_speed(gs, 10);

                            p1->pilot->har_id = HAR_JAGUAR;
                            p1->pilot->pilot_id = 0;
                            p2->pilot->har_id = HAR_JAGUAR;
                            p2->pilot->pilot_id = 0;

                            player1_ctrl = omf_calloc(1, sizeof(controller));
                            controller_init(player1_ctrl, gs);
                            player1_ctrl->har_obj_id = p1->har_obj_id;
                            player2_ctrl = omf_calloc(1, sizeof(controller));
                            controller_init(player2_ctrl, gs);
                            player2_ctrl->har_obj_id = p2->har_obj_id;

                            // Player 1 controller -- Network
                            net_controller_create(player1_ctrl, local->client, event.peer, local->peer, ROLE_CLIENT);
                            game_player_set_ctrl(p1, player1_ctrl);

                            // Player 2 controller -- Keyboard
                            settings_keyboard *k = &settings_get()->keys;
                            keys = omf_calloc(1, sizeof(keyboard_keys));
                            keys->jump_up = SDL_GetScancodeFromName(k->key1_jump_up);
                            keys->jump_right = SDL_GetScancodeFromName(k->key1_jump_right);
                            keys->walk_right = SDL_GetScancodeFromName(k->key1_walk_right);
                            keys->duck_forward = SDL_GetScancodeFromName(k->key1_duck_forward);
                            keys->duck = SDL_GetScancodeFromName(k->key1_duck);
                            keys->duck_back = SDL_GetScancodeFromName(k->key1_duck_back);
                            keys->walk_back = SDL_GetScancodeFromName(k->key1_walk_back);
                            keys->jump_left = SDL_GetScancodeFromName(k->key1_jump_left);
                            keys->punch = SDL_GetScancodeFromName(k->key1_punch);
                            keys->kick = SDL_GetScancodeFromName(k->key1_kick);
                            keyboard_create(player2_ctrl, keys, 0);
                            game_player_set_ctrl(p2, player2_ctrl);
                            game_player_set_selectable(p2, 1);

                            chr_score_set_difficulty(game_player_get_score(game_state_get_player(gs, 0)),
                                                     AI_DIFFICULTY_CHAMPION);
                            chr_score_set_difficulty(game_player_get_score(game_state_get_player(gs, 1)),
                                                     AI_DIFFICULTY_CHAMPION);

                            local->controllers_created = true;

                        } else {
                            DEBUG("opponent peer %d, host %d %d", local->opponent_peer, event.peer->address.host,
                                  local->opponent->address.host);
                        }
                        break;
                    case PACKET_YELL: {
                        log_event log;
                        log.color = YELL_COLOR;
                        strncpy_or_truncate(log.msg, (char *)event.packet->data + 1, sizeof(log.msg));
                        list_append(&local->log, &log, sizeof(log));
                    } break;
                    case PACKET_WHISPER: {
                        log_event log;
                        log.color = WHISPER_COLOR;
                        strncpy_or_truncate(log.msg, (char *)event.packet->data + 1, sizeof(log.msg));
                        list_append(&local->log, &log, sizeof(log));
                    } break;
                    case PACKET_DISCONNECT: {
                        uint32_t connect_id = serial_read_uint32(&ser);
                        iterator it;
                        list_iter_begin(&local->users, &it);
                        lobby_user *user;
                        while((user = list_iter_next(&it))) {
                            if(user->id == connect_id) {
                                log_event log;
                                log.color = LEAVE_COLOR;
                                snprintf(log.msg, sizeof(log.msg), "%s has left the Arena", user->name);
                                list_append(&local->log, &log, sizeof(log));
                                if(local->opponent == user) {
                                    local->opponent = NULL;
                                    // any existing dialogs were for this user
                                    if(local->dialog) {
                                        dialog_free(local->dialog);
                                        omf_free(local->dialog);
                                    }
                                    // TODO do we need to pop a dialog saying "user disconnected"?
                                }
                                list_delete(&local->users, &it);
                                break;
                            }
                        }
                    } break;
                    case PACKET_CHALLENGE: {
                        switch(control_byte & 0xf) {
                            case 0: {
                                uint32_t connect_id = serial_read_uint32(&ser);

                                DEBUG("got challenge from %d, we are %d", connect_id, local->id);
                                iterator it;
                                list_iter_begin(&local->users, &it);
                                lobby_user *user;
                                bool found = false;
                                while((user = list_iter_next(&it)) && !found) {
                                    if(user->id == connect_id) {
                                        found = true;
                                        break;
                                    }
                                }

                                if(found) {
                                    local->opponent = user;
                                    local->dialog = omf_calloc(1, sizeof(dialog));
                                    char buf[80];

                                    snprintf(buf, sizeof(buf), "Accept challenge from %s?", user->name);
                                    dialog_create(local->dialog, DIALOG_STYLE_YES_NO, buf, 72, 60);
                                    local->dialog->userdata = scene;
                                    local->dialog->clicked = lobby_dialog_accept_challenge;
                                    dialog_show(local->dialog, 1);
                                } else {
                                    DEBUG("unable to find user with id %d", connect_id);
                                }
                            } break;
                            case CHALLENGE_FLAG_ACCEPT:
                                // peer accepted, try to connect to them
                                if(local->dialog) {
                                    dialog_show(local->dialog, 0);
                                    dialog_free(local->dialog);
                                } else {
                                    local->dialog = omf_calloc(1, sizeof(dialog));
                                }
                                dialog_create(local->dialog, DIALOG_STYLE_CANCEL, "Establishing connection...", 72, 60);

                                // try to connect immediately
                                local->opponent_peer =
                                    enet_host_connect(local->client, &local->opponent->address, 2, 0);

                                DEBUG("doing immediate outbound connection to %d.%d.%d.%d port %d",
                                      local->opponent->address.host & 0xFF, (local->opponent->address.host >> 8) & 0xFF,
                                      (local->opponent->address.host >> 16) & 0xF,
                                      (local->opponent->address.host >> 24) & 0xFF, local->opponent->address.port);
                                enet_peer_timeout(local->opponent_peer, 4, 1000, 1000);
                                local->connection_count = 0;

                                dialog_show(local->dialog, 1);
                                local->dialog->userdata = scene;
                                local->dialog->clicked = lobby_dialog_cancel_connect;
                                break;
                            case CHALLENGE_FLAG_REJECT:
                                // peer accepted, try to connect to them
                                if(local->dialog) {
                                    dialog_show(local->dialog, 0);
                                    dialog_free(local->dialog);
                                } else {
                                    local->dialog = omf_calloc(1, sizeof(dialog));
                                }
                                dialog_create(local->dialog, DIALOG_STYLE_OK, "Challenge rejected.", 72, 60);
                                dialog_show(local->dialog, 1);
                                local->dialog->userdata = scene;
                                local->dialog->clicked = lobby_dialog_close;
                                break;
                            case CHALLENGE_FLAG_CANCEL:
                                // peer accepted, try to connect to them
                                if(local->dialog) {
                                    dialog_show(local->dialog, 0);
                                    dialog_free(local->dialog);
                                } else {
                                    local->dialog = omf_calloc(1, sizeof(dialog));
                                }
                                dialog_create(local->dialog, DIALOG_STYLE_OK, "Challenge cancelled by peer.", 72, 60);
                                dialog_show(local->dialog, 1);
                                local->dialog->userdata = scene;
                                local->dialog->clicked = lobby_dialog_close;
                                local->opponent_peer = NULL;
                                break;
                        }
                    } break;
                    default:
                        DEBUG("unknown packet of type %d received", event.packet->data[0] >> 4);
                        break;
                }
                serial_free(&ser);
                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy(event.packet);

                break;
            case ENET_EVENT_TYPE_DISCONNECT:

                if(event.peer == local->opponent_peer) {
                    local->connection_count++;
                    DEBUG("outbound peer connection failed");
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
    guiframe_tick(local->frame);

    component *c = guiframe_get_root(local->frame);
    if((c = menu_get_submenu(c)) && menu_is_finished(c)) {
        local->mode = LOBBY_MAIN;
    }

    game_player *p1 = game_state_get_player(gs, 0);
    controller *c1 = game_player_get_ctrl(p1);
    if(c1->type == CTRL_TYPE_NETWORK && net_controller_ready(c1)) {
        DEBUG("network peer is ready, tick offset is %d and rtt is %d", net_controller_tick_offset(c1), c1->rtt);
        local->client = NULL;
        game_state_set_next(gs, SCENE_MELEE);
    }

    game_player *p2 = game_state_get_player(gs, 1);
    controller *c2 = game_player_get_ctrl(p2);
    if(c2->type == CTRL_TYPE_NETWORK && net_controller_ready(c2) == 1) {
        DEBUG("network peer is ready, tick offset is %d and rtt is %d", net_controller_tick_offset(c2), c2->rtt);
        local->client = NULL;
        game_state_set_next(gs, SCENE_MELEE);
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
    local->mode = LOBBY_STARTING;
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
    menu_set_padding(menu, 6);

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

    // check if there's already a net controller provisioned
    // and harvest the information from it, if possible
    if(game_state_get_player(scene->gs, 0)->ctrl->type == CTRL_TYPE_NETWORK) {
        local->peer = net_controller_get_lobby_connection(game_state_get_player(scene->gs, 0)->ctrl);
        local->client = net_controller_get_host(game_state_get_player(scene->gs, 0)->ctrl);
        local->mode = LOBBY_MAIN;
    } else if(game_state_get_player(scene->gs, 1)->ctrl->type == CTRL_TYPE_NETWORK) {
        local->peer = net_controller_get_lobby_connection(game_state_get_player(scene->gs, 1)->ctrl);
        local->client = net_controller_get_host(game_state_get_player(scene->gs, 1)->ctrl);
        local->mode = LOBBY_MAIN;
    }

    // Cleanups and resets
    for(int i = 0; i < 2; i++) {
        // destroy any leftover controllers
        game_player *player = game_state_get_player(scene->gs, i);
        game_player_set_ctrl(player, NULL);
    }
    reconfigure_controller(scene->gs);

    local->frame = guiframe_create(9, 128, 300, 12);
    guiframe_set_root(local->frame, menu);
    guiframe_layout(local->frame);

    if(local->mode == LOBBY_STARTING) {

        component *name_menu = menu_create(11);
        menu_set_horizontal(name_menu, true);
        menu_set_background(name_menu, false);
        menu_set_padding(menu, 0);

        menu_attach(name_menu, label_create(&tconf, "Enter your name:"));
        // TODO pull the last used name from settings
        component *name_input = textinput_create(&tconf, 14, "", settings_get()->net.net_username);
        textinput_enable_background(name_input, 0);
        textinput_set_done_cb(name_input, lobby_entered_name, scene);
        menu_attach(name_menu, name_input);

        menu_set_submenu(menu, name_menu);
    } else {
        // TODO send the server a REFRESH command so we can get the userlist, our username, etc
        serial ser;
        serial_create(&ser);
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
