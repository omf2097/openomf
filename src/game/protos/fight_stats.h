#ifndef FIGHT_STATS_H
#define FIGHT_STATS_H

typedef struct fight_stats_t {
    int winnings;
    int bonuses;
    int repair_cost;
    int profit;

    unsigned hits_landed[2];
    float average_damage[2];
    unsigned total_attacks[2];
    unsigned hit_miss_ratio[2];
} fight_stats;

#endif // FIGHT_STATS_H
