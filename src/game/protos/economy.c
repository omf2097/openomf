#include "game/protos/economy.h"
#include "game/scenes/mechlab/lab_har_constants.h"

void update_total_value(sd_pilot *pilot) {
    int value = har_prices[pilot->har_id];
    for(int i = 1; i < pilot->arm_power; i++) {
        value += har_upgrade_price[pilot->har_id] * upgrade_level_multiplier[i] * arm_leg_multiplier;
    }

    for(int i = 1; i < pilot->leg_power; i++) {
        value += har_upgrade_price[pilot->har_id] * upgrade_level_multiplier[i] * arm_leg_multiplier;
    }

    for(int i = 1; i < pilot->arm_speed; i++) {
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

    pilot->total_value = (value * 85) / 100;
}
