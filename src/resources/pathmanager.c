#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(WIN32)
#include <shlobj.h>  //SHCreateDirectoryEx
#include <shlwapi.h> //PathFileExists
#else
#include <sys/stat.h> // mkdir
#include <unistd.h>
#endif

#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/crash.h"
#include "utils/log.h"
#include "utils/str.h"

// Files
static const char *logfile_name = "openomf.log";
static const char *configfile_name = "openomf.conf";
static const char *scorefile_name = "SCORES.DAT";
static char errormessage[128];

// Lists
static char *local_paths[NUMBER_OF_LOCAL_PATHS];
static char *resource_paths[NUMBER_OF_RESOURCES];

#if _WIN32
char const pm_path_sep = '\\';
#else
char const pm_path_sep = '/';
#endif

static char get_platform_sep(void) {
    if(strcmp(SDL_GetPlatform(), "Windows") == 0) {
        return '\\';
    }
    return '/';
}

int str_ends_with_sep(const char *str) {
    int pos = strlen(str) - 1;
    return (str[pos] == '/' || str[pos] == '\\');
}

// Build directory
static void local_path_build(int path_id, const char *path, const char *ext) {
    if(str_ends_with_sep(path)) {
        int len = strlen(path) + strlen(ext) + 1;
        local_paths[path_id] = omf_realloc(local_paths[path_id], len);
        snprintf(local_paths[path_id], len, "%s%s", path, ext);
    } else {
        int len = strlen(path) + strlen(ext) + 2;
        local_paths[path_id] = omf_realloc(local_paths[path_id], len);
        snprintf(local_paths[path_id], len, "%s%c%s", path, get_platform_sep(), ext);
    }
}

static void resource_path_build(int path_id, const char *path, const char *ext) {
    if(str_ends_with_sep(path)) {
        int len = strlen(path) + strlen(ext) + 1;
        resource_paths[path_id] = omf_realloc(resource_paths[path_id], len);
        snprintf(resource_paths[path_id], len, "%s%s", path, ext);
    } else {
        int len = strlen(path) + strlen(ext) + 2;
        resource_paths[path_id] = omf_realloc(resource_paths[path_id], len);
        snprintf(resource_paths[path_id], len, "%s%c%s", path, get_platform_sep(), ext);
    }
}

static bool file_exists(const char *test) {
#if defined(_WIN32) || defined(WIN32)
    return PathFileExistsA(test) == TRUE;
#else
    return access(test, F_OK) == 0;
#endif
}

// Makes sure resource file exists
int pm_validate_resources(void) {
    for(int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        const char *testfile = pm_get_resource_path(i);
        if(!file_exists(testfile)) {
            snprintf(errormessage, 128, "Missing file %s.", testfile);
            return 1;
        }
    }
    return 0;
}

