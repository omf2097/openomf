#include "ap_mechlab.h"

#include <math.h>
#include <string.h>
#include <ctype.h>

#include "archipelago/apconnect.h"
#include "archipelago/apitems.h"
#include "archipelago/apstate.h"
#include "archipelago/ap_arena.h"
#include "formats/chr.h"
#include "formats/pilot.h"
#include "formats/tournament.h"
#include "game/game_player.h"
#include "game/game_state.h"
#include "game/gui/component.h"
#include "game/gui/label.h"
#include "game/gui/trn_menu.h"
#include "game/scenes/mechlab.h"
#include "game/scenes/mechlab/har_economy.h"
#include "utils/c_string_util.h"
#include "game/scenes/mechlab/lab_menu_confirm.h"
#include "game/scenes/mechlab/lab_menu_trade.h"
#include "game/utils/formatting.h"
#include "resources/languages.h"
#include "archipelago/ap_sgmanager.h"
#include "resources/trnmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"

// --- Private state ---

static scene   *g_scene       = NULL;
static int64_t  g_scout_loc   = 0;

static component *g_train_stat_label  = NULL;
static component *g_train_price_label = NULL;
static component *g_buy_header_label  = NULL;
static component *g_buy_details_label = NULL;

// --- Price tables ---

// Training prices (AP fixed table, shared with lab_menu_training.c).
static const int32_t s_train_prices[] = {50,   80,   120,   180,   240,   300,   450,   600,   800,   1100,   1500,   2500,
                                         4000, 7000, 10000, 14000, 20000, 28000, 40000, 55000, 75000, 100000, 140000, 200000};

static const char *const s_buy_headers[AP_STAT_COUNT] = {
    "ARM POWER:\n\nUPGRADE COST:",
    "LEG POWER:\n\nUPGRADE COST:",
    "ARM SPEED:\n\nUPGRADE COST:",
    "LEG SPEED:\n\nUPGRADE COST:",
    "ARMOR PLATE:\n\nUPGRADE COST:",
    "STUN RES.:\n\nUPGRADE COST:",
};

static const int32_t s_buy_multipliers[AP_STAT_COUNT] = {
    arm_leg_multiplier, arm_leg_multiplier, arm_leg_multiplier,
    arm_leg_multiplier, armor_multiplier,   stun_res_multiplier,
};

// --- Pure helpers ---

// Map tournament filename prefix to AP index 0-3, or -1 if unknown.
static int trn_filename_to_idx(const char *filename) {
    static const struct { const char *prefix; int len; } trns[] = {
        {"NORTH_AM", 8}, {"KATUSHAI", 8}, {"WAR", 3}, {"WORLD", 5},
    };
    for(int i = 0; i < 4; i++) {
        if(strncmp(filename, trns[i].prefix, trns[i].len) == 0) return i;
    }
    return -1;
}

// Wrap buf in-place at 39 chars: insert '\n' at the last space before col 39,
// or hard-truncate if no space found. buf must be at least 79 bytes.
static void ap_word_wrap(char *buf) {
    if(strlen(buf) <= 39) return;
    for(int i = 38; i >= 0; i--) {
        if(buf[i] != ' ') continue;
        buf[i] = '\n';
        buf[i + 40] = '\0';
        return;
    }
    buf[39] = '\0';
}

static bool har_color_bit(int bit) {
    return (APItems.har_color_unlocked & (uint8_t)(1u << bit)) != 0;
}

// --- Format helpers ---

