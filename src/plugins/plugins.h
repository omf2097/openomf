#ifndef PLUGINS_H
#define PLUGINS_H

#include "plugins/base_plugin.h"
#include "plugins/scaler_plugin.h"
#include "utils/list.h"

void plugins_init(void);
void plugins_close(void);

int plugins_get_scaler(scaler_plugin *scaler, const char *name);

int plugins_get_list_by_type(list *tlist, const char *type);

#endif // PLUGINS_H
