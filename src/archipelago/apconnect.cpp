// Suppress warnings from heavy header-only libs before including them.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#define AP_NO_SCHEMA
#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_STRICT_

#include "submodule/apclientpp/apclient.hpp"
#include "submodule/apclientpp/apuuid.hpp"

#pragma GCC diagnostic pop

extern "C" {
#include "apconnect.h"
#include "apitems.h"
#include "apstate.h"
#include "ap_sgmanager.h"
#include "game/gui/osd/osd.h"
#include "resources/resource_files.h"
#include "utils/log.h"
}

#include <memory>
#include <string>
#include <cstring>
#include <cstdio>

using nlohmann::json;

// ----- Exported globals -----
ap_items_t            APItems        = {};
ap_stats_t            APStats        = {};
ap_seed_settings_t    APSeedSettings = {};
ap_save_t             APSave         = {};
ap_checks_t           APChecks       = {};
ap_tournament_state_t APTournament   = {};
bool                  ap_mode        = false;

// OSD color palette indices for AP notifications
#define AP_OSD_ITEM_COLOR   ((vga_index)0xE7)
#define AP_OSD_ITEM_SHADOW  ((vga_index)0xF8)
#define AP_OSD_CONN_COLOR   ((vga_index)0xFD)
#define AP_OSD_CONN_SHADOW  ((vga_index)0xC0)

// ----- Internal state -----
static std::unique_ptr<APClient> ap;
static ap_connection_status_t    g_status = APCONN_NOT_CONNECTED;
static void (*g_item_cb)(const char *item_name, const char *player_name) = nullptr;
static void (*g_buy_hint_cb)(int64_t location_id, const char *item_name, const char *player_name) = nullptr;
static void (*g_items_done_cb)(void) = nullptr;

// ----- OSD helpers -----

static std::string format_item_osd(const std::string& item, const std::string& player) {
    std::string s = item + " from " + player;
    for (char& c : s) c = (char)tolower((unsigned char)c);
    auto repl = [](std::string& str, const char* from, const char* to) {
        size_t p = 0, flen = strlen(from), tlen = strlen(to);
        while ((p = str.find(from, p)) != std::string::npos) {
            str.replace(p, flen, to);
            p += tlen;
        }
    };
    repl(s, "progressive ", "prog. ");
    repl(s, "stun resist", "stun res");
    repl(s, "endurance", "endur.");
    return s;
}

// ----- Item helpers -----

static bool is_har_unlock(int64_t id) {
    return id >= AP_BASE_ID && id <= AP_BASE_ID + 10;
}

static bool is_har_stat(int64_t id) {
    return id >= AP_BASE_ID + 100 && id < AP_BASE_ID + 100 + 11 * 6;
}

static bool is_pilot_stat(int64_t id) {
    return id >= AP_BASE_ID + 200 && id < AP_BASE_ID + 200 + 3;
}

static bool is_progressive(int64_t id) {
    return is_har_stat(id) || is_pilot_stat(id);
}

static void apply_item_idempotent(int64_t id) {
    if (is_har_unlock(id)) {
        int har = (int)(id - AP_BASE_ID);
        APItems.har_unlocked |= (uint16_t)(1u << har);
    } else if (is_har_stat(id)) {
        int offset = (int)(id - (AP_BASE_ID + 100));
        int har    = offset / AP_STAT_COUNT;
        int stat   = offset % AP_STAT_COUNT;
        if (har < 11 && stat < AP_STAT_COUNT) {
            uint8_t cur = APItems.har_stats[har][stat];
            if (cur < APSeedSettings.har_stat_max) {
                APItems.har_stats[har][stat] = (uint8_t)(cur + 1);
            }
        }
    } else if (is_pilot_stat(id)) {
        int stat = (int)(id - (AP_BASE_ID + 200));
        if (stat < AP_PILOT_STAT_COUNT) {
            uint8_t cur = APItems.pilot_stats[stat];
            if (cur < APSeedSettings.pilot_stat_max) {
                APItems.pilot_stats[stat] = (uint8_t)(cur + 1);
            }
        }
    } else if (id == AP_ITEM_TOURNAMENT_ACCESS) {
        if (APItems.tournament_access_count < 3)
            APItems.tournament_access_count++;
    } else if (id == AP_ITEM_HAR_COLOR_PRIMARY) {
        APItems.har_color_unlocked |= 0x01;
    } else if (id == AP_ITEM_HAR_COLOR_SECONDARY) {
        APItems.har_color_unlocked |= 0x02;
    } else if (id == AP_ITEM_HAR_COLOR_TERTIARY) {
        APItems.har_color_unlocked |= 0x04;
    }
}

