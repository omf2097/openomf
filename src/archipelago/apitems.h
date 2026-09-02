#pragma once
#include <stdint.h>

// Must stay in sync with archipelago/worlds/openomf/Items.py and Locations.py.

#define AP_BASE_ID ((int64_t)20970000)

// --- Item IDs ---

// HAR unlock items: BASE + 0..10
#define AP_ITEM_HAR_UNLOCK(har_idx)   (AP_BASE_ID + (har_idx))

// HAR stat progressive items: BASE + 100 + har*6 + stat
#define AP_ITEM_HAR_STAT(har_idx, stat_idx) \
    (AP_BASE_ID + 100 + (har_idx) * 6 + (stat_idx))

// Pilot stat progressive items: BASE + 200 + stat
#define AP_ITEM_PILOT_STAT(stat_idx)  (AP_BASE_ID + 200 + (stat_idx))

// Filler / miscellaneous items
#define AP_ITEM_MONEY_SMALL         (AP_BASE_ID + 300)
#define AP_ITEM_MONEY_LARGE         (AP_BASE_ID + 301)
#define AP_ITEM_HAR_COLOR_PRIMARY   (AP_BASE_ID + 302)
#define AP_ITEM_HAR_COLOR_SECONDARY (AP_BASE_ID + 303)
#define AP_ITEM_HAR_COLOR_TERTIARY  (AP_BASE_ID + 304)

// Progressive tournament access (3 copies unlock Katushai=1, WAR=2, World=3)
#define AP_ITEM_TOURNAMENT_ACCESS   (AP_BASE_ID + 500)

// --- Location IDs ---

// Match locations: BASE + 1000 + global_match_index
#define AP_LOC_MATCH_BASE    (AP_BASE_ID + 1000)
#define ap_match_location_id(global_idx) \
    (AP_LOC_MATCH_BASE + (int64_t)(global_idx))

// Tournament win locations: BASE + 2000 + tournament_idx*3 + reward_idx (0-2)
#define AP_LOC_WIN_BASE      (AP_BASE_ID + 2000)
#define ap_tournament_win_id(tournament_idx, reward_idx) \
    (AP_LOC_WIN_BASE + (int64_t)(tournament_idx) * 3 + (int64_t)(reward_idx))

// HAR buy locations: BASE + 3000 + har*120 + stat*20 + (level-1)
// stride: 6 stats * 20 level ceiling = 120 per HAR
#define AP_LOC_BUY_BASE          (AP_BASE_ID + 3000)
#define AP_LOC_BUY_HAR_STRIDE    120
#define AP_LOC_BUY_STAT_STRIDE   20
#define ap_har_buy_location_id(har, stat, level) \
    (AP_LOC_BUY_BASE + (int64_t)(har) * AP_LOC_BUY_HAR_STRIDE \
     + (int64_t)(stat) * AP_LOC_BUY_STAT_STRIDE + (int64_t)((level) - 1))

// Pilot train locations: BASE + 5000 + stat*50 + (level-1)
#define AP_LOC_TRAIN_BASE        (AP_BASE_ID + 5000)
#define AP_LOC_TRAIN_STAT_STRIDE 50
#define ap_train_location_id(stat, level) \
    (AP_LOC_TRAIN_BASE + (int64_t)(stat) * AP_LOC_TRAIN_STAT_STRIDE \
     + (int64_t)((level) - 1))

// Tournament indices (match Python world GoalTournament option values)
#define AP_TOURNAMENT_NORTH_AMERICAN_OPEN 0
#define AP_TOURNAMENT_KATUSHAI_CHALLENGE  1
#define AP_TOURNAMENT_WAR_INVITATIONAL    2
#define AP_TOURNAMENT_WORLD_CHAMPIONSHIP  3
#define AP_TOURNAMENT_ALL                 4

// Money bundle values (in game credits).
#define AP_MONEY_SMALL_VALUE  3000
#define AP_MONEY_LARGE_VALUE  15000

// Growth in award value per money item of that type already received (0 = flat).
#define AP_MONEY_SMALL_STEP   150
#define AP_MONEY_LARGE_STEP   750
