#ifndef PLUGINS_H
#define PLUGINS_H

#include "utils/list.h"
#include "plugins/base_plugin.h"
#include "plugins/scaler_plugin.h"

void plugins_init();
void plugins_close();

int plugins_get_scaler(scaler_plugin *scaler, const char* name);

int plugins_get_list_by_type(list *tlist, const char* type);

#endif // PLUGINS_H