extern "C" int32_t ap_money_award(int32_t base, int32_t step, uint32_t received_count) {
    return base + step * (int32_t)received_count;
}

// Each money check credits every HAR's wallet equally (not just the active HAR),
// so a backlog of checks doesn't over-fund whichever HAR happens to be active.
static void apply_item_consumable(int64_t id) {
    if (id == AP_ITEM_MONEY_SMALL || id == AP_ITEM_MONEY_LARGE) {
        bool small = (id == AP_ITEM_MONEY_SMALL);
        uint32_t count = small ? APSave.money_small_received : APSave.money_large_received;
        int32_t base   = small ? APSeedSettings.money_small_value : APSeedSettings.money_large_value;
        int32_t step   = small ? APSeedSettings.money_small_step  : APSeedSettings.money_large_step;
        int32_t award  = ap_money_award(base, step, count);
        if (small) APSave.money_small_received++; else APSave.money_large_received++;
        for (int i = 0; i < 11; i++) APSave.har_money[i] += award;
        APStats.pending_money += award;
        log_debug("AP - money RECEIVED %s: count=%u base=%d step=%d award=%d -> each HAR wallet +%d (HAR0 now %d)",
                  small ? "small" : "large", count,
                  small ? APSeedSettings.money_small_value : APSeedSettings.money_large_value,
                  small ? APSeedSettings.money_small_step  : APSeedSettings.money_large_step,
                  award, award, APSave.har_money[0]);
    }
    // HAR color and tournament access items are idempotent progressives, not consumables
}

// ----- Handlers -----

static void on_items_received(const std::list<APClient::NetworkItem>& items) {
    if (items.empty()) return;

    // The server sends either a full replay (index starts at 0) or an incremental
    // batch (index > 0). Only wipe and rebuild on a full replay; for incremental
    // batches, accumulate on top of the existing state.
    bool full_replay = (items.front().index == 0);
    if (full_replay) {
        log_debug("AP - items_received: full replay (%zu items)", items.size());
        memset(&APItems,  0, sizeof(APItems));
        memset(&APChecks, 0, sizeof(APChecks));
        // Re-apply starting HAR in case it is precollected and absent from the replay.
        if (APSeedSettings.starting_har >= 0 && APSeedSettings.starting_har <= 10) {
            APItems.har_unlocked |= (uint16_t)(1u << APSeedSettings.starting_har);
        }
    } else {
        log_debug("AP - items_received: incremental batch (%zu items, first index %u)",
                  items.size(), (unsigned)items.front().index);
    }

    for (auto& net_item : items) {
        apply_item_idempotent(net_item.item);

        // Rebuild check counters from the location each item came from.
        // This tells us how many times each buy/train location has been checked,
        // independently of what item was received there.
        int64_t loc = net_item.location;
        if (loc >= AP_LOC_TRAIN_BASE &&
                loc < AP_LOC_TRAIN_BASE + (int64_t)AP_PILOT_STAT_COUNT * AP_LOC_TRAIN_STAT_STRIDE) {
            int stat = (int)((loc - AP_LOC_TRAIN_BASE) / AP_LOC_TRAIN_STAT_STRIDE);
            if (stat < AP_PILOT_STAT_COUNT && APChecks.pilot_train[stat] < 255)
                APChecks.pilot_train[stat]++;
        } else if (loc >= AP_LOC_BUY_BASE &&
                   loc < AP_LOC_BUY_BASE + (int64_t)11 * AP_LOC_BUY_HAR_STRIDE) {
            int64_t off = loc - AP_LOC_BUY_BASE;
            int har  = (int)(off / AP_LOC_BUY_HAR_STRIDE);
            int stat = (int)((off % AP_LOC_BUY_HAR_STRIDE) / AP_LOC_BUY_STAT_STRIDE);
            if (har < 11 && stat < AP_STAT_COUNT && APChecks.har_buy[har][stat] < 255)
                APChecks.har_buy[har][stat]++;
        }

        // Consumables (money): only apply for indices beyond what we already processed.
        if ((uint32_t)net_item.index > APSave.last_applied_item_index) {
            apply_item_consumable(net_item.item);
            APSave.last_applied_item_index = (uint32_t)net_item.index;
        }

        if (g_item_cb || !full_replay) {
            std::string iname = ap->get_item_name(net_item.item, ap->get_player_game(net_item.player));
            std::string pname = ap->get_player_alias(net_item.player);
            log_debug("AP - item: %s from %s (index %u, loc %lld)",
                      iname.c_str(), pname.c_str(),
                      (unsigned)net_item.index, (long long)net_item.location);
            if (g_item_cb)
                g_item_cb(iname.c_str(), pname.c_str());
            if (!full_replay) {
                std::string osd_text = format_item_osd(iname, pname);
                osd_print_color(AP_OSD_ITEM_COLOR, AP_OSD_ITEM_SHADOW, "%s", osd_text.c_str());
            }
        }
    }

    // Fire once after the full batch so callers can refresh UI with complete state.
    if (g_items_done_cb) g_items_done_cb();
}

