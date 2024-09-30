#include "plugins/plugins.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/list.h"
#include "utils/log.h"
#include "utils/scandir.h"
#include <SDL2/SDL_loadso.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PLUGIN_MAX_COUNT 128
static base_plugin _plugins[PLUGIN_MAX_COUNT];
static int _plugins_count;

void plugins_init(void) {
    // Zero out plugin list
    _plugins_count = 0;
    for(int i = 0; i < PLUGIN_MAX_COUNT; i++) {
        _plugins[i].handle = NULL;
        _plugins[i].get_name = NULL;
        _plugins[i].get_author = NULL;
        _plugins[i].get_license = NULL;
        _plugins[i].get_type = NULL;
        _plugins[i].get_version = NULL;
    }

    // Search for plugins
    INFO("Looking for plugins ...");
    list scanned;

    // Find all files from the plugin path
    list_create(&scanned);
    const char *plugin_dir = pm_get_local_path(PLUGIN_PATH);
    if(scan_directory(&scanned, plugin_dir)) {
        PERROR("Could not scan directory %s: %s", plugin_dir, strerror(errno));
        return;
    }

    // Walk through all the plugins, search for valid files
    if(list_size(&scanned) > 0) {
        iterator it;
        list_iter_begin(&scanned, &it);
        char *plugin_file;
        char *plugin_path;
        void *handle;
        while((plugin_file = iter_next(&it)) != NULL) {
            // Skip for .. and . :)
            if(strlen(plugin_file) <= 2) {
                continue;
            }

            // Open the plugin file
            int psize = strlen(plugin_dir) + strlen(plugin_file) + 1;
            plugin_path = omf_calloc(psize, 1);
            snprintf(plugin_path, psize, "%s%s", plugin_dir, plugin_file);
            handle = SDL_LoadObject(plugin_path);
            omf_free(plugin_path);

            // Check for errors in plugin open
            if(handle == NULL) {
                PERROR("Plugin file %s could not be opened: %s", plugin_file, SDL_GetError());
                continue;
            }

            // Build plugin
            _plugins[_plugins_count].handle = handle;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
            _plugins[_plugins_count].get_name = SDL_LoadFunction(handle, "plugin_get_name");
            _plugins[_plugins_count].get_author = SDL_LoadFunction(handle, "plugin_get_author");
            _plugins[_plugins_count].get_license = SDL_LoadFunction(handle, "plugin_get_license");
            _plugins[_plugins_count].get_type = SDL_LoadFunction(handle, "plugin_get_type");
            _plugins[_plugins_count].get_version = SDL_LoadFunction(handle, "plugin_get_version");
#pragma GCC diagnostic pop

            // Make sure we have all functions
            if(_plugins[_plugins_count].get_name == NULL) {
                PERROR("Plugin get_name handle not found: %s", SDL_GetError());
                continue;
            }
            if(_plugins[_plugins_count].get_author == NULL) {
                PERROR("Plugin get_author handle not found: %s", SDL_GetError());
                continue;
            }
            if(_plugins[_plugins_count].get_license == NULL) {
                PERROR("Plugin get_license handle not found: %s", SDL_GetError());
                continue;
            }
            if(_plugins[_plugins_count].get_type == NULL) {
                PERROR("Plugin get_type handle not found: %s", SDL_GetError());
                continue;
            }
            if(_plugins[_plugins_count].get_version == NULL) {
                DEBUG("Plugin get_version handle not found; your plugin is old.");
            }
#ifdef DEBUGMODE
            // Print some debug information
            base_plugin *tmp = &_plugins[_plugins_count];
            DEBUG(" * File: %s", plugin_file);
            DEBUG("   - Name: %s", tmp->get_name());
            DEBUG("   - Author: %s", tmp->get_author());
            DEBUG("   - License: %s", tmp->get_license());
            DEBUG("   - Type: %s", tmp->get_type());
            if(tmp->get_version) {
                DEBUG("   - Version: %s", tmp->get_version());
            }
#endif
            _plugins_count++;
        }
    }

    // Free up the temporary list
    list_free(&scanned);

    // Print some information
    INFO("%d plugins found.", _plugins_count);
}

int plugins_get_scaler(scaler_plugin *scaler, const char *name) {
    // Search for a scaler with given name
    for(int i = 0; i < PLUGIN_MAX_COUNT; i++) {
        if(_plugins[i].handle != NULL && strcmp(_plugins[i].get_name(), name) == 0 &&
           strcmp(_plugins[i].get_type(), "scaler") == 0) {
            scaler->base = &_plugins[i];
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
            scaler->is_factor_available = SDL_LoadFunction(scaler->base->handle, "scaler_is_factor_available");
            scaler->get_factors_list = SDL_LoadFunction(scaler->base->handle, "scaler_get_factors_list");
            scaler->get_color_format = SDL_LoadFunction(scaler->base->handle, "scaler_get_color_format");
            scaler->scale = SDL_LoadFunction(scaler->base->handle, "scaler_handle");
#pragma GCC diagnostic pop
            return 0;
        }
    }
    return 1;
}

int plugins_get_list_by_type(list *tlist, const char *type) {
    // Search for a scaler with given type
    int count = 0;
    for(int i = 0; i < PLUGIN_MAX_COUNT; i++) {
        if(_plugins[i].handle != NULL && strcmp(_plugins[i].get_type(), type) == 0) {
            void *ptr = &_plugins[i];
            list_append(tlist, &ptr, sizeof(base_plugin *));
            count++;
        }
    }
    return count;
}

void plugins_close(void) {
    for(int i = 0; i < PLUGIN_MAX_COUNT; i++) {
        if(_plugins[i].handle != NULL) {
            SDL_UnloadObject(_plugins[i].handle);
            _plugins[i].handle = NULL;
        }
    }
}
