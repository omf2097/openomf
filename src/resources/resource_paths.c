#include "resources/resource_paths.h"
#include "utils/log.h"
#include "utils/path.h"
#include "utils/vector.h"

#define SDL_ORGANISATION_NAME ""
#define SDL_PROJECT_NAME "OpenOMF"

#define OPENOMF_STATE_PATH "OPENOMF_STATE_PATH"
#define OPENOMF_CONFIG_PATH "OPENOMF_CONFIG_PATH"
#define OPENOMF_RESOURCE_PATH "OPENOMF_RESOURCE_PATH"

#define XDG_STATE_HOME "XDG_STATE_HOME"
#define XDG_CONFIG_HOME "XDG_CONFIG_HOME"
#define XDG_DATA_HOME "XDG_DATA_HOME"

#ifndef SYSTEM_RESOURCE_PATH
// This should be set by CMake
#define SYSTEM_RESOURCE_PATH "/usr/local/share/games:/usr/share/games:/usr/local/share:/usr/share"
#endif

static path resource_dir = {0};
static path config_dir = {0};
static path state_dir = {0};

static bool base_path(str *dst) {
    char *sdl_env = SDL_GetBasePath();
    if(sdl_env) {
        str_from_c(dst, sdl_env);
        SDL_free(sdl_env);
        return true;
    }
    return false;
}

static bool env_str(str *dst, const char *name) {
    const char *env_path = getenv(name);
    if(env_path) {
        str_from_c(dst, env_path);
        return true;
    }
    return false;
}

static bool sdl_env(str *dst) {
    char *sdl_env = SDL_GetPrefPath(SDL_ORGANISATION_NAME, SDL_PROJECT_NAME);
    if(sdl_env) {
        str_from_c(dst, sdl_env);
        SDL_free(sdl_env);
        return true;
    }
    return false;
}

static bool system_path(str *dst) {
    const char *system_path = SYSTEM_RESOURCE_PATH;
    if(system_path != NULL && strlen(system_path) > 0) {
        str_from_c(dst, system_path);
        return true;
    }
    return false;
}

static bool find_writeable_path(path *dst, const char *own_env_name, const char *xdg_env_name) {
    str tmp;
    path_clear(dst);
    if(env_str(&tmp, own_env_name)) {
        goto ok;
    }
    if(env_str(&tmp, xdg_env_name)) {
        goto ok;
    }
    if(sdl_env(&tmp)) {
        goto ok;
    }
    return false;

ok:
    path_from_c(dst, str_c(&tmp));
    str_free(&tmp);
    return true;
}

static bool scan_potential_resource_dirs(path *result, const str *src, const char *find, const char *append) {
    str *slice;
    path test;
    vector paths;
    iterator it;

    str_split(&paths, src, ':');
    vector_iter_begin(&paths, &it);
    foreach(it, slice) {
        path_from_parts(&test, str_c(slice), find);
        log_debug("Looking for resources in %s ...", path_c(&test));
        if(path_is_file(&test)) {
            path_from_parts(result, str_c(slice), append);
            break;
        }
    }
    vector_free(&paths);
    return path_is_set(result);
}

static bool find_resource_path(path *dst) {
    str tmp;
    path_clear(dst);
    if(env_str(&tmp, OPENOMF_RESOURCE_PATH)) {
        if(scan_potential_resource_dirs(dst, &tmp, "resources/openomf.bk", "/")) {
            log_debug("Resources found in %s: %s", OPENOMF_RESOURCE_PATH, path_c(dst));
            goto ok;
        }
        str_free(&tmp);
    }
    if(env_str(&tmp, XDG_DATA_HOME)) {
        if(scan_potential_resource_dirs(dst, &tmp, "resources/openomf.bk", "/")) {
            log_debug("Resources found in %s: %s", XDG_DATA_HOME, path_c(dst));
            goto ok;
        }
        str_free(&tmp);
    }
    if(base_path(&tmp)) {
        // Check the base path. This may work in windows build sand when in dev mode.
        if(scan_potential_resource_dirs(dst, &tmp, "resources/openomf.bk", "/")) {
            log_debug("Resources found in base directory: %s", path_c(dst));
            goto ok;
        }
        str_free(&tmp);
    }
    if(system_path(&tmp)) {
        // Check default paths. This may be set in CMAKE.
        if(scan_potential_resource_dirs(dst, &tmp, "openomf/resources/openomf.bk", "openomf/")) {
            log_debug("Resources found in system path: %s", path_c(dst));
            goto ok;
        }
        str_free(&tmp);
    }
    return false;

ok:
    str_free(&tmp);
    return true;
}

bool resource_path_init(void) {
    if(!find_writeable_path(&state_dir, OPENOMF_STATE_PATH, XDG_STATE_HOME)) {
        log_error("Unable to find state path!");
        return false;
    }
    if(!find_writeable_path(&config_dir, OPENOMF_CONFIG_PATH, XDG_CONFIG_HOME)) {
        log_error("Unable to find config path!");
        return false;
    }
    if(!find_resource_path(&resource_dir)) {
        log_error("Unable to find resource path!");
        return false;
    }
    log_info("Config path = %s", path_c(&config_dir));
    log_info("State path = %s", path_c(&state_dir));
    log_info("Resource path = %s", path_c(&resource_dir));
    return true;
}

void resource_path_create_dirs(void) {
    if(!path_exists(&state_dir)) {
        log_error("State directory does not exist, creating it now.");
        path_mkdir(&state_dir);
    }
    if(!path_exists(&config_dir)) {
        log_error("Config directory does not exist, creating it now.");
        path_mkdir(&config_dir);
    }
}

path get_config_dir(void) {
    return config_dir;
}

path get_state_dir(void) {
    return state_dir;
}

path get_resource_dir(void) {
    return resource_dir;
}
