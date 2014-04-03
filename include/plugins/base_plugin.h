#ifndef _BASE_PLUGIN
#define _BASE_PLUGIN

typedef struct {
    void *handle;
    const char* (*get_name)();
    const char* (*get_author)();
    const char* (*get_license)();
    const char* (*get_type)();
} base_plugin;

#endif // _BASE_PLUGIN