static void on_slot_connected(const json& slot_data) {
    log_info("AP - slot connected");
    // Parse mandatory slot data fields.
    if (slot_data.contains("goal_tournament"))
        APSeedSettings.goal_tournament = slot_data["goal_tournament"].get<int>();
    if (slot_data.contains("starting_har"))
        APSeedSettings.starting_har = slot_data["starting_har"].get<int>();
    if (slot_data.contains("har_stat_max"))
        APSeedSettings.har_stat_max = slot_data["har_stat_max"].get<int>();
    if (slot_data.contains("pilot_stat_max"))
        APSeedSettings.pilot_stat_max = slot_data["pilot_stat_max"].get<int>();
    if (slot_data.contains("include_buy"))
        APSeedSettings.include_buy = slot_data["include_buy"].get<bool>();
    if (slot_data.contains("buy_cost_factor"))
        APSeedSettings.buy_cost_factor = slot_data["buy_cost_factor"].get<int>();
    APSeedSettings.money_small_value = AP_MONEY_SMALL_VALUE;
    APSeedSettings.money_large_value = AP_MONEY_LARGE_VALUE;
    if (slot_data.contains("money_small_value"))
        APSeedSettings.money_small_value = slot_data["money_small_value"].get<int>();
    if (slot_data.contains("money_large_value"))
        APSeedSettings.money_large_value = slot_data["money_large_value"].get<int>();
    APSeedSettings.money_small_step = AP_MONEY_SMALL_STEP;
    APSeedSettings.money_large_step = AP_MONEY_LARGE_STEP;
    if (slot_data.contains("money_small_step"))
        APSeedSettings.money_small_step = slot_data["money_small_step"].get<int>();
    if (slot_data.contains("money_large_step"))
        APSeedSettings.money_large_step = slot_data["money_large_step"].get<int>();
    APSeedSettings.shop_hints = true;
    if (slot_data.contains("shop_hints"))
        APSeedSettings.shop_hints = slot_data["shop_hints"].get<bool>();
    APSeedSettings.difficulty = 1;
    if (slot_data.contains("difficulty"))
        APSeedSettings.difficulty = slot_data["difficulty"].get<int>();

    // Mark starting HAR as unlocked — it's precollected on server but we still need it locally.
    if (APSeedSettings.starting_har >= 0 && APSeedSettings.starting_har <= 10) {
        APItems.har_unlocked |= (uint16_t)(1u << APSeedSettings.starting_har);
    }

    g_status = APCONN_READY;
    ap_mode  = true;
    log_info("AP - ready: goal=%d starting_har=%d har_stat_max=%d pilot_stat_max=%d include_buy=%d",
             APSeedSettings.goal_tournament, APSeedSettings.starting_har,
             APSeedSettings.har_stat_max, APSeedSettings.pilot_stat_max,
             (int)APSeedSettings.include_buy);
    osd_print_color(AP_OSD_CONN_COLOR, AP_OSD_CONN_SHADOW, "Connected to Archipelago");
}