// Build "ITEM - PLAYER", uppercase, abbreviate "PROGRESSIVE "->"PROG. ", word-wrap at 39.
static void ap_format_hint(const char *item_name, const char *player_name, char *out, size_t out_size) {
    char buf[128];
    snprintf(buf, sizeof(buf), "%s - %s", item_name, player_name);
    for(int i = 0; buf[i]; i++) buf[i] = (char)toupper((unsigned char)buf[i]);

    static const char from[] = "PROGRESSIVE ", to[] = "PROG. ";
    const size_t flen = sizeof(from) - 1, tlen = sizeof(to) - 1;
    char sub[128];
    size_t si = 0, ti = 0;
    while(buf[ti] && si + 1 < sizeof(sub)) {
        if(strncmp(buf + ti, from, flen) == 0 && si + tlen < sizeof(sub)) {
            memcpy(sub + si, to, tlen); si += tlen; ti += flen;
        } else {
            sub[si++] = buf[ti++];
        }
    }
    sub[si] = '\0';

    ap_word_wrap(sub);
    snprintf(out, out_size, "%s", sub);
}

// --- Pilot helpers ---

static void ap_sync_pilot(sd_pilot *pilot) {
    int har = pilot->har_id;
    if(har >= 0 && har < 11) {
        pilot->arm_power       = APItems.har_stats[har][AP_STAT_ARM_POWER];
        pilot->leg_power       = APItems.har_stats[har][AP_STAT_LEG_POWER];
        pilot->arm_speed       = APItems.har_stats[har][AP_STAT_ARM_SPEED];
        pilot->leg_speed       = APItems.har_stats[har][AP_STAT_LEG_SPEED];
        pilot->armor           = APItems.har_stats[har][AP_STAT_ARMOR];
        pilot->stun_resistance = APItems.har_stats[har][AP_STAT_STUN_RESIST];
    }
    pilot->power      = APItems.pilot_stats[AP_PILOT_POWER];
    pilot->agility    = APItems.pilot_stats[AP_PILOT_AGILITY];
    pilot->endurance  = APItems.pilot_stats[AP_PILOT_ENDURANCE];
    pilot->har_trades = APItems.har_unlocked;
}

// Money is credited directly into APSave.har_money[] as items arrive (see
// apply_item_consumable), which bumps every HAR's wallet by the same amount.
// Add that delta into the active pilot's runtime money rather than overwriting
// from the wallet: runtime money is authoritative while spending (buys/training
// decrement it directly and only flush to the wallet on mechlab-save), and this
// sync also runs after buy/train checks echo an item back from the server, so
// overwriting here would silently revert an unsaved spend.
static void ap_sync_money(sd_pilot *pilot) {
    if(pilot->har_id >= 11 || APStats.pending_money == 0) return;
    log_debug("AP - money sync: HAR %d pilot->money %d + pending %d (wallet %d)", pilot->har_id, pilot->money,
              APStats.pending_money, APSave.har_money[pilot->har_id]);
    pilot->money += APStats.pending_money;
    APStats.pending_money = 0;
}

// --- Scout cache ---

#define SCOUT_CACHE_MAX 1024
typedef struct { int64_t loc; char item_name[80]; char player_name[48]; } scout_entry_t;
static scout_entry_t g_scout_cache[SCOUT_CACHE_MAX];
static int           g_scout_cache_len = 0;

static void ap_cache_store(int64_t loc, const char *item_name, const char *player_name) {
    for(int i = 0; i < g_scout_cache_len; i++) {
        if(g_scout_cache[i].loc == loc) return;
    }
    if(g_scout_cache_len >= SCOUT_CACHE_MAX) return;
    scout_entry_t *e = &g_scout_cache[g_scout_cache_len++];
    e->loc = loc;
    strncpy(e->item_name,   item_name,   79); e->item_name[79]   = '\0';
    strncpy(e->player_name, player_name, 47); e->player_name[47] = '\0';
}

static bool ap_cache_hit(scene *s, int64_t loc) {
    for(int i = 0; i < g_scout_cache_len; i++) {
        if(g_scout_cache[i].loc != loc) continue;
        char buf[79];
        ap_format_hint(g_scout_cache[i].item_name, g_scout_cache[i].player_name, buf, sizeof(buf));
        mechlab_set_hint(s, buf);
        log_debug("AP - scout cache hit: loc %lld: %s", (long long)loc, g_scout_cache[i].item_name);
        return true;
    }
    return false;
}

