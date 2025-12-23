#ifndef PILOTS_H
#define PILOTS_H

#include <stdint.h>

typedef struct pilot_t {
    int power, agility, endurance;
    uint8_t color_3; ///< HAR Tertiary Color.
    uint8_t color_2; ///< HAR Secondary Color.
    uint8_t color_1; ///< HAR Primary Color.
    void *userdata;
    int sex;
} pilot;

void pilot_get_info(pilot *pilot, int id);

#endif // PILOTS_H
