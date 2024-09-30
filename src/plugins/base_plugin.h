#ifndef BASE_PLUGIN_H
#define BASE_PLUGIN_H

typedef union {
    void *ptr;
    const char *(*fn)(void);
} SDL_fn_ptr;

typedef struct {
    void *handle;
    SDL_fn_ptr get_name;
    SDL_fn_ptr get_author;
    SDL_fn_ptr get_license;
    SDL_fn_ptr get_type;
    SDL_fn_ptr get_version;
} base_plugin;

#endif // BASE_PLUGIN_H
