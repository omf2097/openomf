#ifndef AI_CONTROLLER_H
#define AI_CONTROLLER_H

typedef struct controller_t controller;

void ai_controller_free(controller *ctrl);
void ai_controller_create(controller *ctrl, int difficulty);

#endif // AI_CONTROLLER_H