// Arm a scout for loc: serve from cache if present, otherwise request from server.
static void scout_for_hint(scene *s, int64_t loc) {
    g_scout_loc = loc;
    if(!ap_cache_hit(s, loc)) Archipelago_ScoutBuyLocation(loc);
}

// --- Tournament helpers ---

static void restore_tournament(const char *trn_name) {
    static const int ap_offsets[] = AP_TOURNAMENT_OFFSETS;
    int tidx = trn_filename_to_idx(trn_name);
    if(tidx < 0) return;
    APTournament.tournament_idx = tidx;
    APTournament.match_offset   = ap_offsets[tidx];
    log_debug("AP: restored tournament %d (offset %d) from '%s'", tidx, ap_offsets[tidx], trn_name);
}

// --- AP Callbacks ---

static void on_item_received(const char *item_name, const char *player_name) {
    (void)item_name; (void)player_name;
    if(!g_scene) return;
    game_player *p1 = game_state_get_player(g_scene->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(pilot) ap_sync_pilot(pilot);
}

static void on_items_done(void) {
    if(!g_scene) return;
    game_player *p1 = game_state_get_player(g_scene->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(pilot) {
        ap_sync_money(pilot);
        ap_sync_pilot(pilot);
    }
    log_debug("AP - items_done: pilot synced");
    mechlab_update(g_scene);
}

static void on_buy_hint(int64_t location_id, const char *item_name, const char *player_name) {
    ap_cache_store(location_id, item_name, player_name);
    if(!g_scene) return;
    if(location_id != g_scout_loc) {
        log_debug("AP - stale scout suppressed (loc %lld expected %lld): %s",
                  (long long)location_id, (long long)g_scout_loc, item_name);
        return;
    }
    log_debug("AP - scout hint: %s (from %s)", item_name, player_name);
    char buf[79];
    ap_format_hint(item_name, player_name, buf, sizeof(buf));
    mechlab_set_hint(g_scene, buf);
}

// --- Public label API ---

int32_t ap_train_price(int level) {
    int n = (int)(sizeof(s_train_prices) / sizeof(s_train_prices[0]));
    if(level < 0 || level >= n) return 0;
    int32_t base = s_train_prices[level];
    if(!ap_mode || APSeedSettings.buy_cost_factor == 100) return base;
    return (int32_t)(base * (APSeedSettings.buy_cost_factor / 100.0));
}

void ap_register_train_labels(component *stat_label, component *price_label) {
    g_train_stat_label  = stat_label;
    g_train_price_label = price_label;
}

void ap_update_train_labels(int stat_lang_id, int next_level) {
    if(!g_train_stat_label || !g_train_price_label) return;
    label_set_text(g_train_stat_label, lang_get(stat_lang_id));
    if(next_level >= APSeedSettings.pilot_stat_max) {
        label_set_text(g_train_price_label, "UNAVAILABLE");
    } else {
        char tmp[32], price_str[16];
        score_format(ap_train_price(next_level), price_str, sizeof(price_str));
        snprintf(tmp, sizeof(tmp), "$ %sK", price_str);
        label_set_text(g_train_price_label, tmp);
    }
}

void ap_register_buy_labels(component *header_label, component *details_label) {
    g_buy_header_label  = header_label;
    g_buy_details_label = details_label;
}

void ap_update_buy_har_labels(sd_pilot *pilot, int stat, int next_buy_level) {
    if(!g_buy_header_label || !g_buy_details_label || stat < 0 || stat >= AP_STAT_COUNT) return;
    label_set_text(g_buy_header_label, s_buy_headers[stat]);
    if(next_buy_level >= APSeedSettings.har_stat_max) {
        label_set_text(g_buy_details_label, "Unavailable\n\nUnavailable");
    } else {
        char tmp[200], price_str[32];
        int32_t vanilla = har_upgrade_price[pilot->har_id]
                        * upgrade_level_multiplier[next_buy_level + 1]
                        * s_buy_multipliers[stat];
        int32_t price = ap_buy_price(vanilla, next_buy_level + 1);
        score_format(price, price_str, sizeof(price_str));
        snprintf(tmp, sizeof(tmp), "Level %d\n\n$ %sK", next_buy_level + 1, price_str);
        label_set_text(g_buy_details_label, tmp);
    }
}

// --- Lifecycle ---

void ap_mechlab_attach(scene *s) {
    g_scene     = s;
    g_scout_loc = 0;

    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(pilot) {
        ap_sync_money(pilot);
        ap_sync_pilot(pilot);
    }

    Archipelago_SetItemReceivedCallback(on_item_received);
    Archipelago_SetBuyHintCallback(on_buy_hint);
    Archipelago_SetItemsDoneCallback(on_items_done);
}

void ap_mechlab_detach(void) {
    Archipelago_SetItemReceivedCallback(NULL);
    Archipelago_SetBuyHintCallback(NULL);
    Archipelago_SetItemsDoneCallback(NULL);
    g_scene             = NULL;
    g_scout_loc         = 0;
    g_train_stat_label  = NULL;
    g_train_price_label = NULL;
    g_buy_header_label  = NULL;
    g_buy_details_label = NULL;
}

// --- Training ---

void ap_do_train(scene *s, int stat) {
    int level = APChecks.pilot_train[stat];
    log_debug("AP - train stat %d: check level %d", stat, level + 1);
    Archipelago_SendCheck(ap_train_location_id(stat, level + 1));
    // Do NOT increment APChecks here; on_item_received will increment it when
    // the server echoes the item back, keeping the counter as single source of truth.
    if(level + 2 <= APSeedSettings.pilot_stat_max)
        scout_for_hint(s, ap_train_location_id(stat, level + 2));
}

void ap_focus_train(scene *s, int stat) {
    if(APChecks.pilot_train[stat] >= APSeedSettings.pilot_stat_max) return;
    scout_for_hint(s, ap_train_location_id(stat, APChecks.pilot_train[stat] + 1));
}

// --- HAR upgrades ---

int32_t ap_buy_price(int32_t vanilla_price, int current_level) {
    (void)current_level;
    if(!ap_mode || APSeedSettings.buy_cost_factor == 100) return vanilla_price;
    return (int32_t)(vanilla_price * (APSeedSettings.buy_cost_factor / 100.0));
}

void ap_do_buy_har(scene *s, int har_id, int stat) {
    int buy_level = APChecks.har_buy[har_id][stat];
    log_debug("AP - buy HAR %d stat %d: level %d", har_id, stat, buy_level + 1);
    Archipelago_SendCheck(ap_har_buy_location_id(har_id, stat, buy_level + 1));
    // Do NOT increment APChecks here; on_item_received will increment it when
    // the server echoes the item back, keeping the counter as single source of truth.
    if(buy_level + 2 <= APSeedSettings.har_stat_max)
        scout_for_hint(s, ap_har_buy_location_id(har_id, stat, buy_level + 2));
    mechlab_update(s);
}

void ap_focus_buy_har(scene *s, int har_id, int stat) {
    int buy_level = APChecks.har_buy[har_id][stat];
    if(buy_level >= APSeedSettings.har_stat_max) return;
    scout_for_hint(s, ap_har_buy_location_id(har_id, stat, buy_level + 1));
}

void ap_apply_har_stats(sd_pilot *pilot) {
    ap_sync_pilot(pilot);
}

// --- HAR color queries ---

bool ap_has_har_color_primary(void)   { return har_color_bit(0); }
bool ap_has_har_color_secondary(void) { return har_color_bit(1); }
bool ap_has_har_color_tertiary(void)  { return har_color_bit(2); }

// --- HAR customize ---

static const int s_buy_hint_lang[AP_STAT_COUNT] = {554, 556, 558, 560, 562, 564};
static const char *const s_buy_hint_arg[AP_STAT_COUNT] = {"arm", "leg", "arm", "leg", NULL, NULL};

void ap_customize_buy(scene *s, sd_pilot *pilot, int stat) {
    int buy_level = APChecks.har_buy[pilot->har_id][stat];
    int32_t vanilla = har_upgrade_price[pilot->har_id]
                    * upgrade_level_multiplier[buy_level + 1]
                    * s_buy_multipliers[stat];
    int32_t price = ap_buy_price(vanilla, buy_level + 1);
    int32_t before = pilot->money;
    pilot->money -= price;
    log_debug("AP - money SPENT (buy HAR %d stat %d level %d): price=%d %d -> %d",
              pilot->har_id, stat, buy_level + 1, price, before, pilot->money);
    ap_do_buy_har(s, pilot->har_id, stat);
    ap_update_buy_har_labels(pilot, stat, buy_level + 1);
}

void ap_customize_check_price(component *c, sd_pilot *pilot, int stat) {
    int buy_level = APChecks.har_buy[pilot->har_id][stat];
    int32_t vanilla = har_upgrade_price[pilot->har_id]
                    * upgrade_level_multiplier[buy_level + 1]
                    * s_buy_multipliers[stat];
    int32_t price = ap_buy_price(vanilla, buy_level + 1);
    component_disable(c, price > pilot->money || buy_level >= APSeedSettings.har_stat_max);
}

void ap_customize_focus(scene *s, sd_pilot *pilot, int stat) {
    int next = (int)APChecks.har_buy[pilot->har_id][stat];
    ap_update_buy_har_labels(pilot, stat, next);
    char hint[100];
    const char *fmt = lang_get(s_buy_hint_lang[stat]);
    if(s_buy_hint_arg[stat]) {
        unsafe_snprintf(hint, sizeof(hint), fmt, s_buy_hint_arg[stat]);
    } else {
        snprintf(hint, sizeof(hint), "%s", fmt);
    }
    mechlab_set_hint(s, hint);
    ap_focus_buy_har(s, pilot->har_id, stat);
}

// --- Pilot training ---

static const int s_train_lang_ids[AP_PILOT_STAT_COUNT] = {512, 513, 514};

void ap_training_buy(scene *s, int stat) {
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    int level = APChecks.pilot_train[stat];
    int32_t price = ap_train_price(level);
    int32_t before = pilot->money;
    pilot->money -= price;
    log_debug("AP - money SPENT (train stat %d level %d): price=%d %d -> %d",
              stat, level + 1, price, before, pilot->money);
    ap_do_train(s, stat);
    ap_update_train_labels(s_train_lang_ids[stat], level + 1);
    mechlab_update(s);
}

void ap_training_check_price(component *c, scene *s, int stat) {
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    int level = APChecks.pilot_train[stat];
    if(level >= APSeedSettings.pilot_stat_max) {
        component_disable(c, 1);
        return;
    }
    component_disable(c, ap_train_price(level) > pilot->money);
}

void ap_training_focus(scene *s, int stat) {
    ap_update_train_labels(s_train_lang_ids[stat], APChecks.pilot_train[stat]);
    ap_focus_train(s, stat);
}

// --- Hint helper ---

void ap_set_hint(scene *s, const char *hint) {
    char buf[79];
    snprintf(buf, sizeof(buf), "%s", hint);
    ap_word_wrap(buf);
    mechlab_set_hint(s, buf);
}

// --- Tournament filtering ---

void ap_filter_trnlist(vector *trnlist) {
    int accessible = APItems.tournament_access_count + 1;
    for(int idx = (int)vector_size(trnlist) - 1; idx >= 0; idx--) {
        sd_tournament_file *t = vector_get(trnlist, idx);
        int ap_idx = trn_filename_to_idx(t->filename);
        if(ap_idx >= accessible) {
            sd_tournament_free(t);
            vector_delete_at(trnlist, idx);
        }
    }
}

// --- Match / tournament win ---

void ap_on_match_win(int trn_index) {
    log_debug("AP - match win: trn_index %d offset %d", trn_index, APTournament.match_offset);
    Archipelago_SendCheck(ap_match_location_id(APTournament.match_offset + trn_index));
}

void ap_on_tournament_win(void) {
    int tidx = APTournament.tournament_idx;
    log_info("AP - tournament win: idx %d", tidx);
    for(int r = 0; r < 3; r++)
        Archipelago_SendCheck(ap_tournament_win_id(tidx, r));
    APSave.tournaments_won_mask |= (uint8_t)(1u << tidx);
    // Persist immediately: the mechlab scene we're about to return to reloads APSave from
    // disk on entry (ap_mechlab_find_player), which would otherwise clobber this mask bit
    // with the stale on-disk value and silently drop it, breaking the "all tournaments" goal.
    char ap_ident[12] = "";
    Archipelago_GetSaveIdent(ap_ident, sizeof(ap_ident));
    Archipelago_APSaveState(ap_ident);
    bool goal_met = (APSeedSettings.goal_tournament == tidx) ||
                    (APSeedSettings.goal_tournament == AP_TOURNAMENT_ALL &&
                     (APSave.tournaments_won_mask & 0x0Fu) == 0x0Fu);
    if(goal_met) {
        log_info("AP - goal complete");
        Archipelago_GoalComplete();
    }
}

// --- Mechlab init / save ---

static void setup_first_run(sd_pilot *pilot, int har_id, const char *slot_name) {
    pilot->har_id     = har_id;
    pilot->money      = APSave.har_money[har_id];
    pilot->difficulty = APSeedSettings.difficulty;
    str_from_c(&pilot->name, slot_name);
}

static void restore_from_save(game_player *p1, const char *slot_name) {
    int har_id = p1->pilot->har_id;
    p1->pilot->money      = APSave.har_money[har_id];
    p1->pilot->difficulty = APSeedSettings.difficulty;
    str_from_c(&p1->pilot->name, slot_name);
    if(p1->pilot->trn_name[0] != '\0')
        restore_tournament(p1->pilot->trn_name);
}

bool ap_mechlab_find_player(scene *s) {
    game_player *p1 = game_state_get_player(s->gs, 0);

    char ident[12] = "";
    Archipelago_GetSaveIdent(ident, sizeof(ident));
    char slot_name[18] = "";
    Archipelago_GetSlotName(slot_name, sizeof(slot_name));
    log_debug("AP: save ident '%s' slot '%s'", ident, slot_name);
    Archipelago_APLoadState(ident);

    int har_id = (APSeedSettings.starting_har >= 0 && APSeedSettings.starting_har <= 10)
                     ? APSeedSettings.starting_har : 0;

    sd_chr_file *chr = omf_calloc(1, sizeof(sd_chr_file));
    if(sg_load_ap_pilot(chr, ident) != SD_SUCCESS) {
        omf_free(chr);
        setup_first_run(p1->pilot, har_id, slot_name);
        log_debug("AP: first run, ident '%s' har %d money %d", ident, har_id, p1->pilot->money);
    } else {
        log_debug("AP: loaded save '%s'", ident);
        p1->chr = chr;
        if(&chr->pilot != p1->pilot)
            game_player_set_pilot(p1, &chr->pilot);
        restore_from_save(p1, slot_name);
    }
    mechlab_load_har(s, p1->pilot);
    return true;
}

bool ap_mechlab_find_and_attach(scene *s) {
    bool found = ap_mechlab_find_player(s);
    ap_mechlab_attach(s);
    return found;
}

void ap_mechlab_save(game_player *p1) {
    char ap_ident[12] = "";
    Archipelago_GetSaveIdent(ap_ident, sizeof(ap_ident));
    int har = p1->chr->pilot.har_id;
    if(har >= 0 && har < 11) {
        log_debug("AP - money save: HAR %d wallet %d -> %d", har, APSave.har_money[har], p1->chr->pilot.money);
        APSave.har_money[har] = p1->chr->pilot.money;
    }
    Archipelago_APSaveState(ap_ident);
    int save_ret = sg_save_ap(p1->chr, ap_ident);
    if(save_ret != SD_SUCCESS)
        log_error("Failed to save pilot %s", str_c(&p1->chr->pilot.name));
}

void ap_mechlab_set_tournament(sd_tournament_file *trn) {
    restore_tournament(trn->filename);
}

// --- Arena helpers ---

void ap_arena_match_win(game_state *gs, game_player *p1, game_player *p2) {
    (void)gs;
    int count = p1->chr->pilot.enemies_inc_unranked;
    for(int k = 0; k < count; k++) {
        sd_chr_enemy *enemy = p1->chr->enemies[k];
        if(enemy && &enemy->pilot == p2->pilot) {
            ap_on_match_win(enemy->trn_index);
            break;
        }
    }
    if(p1->pilot->money < 0) {
        log_debug("AP - money clamp: HAR %d money %d -> 0 (repair cost overspend)", p1->pilot->har_id,
                  p1->pilot->money);
        p1->pilot->money = 0;
    }
}

// --- Trade menu ---

int ap_trade_page(int current_har_id, uint16_t har_trades, int *display_out) {
    static int s_page_start = 0;

    int eligible[11], eligible_count = 0;
    for(int i = 0; i < 11; i++) {
        if(i == current_har_id) continue;
        if(!((har_trades >> i) & 1)) continue;
        eligible[eligible_count++] = i;
    }
    if(eligible_count == 0) return 0;
    if(eligible_count <= 6) {
        for(int k = 0; k < eligible_count; k++) display_out[k] = eligible[k];
        return eligible_count;
    }
    s_page_start = s_page_start % eligible_count;
    for(int k = 0; k < 6; k++)
        display_out[k] = eligible[(s_page_start + k) % eligible_count];
    s_page_start = (s_page_start + 6) % eligible_count;
    return 6;
}

void ap_preview_har(scene *s, int har_id) {
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    pilot->har_id = har_id;
    if(har_id >= 0 && har_id < 11) pilot->money = APSave.har_money[har_id];
    ap_apply_har_stats(pilot);
    mechlab_update(s);
}

void ap_confirm_trade(component *c, scene *s, game_player *p1) {
    int old_har = p1->chr->pilot.har_id;
    int new_har = p1->pilot->har_id;
    if(old_har >= 0 && old_har < 11)
        APSave.har_money[old_har] = p1->chr->pilot.money;
    p1->chr->pilot.har_id = new_har;
    if(new_har >= 0 && new_har < 11)
        p1->chr->pilot.money = APSave.har_money[new_har];
    omf_free(p1->pilot);
    p1->pilot = &p1->chr->pilot;
    ap_apply_har_stats(p1->pilot);
    mechlab_update(s);
    trnmenu_finish(c->parent);
}

void ap_do_trade(component *c, scene *s) {
    game_player *p1 = game_state_get_player(s->gs, 0);
    char tmp[100];
    unsafe_snprintf(tmp, sizeof(tmp), lang_get(520), lang_get(31 + p1->chr->pilot.har_id),
                     lang_get(31 + p1->pilot->har_id));
    component *menu = lab_menu_confirm_create(s, confirm_trade, s, cancel_trade, s, tmp);
    trnmenu_set_userdata(menu, s);
    trnmenu_set_submenu_done_cb(menu, lab_menu_trade_done);
    trnmenu_finish(c->parent);
    trnmenu_set_submenu(c->parent->parent, menu);
}
