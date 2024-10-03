#include "game/gui/frame.h"
#include "game/protos/scene.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/miscmath.h"
#include "video/video.h"

#include "game/gui/gui.h"

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
    PACKET_COUNT
};

typedef struct lobby_local_t {
    char name[16];
    list log;
    list users;
    bool named;
    uint8_t mode;
    ENetHost *client;
    ENetPeer *peer;
    uint8_t active_user;

    guiframe *frame;
} lobby_local;

typedef struct log_event_t {
    uint8_t color;
    char msg[51];
} log_event;

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
            if(i->type == EVENT_TYPE_ACTION && i->event_data.action == ACT_DOWN) {
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

    if(local->mode > LOBBY_YELL) {
        snprintf(buf, sizeof(buf), "Player");
        font_render(&font_net1, buf, 16, 7, 3);

        snprintf(buf, sizeof(buf), "Action");
        font_render(&font_net1, buf, 117, 7, 3);

        snprintf(buf, sizeof(buf), "Wn/Loss");
        font_render(&font_net2, buf, 200, 8, 56);

        snprintf(buf, sizeof(buf), "Version");
        font_render(&font_net2, buf, 240, 8, 56);

        snprintf(buf, sizeof(buf), "1 of 0");
        font_render(&font_net2, buf, 284, 8, 56);

        iterator it;
        char *username;
        list_iter_begin(&local->users, &it);
        int i = 0;
        local->active_user = min2(local->active_user, list_size(&local->users) - 1);
        while((username = list_iter_next(&it)) && i < 8) {
            if(i == local->active_user) {
                font_render(&font_net1, username, 16, 18 + (10 * i), 7);
            } else {
                font_render(&font_net1, username, 16, 18 + (10 * i), 8);
            }
            font_render(&font_net2, "available", 117, 18 + (10 * i), 40);
            font_render(&font_net2, "0/0", 200, 18 + (10 * i), 56);
            font_render(&font_net2, "OpenOMF 0.9.6-git", 240, 18 + (10 * i), 56);
            i++;
        }

        i = 0;
        list_iter_end(&local->log, &it);
        log_event *logmsg;
        int length = list_size(&local->log);
        if(length > 4) {
            length = 4;
        }
        while((logmsg = list_iter_next(&it)) && i < 4) {
            font_render(&font_net1, logmsg->msg, 10, 196 - (8 * length) + (8 * i), logmsg->color);
            i++;
        }
    } else if(local->mode == LOBBY_YELL) {
        iterator it;
        int i = 0;
        list_iter_end(&local->log, &it);
        log_event *logmsg;
        int length = list_size(&local->log);
        if(length > 13) {
            length = 13;
        }
        while((logmsg = list_iter_next(&it)) && i < 13) {
            font_render(&font_net1, logmsg->msg, 10, 130 - (8 * length) + (8 * i), logmsg->color);
            i++;
        }
    }

    guiframe_render(local->frame);
}

void lobby_challenge(component *c, void *userdata) {
}

