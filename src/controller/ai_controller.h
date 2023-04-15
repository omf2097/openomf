#ifndef AI_CONTROLLER_H
#define AI_CONTROLLER_H

#include "formats/pilot.h"

typedef struct controller_t controller;

void ai_controller_free(controller *ctrl);
void ai_controller_create(controller *ctrl, int difficulty, sd_pilot *pilot, int pilot_id);

#endif // AI_CONTROLLER_H
