#include "resources/plugins.h"
#include "resources/global_paths.h"
#include "utils/scandir.h"
#include "utils/list.h"
#include "utils/log.h"

static list _plugins[NUMBER_OF_PLUGIN_TYPES];

static const char* plugin_prefixes[NUMBER_OF_PLUGIN_TYPES] = {
    "scaler_",
    "bot_",
};

const char* plugins_get_type_name(int type) {
    switch(type) {
        case PLUGIN_SCALER: return "SCALER";
        case PLUGIN_BOT: return "BOT";
    }
    return "UNKNOWN";
}

void plugins_init() {
    INFO("Looking for plugins ...");
    int plugin_count = 0;
    for(int i = 0; i < NUMBER_OF_PLUGIN_TYPES; i++) {
        list_create(&_plugins[i]);
        if(scandir_prefix(&_plugins[i], global_path_get(PLUGIN_PATH), plugin_prefixes[i])) {
            PERROR("Error while attempting to open plugin directory.");
            return;
        }

        // Pring debug
        if(list_size(&_plugins[i]) > 0) {
            iterator it;
            list_iter_begin(&_plugins[i], &it);
            char *plugin_file;
            DEBUG(" * %s plugins found:", plugins_get_type_name(i));
            while((plugin_file = iter_next(&it)) != NULL) {
                DEBUG("    - %s", plugin_file);
                plugin_count++;
            }
        }
    }

    // Print some information
    if(plugin_count > 0) {
        INFO("%d plugins found.", plugin_count);
    } else {
        INFO("No plugins found.");
    }
}

void plugins_close() {
    for(int i = 0; i < NUMBER_OF_PLUGIN_TYPES; i++) {
        list_free(&_plugins[i]);
    }
    DEBUG("Plugins closed.");
}
