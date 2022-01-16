
#include "resources/pilots.h"

void pilot_get_info(pilot *pilot, int id) {
    // TODO read this from MASTER.DAT
    // XXX the colors are eyeballed
    switch(id) {
        case 0:
            pilot->power = 5;
            pilot->agility = 16;
            pilot->endurance = 9;
            pilot->colors[0] = 5;
            pilot->colors[1] = 11;
            pilot->colors[2] = 15;
            break;
        case 1:

            pilot->power = 13;
            pilot->agility = 9;
            pilot->endurance = 8;
            pilot->colors[0] = 10;
            pilot->colors[1] = 15;
            pilot->colors[2] = 7;
            break;
        case 2:
            pilot->power = 7;
            pilot->agility = 20;
            pilot->endurance = 4;
            pilot->colors[0] = 11;
            pilot->colors[1] = 12;
            pilot->colors[2] = 8;
            break;
        case 3:
            pilot->power = 9;
            pilot->agility = 7;
            pilot->endurance = 15;
            pilot->colors[0] = 8;
            pilot->colors[1] = 15;
            pilot->colors[2] = 12;
            break;
        case 4:
            pilot->power = 20;
            pilot->agility = 1;
            pilot->endurance = 8;
            pilot->colors[0] = 4;
            pilot->colors[1] = 7;
            pilot->colors[2] = 14;
            break;
        case 5:
            pilot->power = 9;
            pilot->agility = 10;
            pilot->endurance = 11;
            pilot->colors[0] = 1;
            pilot->colors[1] = 7;
            pilot->colors[2] = 6;
            break;
        case 6:
            pilot->power = 10;
            pilot->agility = 1;
            pilot->endurance = 20;
            pilot->colors[0] = 8;
            pilot->colors[1] = 6;
            pilot->colors[2] = 14;
            break;
        case 7:
            pilot->power = 7;
            pilot->agility = 10;
            pilot->endurance = 13;
            pilot->colors[0] = 0;
            pilot->colors[1] = 15;
            pilot->colors[2] = 7;
            break;
        case 8:
            pilot->power = 14;
            pilot->agility = 8;
            pilot->endurance = 8;
            pilot->colors[0] = 0;
            pilot->colors[1] = 8;
            pilot->colors[2] = 2;
            break;
        case 9:
            pilot->power = 14;
            pilot->agility = 4;
            pilot->endurance = 12;
            pilot->colors[0] = 9;
            pilot->colors[1] = 10;
            pilot->colors[2] = 4;
            break;
        case 10:
            // totally made up shit here
            pilot->power = 14;
            pilot->agility = 14;
            pilot->endurance = 14;
            pilot->colors[0] = 5;
            pilot->colors[1] = 1;
            pilot->colors[2] = 15;
            break;
    }
}
