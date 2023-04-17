#ifndef TRNMANAGER_H
#define TRNMANAGER_H

#include "formats/tournament.h"

sd_tournament_file** trnlist_init();
int trn_load(sd_tournament_file *trn, const char *trnname);

#endif // TRNMANAGER_H
