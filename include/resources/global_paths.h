#ifndef _PATHS_H
#define _PATHS_H

enum {
    RESOURCE_PATH = 0,
    PLUGIN_PATH,
    LOG_PATH,
    CONFIG_PATH,
    SCORE_PATH,
    NUMBER_OF_PATHS
};

void global_paths_init();
void global_paths_close();
void global_paths_print_debug();

void global_path_build(int path_id, const char* path, const char *ext);
void global_path_set(int path_id, const char* path);
const char* global_path_get(int path_id);

#endif // _PATHS_H