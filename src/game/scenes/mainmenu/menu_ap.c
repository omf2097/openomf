#include "game/scenes/mainmenu/menu_ap.h"

#include "archipelago/apconnect.h"
#include "game/game_state.h"
#include "game/gui/gui.h"
#include "game/gui/label.h"
#include "game/gui/text/text.h"
#include "game/protos/scene.h"
#include "game/utils/settings.h"
#include "resources/ids.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/log.h"

#define AP_SERVER_MAX_LEN 64
#define AP_SLOT_MAX_LEN   32
#define AP_PASS_MAX_LEN   32

typedef struct {
    component *server_input;
    component *slot_input;
    component *pass_input;
    component *status_label;
    component *connect_button;
    component *cancel_button;
    ap_connection_status_t last_status;
    scene     *s;
} ap_menu_data;

static void menu_ap_free(component *c) {
    ap_menu_data *local = menu_get_userdata(c);
    omf_free(local);
    menu_set_userdata(c, NULL);
}

static void menu_ap_update_status(ap_menu_data *local, ap_connection_status_t status) {
    if(status == local->last_status) return;
    local->last_status = status;
    switch(status) {
        case APCONN_NOT_CONNECTED:
            label_set_text(local->status_label, "");
            break;
        case APCONN_CONNECTING:
            label_set_text(local->status_label, "CONNECTING...");
            label_set_text_color(local->status_label, TEXT_YELLOW);
            break;
        case APCONN_READY:
            label_set_text(local->status_label, "CONNECTED!");
            label_set_text_color(local->status_label, TEXT_BRIGHT_GREEN);
            break;
        case APCONN_FATAL_ERROR:
            label_set_text(local->status_label, "CONNECTION FAILED");
            label_set_text_color(local->status_label, 0xF6);
            break;
    }
}

static void menu_ap_tick(component *c) {
    ap_menu_data *local = menu_get_userdata(c);
    ap_connection_status_t status = Archipelago_ConnectionStatus();

    menu_ap_update_status(local, status);

    if(status == APCONN_READY) {
        // Persist settings
        settings_archipelago *ap_cfg = &settings_get()->archipelago;
        omf_free(ap_cfg->ap_server);
        omf_free(ap_cfg->ap_slot);
        omf_free(ap_cfg->ap_password);
        ap_cfg->ap_server   = omf_strdup(textinput_value(local->server_input));
        ap_cfg->ap_slot     = omf_strdup(textinput_value(local->slot_input));
        ap_cfg->ap_password = omf_strdup(textinput_value(local->pass_input));
        settings_save();

        // Transition to tournament play
        game_state_set_next(local->s->gs, SCENE_MECHLAB);
    } else if(status == APCONN_FATAL_ERROR) {
        log_error("Archipelago: connection refused or fatal error");
        Archipelago_Disconnect();
        // Re-enable inputs so player can retry
        component_disable(local->connect_button, 0);
        component_disable(local->server_input, 0);
        component_disable(local->slot_input, 0);
        component_disable(local->pass_input, 0);
        menu_select(c, local->connect_button);
        // Reset last_status so the label updates again on next tick after disconnect
        local->last_status = APCONN_FATAL_ERROR;
    }
}

static void menu_ap_connect(component *c, void *userdata) {
    (void)userdata;
    ap_menu_data *local = menu_get_userdata(c->parent);

    const char *server   = textinput_value(local->server_input);
    const char *slot     = textinput_value(local->slot_input);
    const char *password = textinput_value(local->pass_input);

    if(!server || server[0] == '\0') {
        log_error("Archipelago: server address is empty");
        return;
    }
    if(!slot || slot[0] == '\0') {
        log_error("Archipelago: slot name is empty");
        return;
    }

    log_info("Archipelago: connecting to %s as %s", server, slot);
    Archipelago_Connect(server, slot, password);

    component_disable(local->connect_button, 1);
    component_disable(local->server_input, 1);
    component_disable(local->slot_input, 1);
    component_disable(local->pass_input, 1);
    menu_select(c->parent, local->cancel_button);
}

static void menu_ap_cancel(component *c, void *userdata) {
    (void)userdata;
    Archipelago_Disconnect();
    menu *m = sizer_get_obj(c->parent);
    m->finished = 1;
}

component *menu_ap_create(scene *s) {
    ap_menu_data *local = omf_calloc(1, sizeof(ap_menu_data));
    local->s = s;

    settings_archipelago *ap_cfg = &settings_get()->archipelago;

    component *menu = menu_create();
    menu_attach(menu, label_create_title("ARCHIPELAGO"));

    local->server_input = textinput_create(AP_SERVER_MAX_LEN,
        "Archipelago server address and port (host:port).",
        ap_cfg->ap_server ? ap_cfg->ap_server : "localhost:38281");
    local->slot_input   = textinput_create(AP_SLOT_MAX_LEN,
        "Your slot name (player name from your YAML).",
        ap_cfg->ap_slot ? ap_cfg->ap_slot : "");
    local->pass_input   = textinput_create(AP_PASS_MAX_LEN,
        "Room password (leave blank if none).",
        ap_cfg->ap_password ? ap_cfg->ap_password : "");

    textinput_set_font(local->server_input, FONT_BIG);
    textinput_set_font(local->slot_input,   FONT_BIG);
    textinput_set_font(local->pass_input,   FONT_BIG);

    local->status_label   = label_create("");
    label_set_text_horizontal_align(local->status_label, TEXT_ALIGN_CENTER);
    component_set_size_hints(local->status_label, -1, 11); // reserve one line even when empty
    local->last_status    = APCONN_NOT_CONNECTED;
    local->connect_button = button_create("CONNECT", "Connect to the Archipelago server.",
                                          false, false, menu_ap_connect, s);
    local->cancel_button  = button_create("CANCEL",  "Return to main menu.",
                                          false, false, menu_ap_cancel, s);

    menu_attach(menu, local->status_label);
    menu_attach(menu, local->server_input);
    menu_attach(menu, local->slot_input);
    menu_attach(menu, local->pass_input);
    menu_attach(menu, local->connect_button);
    menu_attach(menu, local->cancel_button);

    menu_set_userdata(menu, local);
    menu_set_free_cb(menu, menu_ap_free);
    menu_set_tick_cb(menu, menu_ap_tick);

    return menu;
}
