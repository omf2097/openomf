#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <SDL2/SDL.h>

#if defined(_WIN32) || defined(WIN32)
#include <shlobj.h> //SHCreateDirectoryEx
#else
#include <sys/stat.h> // mkdir
#endif

#include "resources/pathmanager.h"
#include "utils/log.h"

// Files
static const char* logfile_name = "openomf.log";
static const char* configfile_name = "openomf.conf";
static const char* scorefile_name = "SCORES.DAT";
static const char* savegamedir_name = "save/";
static char errormessage[128];

// Lists
static char* local_paths[NUMBER_OF_LOCAL_PATHS];
static char* resource_paths[NUMBER_OF_RESOURCES];

// Build directory
static void local_path_build(int path_id, const char *path, const char *ext) {
    int len = strlen(path) + strlen(ext) + 1;
    local_paths[path_id] = malloc(len);
    sprintf(local_paths[path_id], "%s%s", path, ext);
}
static void resource_path_build(int path_id, const char *path, const char *ext) {
    int len = strlen(path) + strlen(ext) + 1;
    resource_paths[path_id] = malloc(len);
    sprintf(resource_paths[path_id], "%s%s", path, ext);
}

// Makes sure resource file exists
int pm_validate_resources() {
    for(int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        const char *testfile = pm_get_resource_path(i);
        if(access(testfile, F_OK) == -1) {
            sprintf(errormessage, "Missing file %s.", testfile);
            return 1;
        }
    }
    return 0;
}

int pm_init() {
    char *local_base_dir;
    char *bin_base_dir;

    // Clear everything
    errormessage[0] = 0;

    // Find local basedir
    local_base_dir = pm_get_local_base_dir();
    if(local_base_dir == NULL) {
        goto error_0;
    }

    // Other paths
    local_path_build(LOG_PATH, local_base_dir, logfile_name);
    local_path_build(CONFIG_PATH, local_base_dir, configfile_name);
    local_path_build(SCORE_PATH, local_base_dir, scorefile_name);
    local_path_build(SAVE_PATH, local_base_dir, savegamedir_name);

    // Set default base dirs for resources and plugins
    int m_ok = 0;
    if(pm_in_release_mode()) {
        // where is the openomf binary, if this call fails we will look for resources in ./resources
        bin_base_dir = SDL_GetBasePath();
        if(bin_base_dir != NULL) {
            if(!strcasecmp(SDL_GetPlatform(), "Windows")) {
                // on windows, the resources will be in ./resources, relative to the binary
                local_path_build(RESOURCE_PATH, bin_base_dir, "resources\\");
                local_path_build(PLUGIN_PATH, bin_base_dir, "plugins\\");
                m_ok = 1;
            } else if(!strcasecmp(SDL_GetPlatform(), "Linux")) {
                // on linux, the resources will be in ../share/openomf, relative to the binary
                // so if openomf is installed to /usr/local/bin,
                // the resources will be in /usr/local/share/openomf
                local_path_build(RESOURCE_PATH, bin_base_dir, "../share/openomf/");
                local_path_build(PLUGIN_PATH, bin_base_dir, "../lib/openomf/");
                m_ok = 1;
            } else if(!strcasecmp(SDL_GetPlatform(), "Mac OS X")) {
                // on OSX, GetBasePath returns the 'Resources' directory
                // if run from an app bundle, so we can use this as-is
                local_path_build(RESOURCE_PATH, bin_base_dir, "");
                local_path_build(PLUGIN_PATH, bin_base_dir, "plugins/");
                m_ok = 1;
            }
            // any other platform will look in ./resources
            SDL_free(bin_base_dir);
        }
    }

    // Set default resource and plugins paths
    if(!m_ok) {
        if(!strcasecmp(SDL_GetPlatform(), "Windows")) {
            local_path_build(RESOURCE_PATH, "resources\\", "");
            local_path_build(PLUGIN_PATH, "plugins\\", "");
        } else {
            local_path_build(RESOURCE_PATH, "resources/", "");
            local_path_build(PLUGIN_PATH, "plugins/", "");
        }
    }

    // Set resource paths
    for(int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        resource_path_build(i,
            pm_get_local_path(RESOURCE_PATH),
            get_resource_file(i));
    }

    // Check resources
    if(pm_validate_resources() != 0) {
        goto error_1;
    }

    // All done
    free(local_base_dir);
    return 0;

error_1:
    pm_free();
error_0:
    return 1;
}

