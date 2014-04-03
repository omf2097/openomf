#ifndef _SCALER_PLUGIN
#define _SCALER_PLUGIN

#include "plugins/base_plugin.h"

typedef struct {
    base_plugin *base;
    int (*is_factor_available)(int factor);
    int (*get_factors_list)(const int** factors);
    int (*get_color_format)();
    int (*scale)(const char* in, char* out, int w, int h, int factor);
} scaler_plugin;

# endif // _SCALER_PLUGIN