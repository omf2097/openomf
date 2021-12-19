#ifndef BASE_PLUGIN
#define BASE_PLUGIN

typedef struct {
    void *handle;
    const char* (*get_name)();
    const char* (*get_author)();
    const char* (*get_license)();
    const char* (*get_type)();
    const char* (*get_version)();
} base_plugin;

#endif // BASE_PLUGIN
