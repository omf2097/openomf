#ifndef _PLUGINS_H
#define _PLUGINS_H

enum {
    PLUGIN_SCALER = 0,
    PLUGIN_BOT,
    NUMBER_OF_PLUGIN_TYPES
};

void plugins_init();
void plugins_close();

#endif