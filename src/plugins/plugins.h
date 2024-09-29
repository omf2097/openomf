#ifndef PLUGINS_H
#define PLUGINS_H

#include "plugins/base_plugin.h"
#include "utils/list.h"

void plugins_init(void);
void plugins_close(void);

int plugins_get_list_by_type(list *type_list, const char *type);

#endif // PLUGINS_H