int pm_init(void) {
    char *config_base_dir;
    char *state_base_dir;
    char *bin_base_dir;

    // Clear everything
    errormessage[0] = 0;
    memset(local_paths, 0, sizeof(local_paths));
    memset(resource_paths, 0, sizeof(resource_paths));

    // Find local basedir
    config_base_dir = pm_get_local_base_dir();
    if(config_base_dir == NULL) {
        goto error_0;
    }

    char *xdg_env = getenv("XDG_STATE_HOME");
    if(xdg_env) {
        state_base_dir = omf_strdup(xdg_env);
    } else {
        state_base_dir = config_base_dir;
    }

    // Other paths
    local_path_build(LOG_PATH, state_base_dir, logfile_name);
    local_path_build(CONFIG_PATH, config_base_dir, configfile_name);
    local_path_build(SCORE_PATH, state_base_dir, scorefile_name);
    if(strcmp(SDL_GetPlatform(), "Windows") == 0) {
        local_path_build(SAVE_PATH, state_base_dir, "save\\");
    } else {
        local_path_build(SAVE_PATH, state_base_dir, "save/");
    }

    // Set default base dirs for resources
    int m_ok = 0;
    if(pm_in_release_mode()) {
        // where is the openomf binary, if this call fails we will look for resources in ./resources
        bin_base_dir = SDL_GetBasePath();
        if(bin_base_dir != NULL) {
            if(strcmp(SDL_GetPlatform(), "Windows") == 0) {
                // on windows, the resources will be in ./resources, relative to the binary
                local_path_build(RESOURCE_PATH, bin_base_dir, "resources\\");
                local_path_build(SHADER_PATH, bin_base_dir, "shaders\\");
                m_ok = 1;
            } else if(strcmp(SDL_GetPlatform(), "Linux") == 0) {
                // on linux, the resources will be in ../share/games/openomf, relative to the binary
                // so if openomf is installed to /usr/local/bin,
                // the resources will be in /usr/local/share/games/openomf
                local_path_build(RESOURCE_PATH, bin_base_dir, "../share/games/openomf/");
                local_path_build(SHADER_PATH, bin_base_dir, "../share/games/openomf/shaders/");
                m_ok = 1;
            } else if(strcmp(SDL_GetPlatform(), "Mac OS X") == 0) {
                // on OSX, GetBasePath returns the 'Resources' directory
                // if run from an app bundle, so we can use this as-is
                local_path_build(RESOURCE_PATH, bin_base_dir, "");
                local_path_build(SHADER_PATH, bin_base_dir, "shaders/");
                m_ok = 1;
            }
            // any other platform will look in ./resources
            SDL_free(bin_base_dir);
        }
    }

    // Set default resource paths
    if(!m_ok) {
        if(strcmp(SDL_GetPlatform(), "Windows") == 0) {
            local_path_build(RESOURCE_PATH, "resources\\", "");
            local_path_build(SHADER_PATH, "shaders\\", "");
        } else {
            local_path_build(RESOURCE_PATH, "resources/", "");
            local_path_build(SHADER_PATH, "shaders/", "");
        }
    }

    // check if we have overrides from the environment
    char *resource_env = getenv("OPENOMF_RESOURCE_DIR");
    if(resource_env) {
        // make sure it ends with a separator
        local_path_build(RESOURCE_PATH, resource_env, "");
    }

    char *shader_env = getenv("OPENOMF_SHADER_DIR");
    if(shader_env) {
        // make sure it ends with a separator
        local_path_build(SHADER_PATH, shader_env, "");
    }

    // Set resource paths
    for(int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        resource_path_build(i, pm_get_local_path(RESOURCE_PATH), get_resource_file(i));
    }

    // Check resources
    if(pm_validate_resources() != 0) {
        goto error_1;
    }

    // All done
    if(config_base_dir != state_base_dir) {
        omf_free(state_base_dir);
    }
    omf_free(config_base_dir);
    return 0;

error_1:
    pm_free();
    omf_free(config_base_dir);
error_0:
    return 1;
}

void pm_log(void) {
    // Debug info
    for(unsigned int i = 0; i < NUMBER_OF_LOCAL_PATHS; i++) {
        log_debug("%s: %s", pm_get_local_path_type_name(i), pm_get_local_path(i));
    }
}

void pm_free(void) {
    for(int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        omf_free(resource_paths[i]);
    }
    for(int i = 0; i < NUMBER_OF_LOCAL_PATHS; i++) {
        omf_free(local_paths[i]);
    }
}

const char *pm_get_errormsg(void) {
    return errormessage;
}

int pm_in_debug_mode(void) {
#ifdef DEBUGMODE
    return 1;
#else
    return 0;
#endif
}

int pm_in_release_mode(void) {
    if(!pm_in_portable_mode() && !pm_in_debug_mode()) {
        return 1;
    }
    return 0;
}

int pm_in_portable_mode(void) {
#if defined(_WIN32) || defined(WIN32)
    if(PathFileExistsA(configfile_name) != FALSE) {
        return 1;
    }
#else
    if(access(configfile_name, F_OK) != -1) {
        return 1;
    }
#endif
    return 0;
}

