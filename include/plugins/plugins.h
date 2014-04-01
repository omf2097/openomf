#ifndef _PLUGINS_H
#define _PLUGINS_H

#include "utils/list.h"

typedef struct {
    void *handle;
    const char* (*get_name)();
    const char* (*get_author)();
    const char* (*get_license)();
    const char* (*get_type)();
} base_plugin;

typedef struct {
    base_plugin *base;
    int (*is_factor_available)(int factor);
    int (*get_factors_list)(const int** factors);
    int (*get_color_format)();
    int (*scale)(const char* in, char* out, int w, int h, int factor);
} scaler_plugin;

void plugins_init();
void plugins_close();

scaler_plugin plugins_get_scaler(const char* name);
int plugins_get_list_by_type(list *tlist, const char* type);

#endif