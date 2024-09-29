#ifndef BASE_PLUGIN_H
#define BASE_PLUGIN_H

typedef struct {
    void *handle;
    const char *(*get_name)(void);
    const char *(*get_author)(void);
    const char *(*get_license)(void);
    const char *(*get_type)(void);
    const char *(*get_version)(void);
} base_plugin;

#endif // BASE_PLUGIN_H