void pm_log() {
    // Debug info
    for(unsigned int i = 0; i < NUMBER_OF_LOCAL_PATHS; i++) {
        DEBUG("%s: %s",
            pm_get_local_path_type_name(i),
            pm_get_local_path(i));
    }
}

void pm_free() {
    for(int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        free(resource_paths[i]);
    }
    for(int i = 0; i < NUMBER_OF_LOCAL_PATHS; i++) {
        free(local_paths[i]);
    }
}

const char* pm_get_errormsg() {
    return errormessage;
}

int pm_in_debug_mode() {
#ifdef DEBUGMODE
    return 1;
#else
    return 0;
#endif
}

int pm_in_release_mode() {
    if(!pm_in_portable_mode() && !pm_in_debug_mode()) {
        return 1;
    }
    return 0;
}

int pm_in_portable_mode() {
    if(access(configfile_name, F_OK) != -1) {
        return 1;
    }
    return 0;
}

char* pm_get_local_base_dir() {
    char *out = NULL;
    if(pm_in_portable_mode()) {
        out = malloc(1);
        out[0] = 0;
        return out;
    }

    // Attempt to open up locally writable directory
    char *sdl_path = SDL_GetPrefPath("openomfproject", "OpenOMF");
    if(sdl_path == NULL) {
        sprintf(errormessage, "Error getting config path: %s", SDL_GetError());
        return NULL;
    }
    out = malloc(strlen(sdl_path)+1);
    strcpy(out, sdl_path);
    SDL_free(sdl_path);

    // Ensure the path exists before continuing on
    // XXX shouldn't SDL_GetPrefPath automatically create the path if it doesn't exist?
#if defined(_WIN32) || defined(WIN32)
    int sherr = SHCreateDirectoryEx(NULL, out, NULL);
    if(sherr == ERROR_FILE_EXISTS) {
        sprintf(errormessage, "Please delete this file and relaunch OpenOMF: %s", out);
        return NULL;
    } else if(sherr != ERROR_SUCCESS && sherr != ERROR_ALREADY_EXISTS) {
        sprintf(errormessage, "Failed to create config path: %s", out);
        return NULL;
    }
#endif

    // All done
    return out;
}

const char* pm_get_local_path(unsigned int local_id) {
    if(local_id >= NUMBER_OF_LOCAL_PATHS) {
        return NULL;
    }
    return local_paths[local_id];
}

const char* pm_get_resource_path(unsigned int resource_id) {
    if(resource_id >= NUMBER_OF_RESOURCES) {
        return NULL;
    }
    return resource_paths[resource_id];
}

const char* pm_get_local_path_type_name(unsigned int path_id) {
    switch(path_id) {
        case RESOURCE_PATH: return "RESOURCE_PATH";
        case PLUGIN_PATH: return "PLUGIN_PATH";
        case CONFIG_PATH: return "CONFIG_PATH";
        case LOG_PATH: return "LOG_PATH";
        case SCORE_PATH: return "SCORE_PATH";
        case SAVE_PATH: return "SAVE_PATH";
    }
    return "UNKNOWN";
}

int pm_create_dir(const char* dirname) {
    #if defined(_WIN32) || defined(WIN32)
    if(SHCreateDirectoryEx(NULL, dirname, NULL) != ERROR_SUCCESS) {
        PERROR("Error while attempting to create directory '%s'.", dirname);
        return 1;
    }
    #else
    if(mkdir(dirname, 0644) != 0) {
        PERROR("Error while attempting to create directory '%s'.", dirname);
        return 1;
    }
    #endif
    return 0;
}
