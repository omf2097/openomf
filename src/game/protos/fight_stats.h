#ifndef FIGHT_STATS_H
#define FIGHT_STATS_H

#include "formats/pilot.h"

#define SOLD_BUF_SIZE 24

typedef enum
{
    FINISH_NONE = 0,
    FINISH_SCRAP,
    FINISH_DESTRUCTION
} fight_finisher;

typedef struct fight_stats_t {
    int winner;
    unsigned plug_text;
    char sold[SOLD_BUF_SIZE];

    int winnings;
    int bonuses;
    int repair_cost;
    int profit;
    int hp;
    int max_hp;

    fight_finisher finish;

    sd_pilot *challenger;

    unsigned hits_landed[2];
    float average_damage[2];
    unsigned total_attacks[2];
    unsigned hit_miss_ratio[2];
} fight_stats;

#define PLUG_TEXT_START 587
#define PLUG_ENHANCEMENT 0
#define PLUG_FORFEIT 1
#define PLUG_LOSE 2
#define PLUG_WIN 7
#define PLUG_WIN_OK 10
#define PLUG_WIN_BIG 13
#define PLUG_WARNING 16
#define PLUG_KICK_OUT 17
#define PLUG_SOLD_UPGRADE 18

#endif // FIGHT_STATS_H
