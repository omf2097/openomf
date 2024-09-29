#ifndef SCALER_PLUGIN_H
#define SCALER_PLUGIN_H

#include "plugins/base_plugin.h"

typedef struct {
    base_plugin *base;
    int (*is_factor_available)(int factor);
    int (*get_factors_list)(int **factors);
    int (*get_color_format)(void);
    int (*scale)(const char *in, char *out, int w, int h, int factor);
} scaler_plugin;

void scaler_init(scaler_plugin *scaler);
int scaler_is_factor_available(scaler_plugin *scaler, int factor);
int scaler_get_factors_list(scaler_plugin *scaler, int **factors);
int scaler_get_color_format(scaler_plugin *scaler);
int scaler_scale(scaler_plugin *scaler, const char *in, char *out, int w, int h, int factor);

#endif // SCALER_PLUGIN_H