// ----- Public API -----

extern "C" void Archipelago_Connect(const char *uri, const char *slot, const char *password) {
    log_info("AP - connecting: uri=%s slot=%s", uri, slot);
    g_status = APCONN_CONNECTING;
    ap_mode  = false;

    std::string uuid     = ap_get_uuid("openomf");
    std::string slot_str = slot     ? slot     : "";
    std::string pass_str = password ? password : "";
    ap = std::make_unique<APClient>(uuid, "One Must Fall: 2097", uri);

    ap->set_room_info_handler([=]() {
        ap->ConnectSlot(slot_str, pass_str,
                        0b111 /* ITEMS_HANDLING_ALL */, {}, {0, 6, 0});
    });
    ap->set_slot_connected_handler(on_slot_connected);
    ap->set_slot_refused_handler([](const std::list<std::string>& errors) {
        (void)errors;
        log_error("AP - slot refused");
        g_status = APCONN_FATAL_ERROR;
        osd_print_color(AP_OSD_CONN_COLOR, AP_OSD_CONN_SHADOW, "AP: connection refused");
    });
    ap->set_items_received_handler(on_items_received);
    ap->set_location_info_handler([=](const std::list<APClient::NetworkItem>& items) {
        if (!g_buy_hint_cb) return;
        for (const auto& item : items) {
            std::string iname = ap->get_item_name(item.item, ap->get_player_game(item.player));
            std::string pname = ap->get_player_alias(item.player);
            g_buy_hint_cb(item.location, iname.c_str(), pname.c_str());
        }
    });
    ap->set_socket_disconnected_handler([]() {
        log_info("AP - socket disconnected");
        if (g_status == APCONN_READY) {
            g_status = APCONN_CONNECTING;
            osd_print_color(AP_OSD_CONN_COLOR, AP_OSD_CONN_SHADOW, "AP: disconnected, reconnecting...");
        }
    });
    ap->set_print_json_handler([](const APClient::PrintJSONArgs& args) {
        if (args.type == "Chat") {
            std::string text = ap->render_json(args.data);
            if (text.size() >= 255) text.resize(255);
            osd_print_color(AP_OSD_CONN_COLOR, AP_OSD_CONN_SHADOW, "%s", text.c_str());
        } else if (args.type == "ItemSend" && args.receiving != nullptr) {
            if (!ap->slot_concerns_self(*args.receiving)) {
                std::string text = ap->render_json(args.data);
                if (text.size() >= 255) text.resize(255);
                osd_print_color(AP_OSD_ITEM_COLOR, AP_OSD_ITEM_SHADOW, "%s", text.c_str());
            }
        }
    });
}

extern "C" void Archipelago_Poll(void) {
    if (ap) ap->poll();
}

extern "C" void Archipelago_Disconnect(void) {
    ap.reset();
    g_status = APCONN_NOT_CONNECTED;
    ap_mode  = false;

}

extern "C" ap_connection_status_t Archipelago_ConnectionStatus(void) {
    return g_status;
}

extern "C" void Archipelago_SendCheck(int64_t location_id) {
    log_debug("AP - send check: loc %lld", (long long)location_id);
    if (ap && g_status == APCONN_READY) {
        ap->LocationChecks({location_id});
    }
}

extern "C" void Archipelago_GoalComplete(void) {
    log_info("AP - goal complete");
    if (ap && g_status == APCONN_READY) {
        ap->StatusUpdate(APClient::ClientStatus::GOAL);
    }
}

