#ifndef TRNMANAGER_H
#define TRNMANAGER_H

#include "formats/tournament.h"
#include "utils/list.h"

list *trnlist_init(void);
int trn_load(sd_tournament_file *trn, const char *trnname);

#endif // TRNMANAGER_H
