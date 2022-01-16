#ifndef SGMANAGER_H
#define SGMANAGER_H

#include "formats/pilot.h"

int sg_init();
int sg_load(sd_pilot *pilot, const char *pilotname);
int sd_save(const sd_pilot *pilot, const char *pilotname);

#endif // SGMANAGER_H
