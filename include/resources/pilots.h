#ifndef PILOTS_H
#define PILOTS_H

typedef struct pilot_t {
    int power, agility, endurance;
    char colors[3];
    void *userdata;
} pilot;

void pilot_get_info(pilot *pilot, int id);

#endif // PILOTS_H
