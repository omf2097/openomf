#ifndef _SCRAP_H
#define _SCRAP_H

#include "resources/animation.h"
#include "game/protos/object.h"

typedef struct scrap_local_t {
	int tick;
} scrap_local;

int scrap_create(object *obj);
void scrap_tick(object *obj); 
void scrap_act(object *obj, int act_type);
void scrap_free(object *obj);

#endif // _SCRAP_H