void lobby_do_yell(component *c, void *userdata) {
    scene *scene = userdata;
    lobby_local *local = scene_get_userdata(scene);

    // menu *m = sizer_get_obj(c->parent);
    char *yell = textinput_value(c);

    if(strlen(yell) > 0) {
        DEBUG("yelled %s", textinput_value(c));

        char yell_packet[50];
        snprintf(yell_packet, sizeof(yell_packet), "\2%s", yell);

        ENetPacket *packet = enet_packet_create(yell_packet, strlen(yell_packet) + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(local->peer, 0, packet);
        textinput_clear(c);
    }
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
        textinput_create(&tconf, "Yell:",
                         "Yell a message to everybody in the challenge arena.\n\n\n\n\nTo whisper to one player, type "
                         "their name, a ':', and your message.\nPress 'esc' to return to the challenge arena menu.",
                         "");
    textinput_set_max_chars(yell_input, 36);
    menu_attach(menu, yell_input);
    textinput_enable_background(yell_input, 0);
    textinput_set_done_cb(yell_input, lobby_do_yell, s);

    return menu;
}

void lobby_do_whisper(component *c, void *userdata) {
    menu *m = sizer_get_obj(c->parent);
    scene *s = userdata;
    lobby_local *local = scene_get_userdata(s);
    DEBUG("whispered %s", textinput_value(c));
    // TODO get the message and send/log it from the textinput component 'c'
    m->finished = 1;
    local->mode = LOBBY_MAIN;
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
    textinput_set_max_chars(whisper_input, 36);
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
}

void lobby_do_exit(component *c, void *userdata) {
    scene *scene = userdata;
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
    scene *scene = userdata;
    lobby_local *local = scene_get_userdata(scene);

    local->client = enet_host_create(NULL /* create a client host */, 1 /* only allow 1 outgoing connection */,
                                     2 /* allow up 2 channels to be used, 0 and 1 */,
                                     0 /* assume any amount of incoming bandwidth */,
                                     0 /* assume any amount of outgoing bandwidth */);

    ENetAddress address;
    ENetEvent event;
    ENetPeer *peer = NULL;
    if(local->client == NULL) {
        DEBUG("An error occurred while trying to create an ENet client host.\n");
    }
    enet_address_set_host(&address, "127.0.0.1");
    address.port = 2098;
    /* Initiate the connection, allocating the two channels 0 and 1. */
    local->peer = enet_host_connect(local->client, &address, 2, 0);

    if(local->peer == NULL) {
        DEBUG("No available peers for initiating an ENet connection.\n");
    }
    /* Wait up to 5 seconds for the connection attempt to succeed. */
    else if(enet_host_service(local->client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        DEBUG("Connection to server succeeded.");

        event.peer->data = "Client information";
        strncpy(local->name, textinput_value(c), sizeof(local->name));

        char name_packet[20];
        snprintf(name_packet, sizeof(name_packet), "\1%s", textinput_value(c));

        ENetPacket *packet = enet_packet_create(name_packet, strlen(name_packet) + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(local->peer, 0, packet);

        menu *m = sizer_get_obj(c->parent);
        m->finished = 1;
        local->mode = LOBBY_MAIN;
    } else {
        /* Either the 5 seconds are up or a disconnect event was */
        /* received. Reset the peer in the event the 5 seconds   */
        /* had run out without any significant event.            */
        enet_peer_reset(peer);

        DEBUG("Connection to server failed.");
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
    ENetEvent event;
    while(local->client && enet_host_service(local->client, &event, 0) > 0) {
        switch(event.type) {
            case ENET_EVENT_TYPE_NONE:
                break;
            case ENET_EVENT_TYPE_CONNECT:
                DEBUG("A new client connected from %x:%u.", event.peer->address.host, event.peer->address.port);

                /* Store any relevant client information here. */
                event.peer->data = "Client information";

                break;
            case ENET_EVENT_TYPE_RECEIVE:
                DEBUG("A packet of length %u containing %s was received from %s on channel %u.",
                      event.packet->dataLength, event.packet->data, (char *)event.peer->data, event.channelID);
                switch(event.packet->data[0]) {
                    case PACKET_PRESENCE:
                        list_append(&local->users, event.packet->data + 1, event.packet->dataLength - 1);
                        break;
                    case PACKET_JOIN:
                        list_append(&local->users, event.packet->data + 1, event.packet->dataLength - 1);
                        log_event log;
                        log.color = JOIN_COLOR;
                        snprintf(log.msg, sizeof(log.msg), "%s has entered the Arena", event.packet->data + 1),
                            list_append(&local->log, &log, sizeof(log));
                        break;
                    case PACKET_YELL: {
                        log_event log;
                        log.color = YELL_COLOR;
                        strncpy(log.msg, (char *)event.packet->data + 1, sizeof(log.msg));
                        list_append(&local->log, &log, sizeof(log));
                    } break;
                    case PACKET_WHISPER: {
                        log_event log;
                        log.color = WHISPER_COLOR;
                        strncpy(log.msg, (char *)event.packet->data + 1, sizeof(log.msg));
                        list_append(&local->log, &log, sizeof(log));
                    } break;
                    case PACKET_DISCONNECT: {
                        char *name = (char *)(event.packet->data + 1);
                        iterator it;
                        char *username;
                        list_iter_begin(&local->users, &it);
                        bool found = false;
                        while((username = list_iter_next(&it))) {
                            if(strncmp(name, username, 16) == 0) {
                                list_delete(&local->users, &it);
                                found = true;
                                break;
                            }
                        }
                        if(found) {
                            log_event log;
                            log.color = LEAVE_COLOR;
                            snprintf(log.msg, sizeof(log.msg), "%s has left the Arena", name);
                            list_append(&local->log, &log, sizeof(log));
                        }
                    } break;
                    default:
                        DEBUG("unknown packet of type %d received", event.packet->data[0]);
                        break;
                }
                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy(event.packet);

                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                DEBUG("%s disconnected.\n", (char *)event.peer->data);

                /* Reset the peer's client information. */

                event.peer->data = NULL;
        }
    }
    guiframe_tick(local->frame);

    component *c = guiframe_get_root(local->frame);
    if((c = menu_get_submenu(c)) && menu_is_finished(c)) {
        local->mode = LOBBY_MAIN;
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
    textinput_set_max_chars(name_input, 14);
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