extern "C" void Archipelago_SetItemReceivedCallback(void (*cb)(const char *item_name, const char *player_name)) {
    g_item_cb = cb;
}

extern "C" void Archipelago_ScoutBuyLocation(int64_t location_id) {
    log_debug("AP - scout: loc %lld", (long long)location_id);
    if (ap && g_status == APCONN_READY) {
        int as_hint = APSeedSettings.shop_hints ? 1 : 0;
        ap->LocationScouts({location_id}, as_hint);
    }
}

extern "C" void Archipelago_SetBuyHintCallback(void (*cb)(int64_t location_id, const char *item_name, const char *player_name)) {
    g_buy_hint_cb = cb;
}

extern "C" void Archipelago_SetItemsDoneCallback(void (*cb)(void)) {
    g_items_done_cb = cb;
}


extern "C" void Archipelago_GetSaveIdent(char *out, size_t len) {
    if (!ap || !len) return;
    std::string key = ap->get_seed() + '\0' + ap->get_slot();
    uint32_t h = 2166136261u;
    for (unsigned char c : key) { h ^= c; h *= 16777619u; }
    snprintf(out, len, "AP%08X", (unsigned)h);
}

extern "C" void Archipelago_GetSlotName(char *out, size_t len) {
    if (!ap || !len) return;
    snprintf(out, len, "%s", ap->get_slot().c_str());
}

extern "C" void Archipelago_APSaveState(const char *ident) {
    path save = get_ap_save_directory();
    path_append(&save, ident);
    path_set_ext(&save, ".APS");
    FILE *f = fopen(path_c(&save), "wb");
    if (!f) { log_error("AP - can't write state %s", path_c(&save)); return; }
    const uint8_t magic[4] = {'A','P','S','T'};
    const uint8_t version  = 2;
    fwrite(magic,                           1,             4,  f);
    fwrite(&version,                        1,             1,  f);
    fwrite(APSave.har_money,                sizeof(int32_t), 11, f);
    fwrite(&APSave.last_applied_item_index, sizeof(uint32_t), 1,  f);
    fwrite(&APSave.tournaments_won_mask,    sizeof(uint8_t),  1,  f);
    fwrite(&APSave.money_small_received,    sizeof(uint32_t), 1,  f);
    fwrite(&APSave.money_large_received,    sizeof(uint32_t), 1,  f);
    fclose(f);
    log_debug("AP - state saved: %s (last_idx=%u)", path_c(&save), APSave.last_applied_item_index);
}

extern "C" bool Archipelago_APLoadState(const char *ident) {
    path save = get_ap_save_directory();
    path_append(&save, ident);
    path_set_ext(&save, ".APS");
    FILE *f = fopen(path_c(&save), "rb");
    if (!f) { log_debug("AP - no state file: %s", path_c(&save)); return false; }
    uint8_t magic[4] = {};
    uint8_t version  = 0;
    fread(magic,    1, 4, f);
    fread(&version, 1, 1, f);
    if (memcmp(magic, "APST", 4) != 0) {
        log_error("AP - bad magic in state file %s", path_c(&save));
        fclose(f);
        return false;
    }
    if (version >= 1) {
        fread(APSave.har_money,             sizeof(int32_t),  11, f);
        uint32_t disk_idx = 0;
        fread(&disk_idx,                    sizeof(uint32_t), 1,  f);
        if (disk_idx > APSave.last_applied_item_index)
            APSave.last_applied_item_index = disk_idx;
        fread(&APSave.tournaments_won_mask, sizeof(uint8_t),  1,  f);
    }
    if (version >= 2) {
        fread(&APSave.money_small_received, sizeof(uint32_t), 1, f);
        fread(&APSave.money_large_received, sizeof(uint32_t), 1, f);
    }
    fclose(f);
    log_debug("AP - state loaded: %s (last_idx=%u)", path_c(&save), APSave.last_applied_item_index);
    return true;
}
