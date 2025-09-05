#include "utils/resource_paths.h"
#include "utils/vector.h"
#include "utils/path.h"
#include "utils/log.h"

#define SDL_ORGANISATION_NAME ""
#define SDL_PROJECT_NAME "OpenOMF"

#define OPENOMF_STATE_PATH "OPENOMF_STATE_PATH"
#define OPENOMF_CONFIG_PATH "OPENOMF_CONFIG_PATH"
#define OPENOMF_RESOURCE_PATH "OPENOMF_RESOURCE_PATH"

#define XDG_STATE_HOME "XDG_STATE_HOME"
#define XDG_CONFIG_HOME "XDG_CONFIG_HOME"
#define XDG_DATA_HOME "XDG_DATA_HOME"

#ifndef DEFAULT_RESOURCE_PATH
// This should be set by CMake
#define DEFAULT_RESOURCE_PATH "./"
#endif

static path resource_dir = {0};
static path config_dir = {0};
static path state_dir = {0};

/*
static void find_current_dir(path *dst) {
    str tmp;
    char *base_path = SDL_GetBasePath();
    str_from_c(&tmp, base_path);
    SDL_free(base_path);

    str_replace(&tmp, "\\", "/", -1);
    strncpy(dst->buf, str_c(&tmp), PATH_MAX_LENGTH);
    str_free(&tmp);
}
*/

static bool env_str(str *dst, const char *name) {
    const char *env_path = getenv(name);
    if(env_path) {
        log_debug("Found variable %s: %s", name, env_path);
        str_from_c(dst, env_path);
        return true;
    }
    return false;
}

static bool sdl_env(str *dst) {
    char *sdl_env = SDL_GetPrefPath(SDL_ORGANISATION_NAME, SDL_PROJECT_NAME);
    if(sdl_env) {
        log_debug("Found SDL path: %s", sdl_env);
        str_from_c(dst, sdl_env);
        SDL_free(sdl_env);
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
    str_replace(&tmp, "\\", "/", -1);
    path_from_c(dst, str_c(&tmp));
    str_free(&tmp);
    return true;
}

static bool scan_potential_resource_dirs(path *dst, str *src, const char *find) {
    str *test_path;
    vector paths;
    iterator it;
    str_split(&paths, src, ':');
    vector_iter_begin(&paths, &it);
    foreach(it, test_path) {
        str_append_c(test_path, find);
        log_debug("Testing %s ...", str_c(test_path));
        path_from_str(dst, test_path);
        if (!path_is_directory(dst)) {
            path_clear(dst);
        }
    }
    vector_free(&paths);
    return path_is_set(dst);
}

static bool find_resource_path(path *dst) {
    str tmp;
    path_clear(dst);
    if(env_str(&tmp, OPENOMF_RESOURCE_PATH)) {
        if(scan_potential_resource_dirs(dst, &tmp, "resources/")) {
            log_debug("Resources found in %s: %s", OPENOMF_RESOURCE_PATH, path_c(dst));
            goto ok;
        }
        str_free(&tmp);
    }
    if(env_str(&tmp, XDG_DATA_HOME)) {
        if(scan_potential_resource_dirs(dst, &tmp, "resources/")) {
            log_debug("Resources found in %s: %s", XDG_DATA_HOME, path_c(dst));
            goto ok;
        }
        str_free(&tmp);
    }
    if(strlen(DEFAULT_RESOURCE_PATH) > 0) {
        str_from_c(&tmp, DEFAULT_RESOURCE_PATH);
        if(scan_potential_resource_dirs(dst, &tmp, "resources/")) {
            log_debug("Resources found in DEFAULT_RESOURCE_PATH: %s", path_c(dst));
            goto ok;
        }
        str_free(&tmp);
    }
    return false;

ok:
    str_replace(&tmp, "\\", "/", -1);
    path_from_c(dst, str_c(&tmp));
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
    if(!find_resource_path(&config_dir)) {
        log_error("Unable to find resource path!");
        return false;
    }
    log_info("Config path = %s", path_c(&config_dir));
    log_info("State path = %s", path_c(&state_dir));
    log_info("Resource path = %s", path_c(&resource_dir));
    return true;
}

path get_config_dir(void) {
    return config_dir;
}

path get_state_dir(void) {
    return  state_dir;
}

path get_resource_dir(void) {
    return resource_dir;
}
