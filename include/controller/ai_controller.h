#ifndef _AI_H
#define _AI_H

typedef struct controller_t controller;

void ai_controller_free(controller *ctrl);
void ai_controller_create(controller *ctrl, int difficulty);

#endif
