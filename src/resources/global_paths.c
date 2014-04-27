#include "resources/global_paths.h"
#include "utils/log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char* _paths[NUMBER_OF_PATHS];

const char* global_path_name(int path_id) {
    switch(path_id) {
        case RESOURCE_PATH: return "RESOURCE_PATH";
        case PLUGIN_PATH: return "PLUGIN_PATH";
        case CONFIG_PATH: return "CONFIG_PATH";
        case LOG_PATH: return "LOG_PATH";
        case SCORE_PATH: return "SCORES_PATH";
    }
    return "UNKNOWN";
}

void global_paths_print_debug() {
    for(int i = 0; i < NUMBER_OF_PATHS; i++) {
        if(_paths[i] != NULL) {
            DEBUG("%s: %s", global_path_name(i), _paths[i]);
        } else {
            DEBUG("%s: <unset>", global_path_name(i));
        }
    }
}

void global_paths_init() {
    for(int i = 0; i < NUMBER_OF_PATHS; i++) {
        _paths[i] = NULL;
    }
}

void global_paths_close() {
    for(int i = 0; i < NUMBER_OF_PATHS; i++) {
        if(_paths[i] != NULL) {
            free(_paths[i]);
        }
    }
}

void global_path_build(int path_id, const char *path, const char *ext) {
    int len = strlen(path) + strlen(ext) + 1;
    char temp[len];
    sprintf(temp, "%s%s", path, ext);
    global_path_set(path_id, temp);
}

void global_path_set(int path_id, const char* path) {
    if(path_id < 0 || path_id >= NUMBER_OF_PATHS) {
        return;
    }
    if(_paths[path_id] != NULL) {
        free(_paths[path_id]);
    }
    _paths[path_id] = malloc(strlen(path)+1);
    strcpy(_paths[path_id], path);
}

const char* global_path_get(int path_id) {
    if(path_id < 0 || path_id >= NUMBER_OF_PATHS) {
        return NULL;
    }
    return _paths[path_id];
}