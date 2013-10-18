#ifndef _SCRAP_H
#define _SCRAP_H

#include "game/protos/object.h"

typedef struct scrap_local_t {
	int tick;
} scrap_local;

int scrap_create(object *obj);

#endif // _SCRAP_H