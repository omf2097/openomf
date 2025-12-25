#include "game/scenes/mechlab/har_economy.h"
#include "formats/pilot.h"
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
