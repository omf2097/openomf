#ifndef REC_CONTROLLER_H
#define REC_CONTROLLER_H

#include "controller/controller.h"
#include "formats/rec.h"
#include "utils/hashmap.h"

void rec_controller_create(controller *ctrl, int player, sd_rec_file *rec);
void rec_controller_free(controller *ctrl);

#endif // REC_CONTROLLER_H
