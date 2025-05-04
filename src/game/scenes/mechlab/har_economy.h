#ifndef HAR_ECONOMY_H
#define HAR_ECONOMY_H

#include "formats/pilot.h"

static const uint8_t max_arm_speed[11] = {6, 8, 4, 6, 9, 7, 8, 6, 9, 6, 7};
static const uint8_t max_leg_speed[11] = {8, 9, 5, 6, 8, 8, 7, 6, 7, 5, 6};
static const uint8_t max_arm_power[11] = {5, 5, 9, 8, 4, 6, 6, 5, 5, 6, 7};
static const uint8_t max_leg_power[11] = {6, 6, 8, 4, 5, 7, 5, 7, 6, 7, 7};
static const uint8_t max_stun_res[11] = {8, 6, 8, 6, 7, 6, 6, 7, 7, 7, 6};
static const uint8_t max_armor[11] = {5, 7, 7, 8, 6, 8, 6, 9, 6, 6, 7};

static const int32_t har_prices[11] = {20000, 36000, 26000, 28000, 29000, 32000, 25000, 30000, 24000, 22000, 75000};
static const int32_t har_upgrade_price[11] = {380, 400, 350, 400, 500, 330, 420, 370, 450, 360, 700};
static const int32_t upgrade_level_multiplier[10] = {0, 1, 3, 7, 12, 18, 30, 50, 75, 120};

static const int32_t arm_leg_multiplier = 2;
static const int32_t stun_res_multiplier = 3;
static const int32_t armor_multiplier = 5;

void purchase_random_har(sd_pilot *pilot);

#endif // HAR_ECONOMY_H