char *pm_get_local_base_dir(void) {
    char *out = NULL;
    if(pm_in_portable_mode()) {
        out = omf_calloc(1, 1);
        return out;
    }

    // Attempt to open up locally writable directory
    char *xdg_env = getenv("XDG_CONFIG_HOME");
    if(xdg_env) {
        out = omf_strdup(xdg_env);
    } else {
        char *sdl_path = SDL_GetPrefPath("openomfproject", "OpenOMF");
        if(sdl_path == NULL) {
            snprintf(errormessage, 128, "Error getting config path: %s", SDL_GetError());
            return NULL;
        }
        out = omf_strdup(sdl_path);
        SDL_free(sdl_path);
    }

    // Ensure the path exists before continuing on
    // XXX shouldn't SDL_GetPrefPath automatically create the path if it doesn't exist?
#if defined(_WIN32) || defined(WIN32)
    int sherr = SHCreateDirectoryEx(NULL, out, NULL);
    if(sherr == ERROR_FILE_EXISTS) {
        snprintf(errormessage, 128, "Please delete this file and relaunch OpenOMF: %s", out);
        omf_free(out);
        return NULL;
    } else if(sherr != ERROR_SUCCESS && sherr != ERROR_ALREADY_EXISTS) {
        snprintf(errormessage, 128, "Failed to create config path: %s", out);
        omf_free(out);
        return NULL;
    }
#endif

    // All done
    return out;
}

const char *pm_get_local_path(unsigned int local_id) {
    if(local_id >= NUMBER_OF_LOCAL_PATHS) {
        return NULL;
    }
    return local_paths[local_id];
}

const char *pm_get_resource_path(unsigned int resource_id) {
    if(resource_id >= NUMBER_OF_RESOURCES) {
        return NULL;
    }
    return resource_paths[resource_id];
}

void pm_get_music_path(char *dst, size_t size, music_file_type *type, unsigned int resource_id) {
    assert(is_music(resource_id));
    size_t ext_pos;
    str path;
    str_from_c(&path, pm_get_resource_path(resource_id));
    if(!str_last_of(&path, '.', &ext_pos)) {
        crash("Unable to find filename extension for music file!");
    }
    str alt;
    str_from_slice(&alt, &path, 0, ext_pos);
    str_append_c(&alt, ".ogg");
    if(file_exists(str_c(&alt))) {
        log_debug("Found alternate music file %s", str_c(&alt));
        snprintf(dst, size, "%s", str_c(&alt));
        *type = MUSIC_FILE_TYPE_OGG;
    } else {
        log_debug("Found original music file %s", str_c(&path));
        snprintf(dst, size, "%s", str_c(&path));
        *type = MUSIC_FILE_TYPE_PSM;
    }
    str_free(&alt);
    str_free(&path);
}

const char *pm_get_local_path_type_name(unsigned int path_id) {
    switch(path_id) {
        case RESOURCE_PATH:
            return "RESOURCE_PATH";
        case CONFIG_PATH:
            return "CONFIG_PATH";
        case LOG_PATH:
            return "LOG_PATH";
        case SCORE_PATH:
            return "SCORE_PATH";
        case SAVE_PATH:
            return "SAVE_PATH";
        case SHADER_PATH:
            return "SHADER_PATH";
    }
    return "UNKNOWN";
}

int pm_create_dir(const char *dirname) {
#if defined(_WIN32) || defined(WIN32)
    if(SHCreateDirectoryEx(NULL, dirname, NULL) != ERROR_SUCCESS) {
        log_error("Error while attempting to create directory '%s'.", dirname);
        return 1;
    }
#else
    if(mkdir(dirname, 0755) != 0) {
        log_error("Error while attempting to create directory '%s'.", dirname);
        return 1;
    }
#endif
    return 0;
}
