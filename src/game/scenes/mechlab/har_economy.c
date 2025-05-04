#include "game/scenes/mechlab/har_economy.h"
#include "formats/pilot.h"
#include "utils/c_array_util.h"
#include "utils/log.h"
#include "utils/random.h"

bool har_can_upgrade(sd_pilot *pilot, enum HAR_UPGRADE upgrade) {
    switch(upgrade) {
        case ARM_POWER:
            return pilot->arm_power < max_arm_power[pilot->har_id];
        case ARM_SPEED:
            return pilot->arm_speed < max_arm_speed[pilot->har_id];
        case LEG_POWER:
            return pilot->leg_power < max_leg_power[pilot->har_id];
        case LEG_SPEED:
            return pilot->leg_speed < max_leg_speed[pilot->har_id];
        case ARMOR:
            return pilot->armor < max_armor[pilot->har_id];
        case STUN_RES:
            return pilot->stun_resistance < max_stun_res[pilot->har_id];
        default:
            abort();
    }
}

void purchase_random_har(sd_pilot *pilot) {
    int32_t possible_purchases[10];
    size_t n = 0;
    for(int32_t i = 0; i < 10; ++i) {
        if(pilot->money >= har_prices[i]) {
            possible_purchases[n] = i;
            ++n;
        }
    }
    assert(n > 0 && "Pilot does not have enough money to purchase any HAR");
    pilot->har_id = possible_purchases[rand_int(n)];
    pilot->money -= har_prices[pilot->har_id];
    log_debug("Pilot %s purchased HAR %u and has %d money left", pilot->name, pilot->har_id, pilot->money);
}

void purchase_random_har_upgrades(sd_pilot *pilot) {
    enum HAR_UPGRADE possible_upgrades[N_UPGRADES];
    size_t n;
    do {
        n = 0;
        for(enum HAR_UPGRADE u = ARM_POWER; u < N_UPGRADES; ++u) {
            if(har_can_upgrade(pilot, u) && pilot->money >= upgrade_price(pilot, u)) {
                possible_upgrades[n] = u;
                ++n;
            }
        }
        if(n == 0) {
            break;
        }
        enum HAR_UPGRADE chosen_upgrade = possible_upgrades[rand_int(n)];
        upgrade_har(pilot, chosen_upgrade);
        log_debug("Pilot %s bought upgrade %d and has %d money left", pilot->name, chosen_upgrade, pilot->money);
    } while(n > 0);
}

int32_t upgrade_price(sd_pilot *pilot, enum HAR_UPGRADE upgrade) {
    int32_t price = har_upgrade_price[pilot->har_id];
    switch(upgrade) {
        case ARM_POWER:
            price *= upgrade_level_multiplier[pilot->arm_power + 1] * arm_leg_multiplier;
            break;
        case ARM_SPEED:
            price *= upgrade_level_multiplier[pilot->arm_speed + 1] * arm_leg_multiplier;
            break;
        case LEG_POWER:
            price *= upgrade_level_multiplier[pilot->leg_power + 1] * arm_leg_multiplier;
            break;
        case LEG_SPEED:
            price *= upgrade_level_multiplier[pilot->leg_speed + 1] * arm_leg_multiplier;
            break;
        case ARMOR:
            price *= upgrade_level_multiplier[pilot->armor + 1] * armor_multiplier;
            break;
        case STUN_RES:
            price *= upgrade_level_multiplier[pilot->stun_resistance + 1] * stun_res_multiplier;
            break;
        default:
            abort();
    }
    return price;
}

void upgrade_har(sd_pilot *pilot, enum HAR_UPGRADE upgrade) {
    int32_t price = upgrade_price(pilot, upgrade);
    assert(pilot->money >= price);
    assert(har_can_upgrade(pilot, upgrade));

    pilot->money -= price;
    switch(upgrade) {
        case ARM_POWER:
            pilot->arm_power++;
            break;
        case ARM_SPEED:
            pilot->arm_speed++;
            break;
        case LEG_POWER:
            pilot->leg_power++;
            break;
        case LEG_SPEED:
            pilot->leg_speed++;
            break;
        case ARMOR:
            pilot->armor++;
            break;
        case STUN_RES:
            pilot->stun_resistance++;
            break;
        default:
            abort();
    }
}

static void update_total_value(sd_pilot *pilot) {
    int value = har_prices[pilot->har_id];
    for(int i = 1; i < pilot->arm_power; i++) {
        value += har_upgrade_price[pilot->har_id] * upgrade_level_multiplier[i] * arm_leg_multiplier;
    }

    for(int i = 1; i < pilot->arm_speed; i++) {
        value += har_upgrade_price[pilot->har_id] * upgrade_level_multiplier[i] * arm_leg_multiplier;
    }

    for(int i = 1; i < pilot->leg_power; i++) {
        value += har_upgrade_price[pilot->har_id] * upgrade_level_multiplier[i] * arm_leg_multiplier;
    }

    for(int i = 1; i < pilot->leg_speed; i++) {
        value += har_upgrade_price[pilot->har_id] * upgrade_level_multiplier[i] * arm_leg_multiplier;
    }

    for(int i = 1; i < pilot->armor; i++) {
        value += har_upgrade_price[pilot->har_id] * upgrade_level_multiplier[i] * armor_multiplier;
    }

    for(int i = 1; i < pilot->stun_resistance; i++) {
        value += har_upgrade_price[pilot->har_id] * upgrade_level_multiplier[i] * stun_res_multiplier;
    }

    pilot->total_value = (value * 70) / 100;
}

static int rank_additions[] = {
    500, 200, 160, 120, 100, 90, 80, 70, 60, 55, 50, 45, 40, 35, 30, 25, 20,
    17,  15,  14,  13,  12,  11, 11, 10, 9,  8,  8,  7,  7,  6,  6,  5,  0,
};

static float rank_scale(unsigned rank) {
    assert(rank >= 1);
    rank -= 1;
    float a = rank * 0.1 + 1.0;
    float b = rank * 0.2 + 1.0;
    return b * a * a;
}

static float rank_bonus(unsigned rank) {
    assert(rank >= 1);
    rank -= 1;
    if(rank < N_ELEMENTS(rank_additions)) {
        return rank_additions[rank];
    }
    return 5.0;
}

float calculate_winnings(sd_pilot *winner, sd_pilot *loser, float winnings_multiplier) {
    update_total_value(winner);
    update_total_value(loser);

    int adjusted_value = winner->total_value - (har_prices[0] * 85) / 100 - 500;
    float loser_value = (loser->total_value + loser->money) / 45.0 + (int)loser->winnings;
    float winnings = loser_value + (adjusted_value / rank_scale(winner->rank) / 30.0);

    winnings += winner->trn_rank_money * rank_bonus(winner->rank);
    winnings *= 0.7f;
    winnings *= winnings_multiplier;
    return winnings;
}
