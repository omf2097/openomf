#ifndef _SGMANAGER_H
#define _SGMANAGER_H

#include <shadowdive/pilot.h>

int sg_init();
int sg_load(sd_pilot *pilot, const char* pilotname);
int sd_save(const sd_pilot *pilot, const char* pilotname);

#endif // _SGMANAGER_H
