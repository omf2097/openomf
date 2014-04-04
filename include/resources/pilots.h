#ifndef _PILOTS_H
#define _PILOTS_H

typedef struct pilot_t {
    int power, agility, endurance;
    int colors[3];
    void *userdata;
} pilot;

void pilot_get_info(pilot *pilot, int id);

#endif // _PILOTS_H
