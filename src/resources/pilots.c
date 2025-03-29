
#include "resources/pilots.h"
#include "game/common_defines.h"

void pilot_get_info(pilot *pilot, int id) {
    // TODO read this from MASTER.DAT
    // XXX the colors are eyeballed
    pilot->sex = PILOT_SEX_MALE;
    switch(id) {
        case PILOT_CRYSTAL:
            pilot->power = 5;
            pilot->agility = 16;
            pilot->endurance = 9;
            pilot->color_1 = 5;
            pilot->color_2 = 11;
            pilot->color_3 = 8;
            pilot->sex = PILOT_SEX_FEMALE;
            break;
        case PILOT_STEFFAN:
            pilot->power = 13;
            pilot->agility = 9;
            pilot->endurance = 8;
            pilot->color_1 = 10;
            pilot->color_2 = 15;
            pilot->color_3 = 7;
            break;
        case PILOT_MILANO:
            pilot->power = 7;
            pilot->agility = 20;
            pilot->endurance = 4;
            pilot->color_1 = 11;
            pilot->color_2 = 12;
            pilot->color_3 = 7;
            break;
        case PILOT_CHRISTIAN:
            pilot->power = 9;
            pilot->agility = 7;
            pilot->endurance = 15;
            pilot->color_1 = 8;
            pilot->color_2 = 15;
            pilot->color_3 = 6;
            break;
        case PILOT_SHIRRO:
            pilot->power = 20;
            pilot->agility = 1;
            pilot->endurance = 8;
            pilot->color_1 = 4;
            pilot->color_2 = 7;
            pilot->color_3 = 14;
            break;
        case PILOT_JEANPAUL:
            pilot->power = 9;
            pilot->agility = 10;
            pilot->endurance = 11;
            pilot->color_1 = 1;
            pilot->color_2 = 7;
            pilot->color_3 = 6;
            break;
        case PILOT_IBRAHIM:
            pilot->power = 10;
            pilot->agility = 1;
            pilot->endurance = 20;
            pilot->color_1 = 8;
            pilot->color_2 = 6;
            pilot->color_3 = 14;
            break;
        case PILOT_ANGEL:
            pilot->power = 7;
            pilot->agility = 10;
            pilot->endurance = 13;
            pilot->color_1 = 0;
            pilot->color_2 = 15;
            pilot->color_3 = 7;
            pilot->sex = PILOT_SEX_FEMALE;
            break;
        case PILOT_COSSETTE:
            pilot->power = 14;
            pilot->agility = 8;
            pilot->endurance = 8;
            pilot->color_1 = 0;
            pilot->color_2 = 8;
            pilot->color_3 = 2;
            pilot->sex = PILOT_SEX_FEMALE;
            break;
        case PILOT_RAVEN:
            pilot->power = 14;
            pilot->agility = 4;
            pilot->endurance = 12;
            pilot->color_1 = 9;
            pilot->color_2 = 10;
            pilot->color_3 = 4;
            break;
        case PILOT_KREISSACK:
            // totally made up shit here
            pilot->power = 14;
            pilot->agility = 14;
            pilot->endurance = 14;
            pilot->color_1 = 5;
            pilot->color_2 = 1;
            pilot->color_3 = 15;
            break;
    }
}
