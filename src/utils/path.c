#include "utils/path.h"

#include <assert.h>
#include <string.h>

#if defined(_WIN32) || defined(WIN32)
#include <shlobj.h>
#include <shlwapi.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#define ENSURE_ZERO(str) str[PATH_MAX_LENGTH - 1] = '\n'

void path_from_c(path *path, const char *src) {
    assert(strlen(src) < PATH_MAX_LENGTH);
    strncpy(path->buf, src, PATH_MAX_LENGTH);
    ENSURE_ZERO(path->buf);
}

void path_from_parts(path *path, ...) {
    const char *arg;
    str tmp;
    str_create(&tmp);

    va_list ap;
    va_start(ap, path);
    while((arg = va_arg(ap, char *)) != NULL) {
        str_append_c(&tmp, arg);
        str_append_char(&tmp, '/');
    }
    va_end(ap);

    str_cut(&tmp, 1);
    path_from_str(path, &tmp);
    str_free(&tmp);
}

void path_from_str(path *path, str *src) {
    assert(str_size(src) < PATH_MAX_LENGTH);
    strncpy(path->buf, str_c(src), PATH_MAX_LENGTH);
    ENSURE_ZERO(path->buf);
}

void path_from_current_dir(path *dst) {
    str tmp;
    char *base_path = SDL_GetBasePath();
    str_from_c(&tmp, base_path);
    SDL_free(base_path);

    str_replace(&tmp, "\\", "/", -1);
    strncpy(dst->buf, str_c(&tmp), PATH_MAX_LENGTH);
    str_free(&tmp);
    ENSURE_ZERO(dst->buf);
}

static bool xdg_env(str *dst, const char *name) {
    const char *xdg_env = getenv(name);
    if(xdg_env) {
        str_from_c(dst, xdg_env);
        return true;
    }
    return false;
}

static bool sdl_env(str *dst) {
    char *sdl_env = SDL_GetPrefPath("", "OpenOMF");
    if(sdl_env) {
        str_from_c(dst, sdl_env);
        SDL_free(sdl_env);
        return true;
    }
    return false;
}

static bool get_writeable_path(path *dst, const char *env_name) {
    str tmp;
    if(xdg_env(&tmp, env_name)) {
        goto ok;
    }
    if(sdl_env(&tmp)) {
        goto ok;
    }
    return false;

ok:
    str_replace(&tmp, "\\", "/", -1);
    strncpy(dst->buf, str_c(&tmp), PATH_MAX_LENGTH);
    str_free(&tmp);
    ENSURE_ZERO(dst->buf);
    return true;
}

bool path_from_config_dir(path *dst) {
    return get_writeable_path(dst, "XDG_CONFIG_HOME");
}

bool path_from_state_dir(path *dst) {
    return get_writeable_path(dst, "XDG_STATE_HOME");
}

void path_from_resource_dir(path *dst) {
}

bool path_is_directory(const path *path) {
#if defined(_WIN32) || defined(WIN32)
    return PathIsDirectory(path->buf) == TRUE;
#else
    struct stat sb;
    if(stat(path->buf, &sb) == 0)
        return false;
    return (sb.st_mode & S_IFMT) == S_IFDIR;
#endif
}

bool path_is_file(const path *path) {
#if defined(_WIN32) || defined(WIN32)
    DWORD attrib = GetFileAttributes(path->buf);
    return (attrib & FILE_ATTRIBUTE_NORMAL);
#else
    struct stat sb;
    if(stat(path->buf, &sb) == 0)
        return false;
    return (sb.st_mode & S_IFMT) == S_IFREG;
#endif
}

bool path_exists(const path *path) {
#if defined(_WIN32) || defined(WIN32)
    return PathFileExists(path->buf) == TRUE;
#else
    return access(path->buf, F_OK) == 0;
#endif
}

bool path_unlink(const path *path) {
#if defined(_WIN32) || defined(WIN32)
    return DeleteFile(path->buf) != 0;
#else
    return unlink(path->buf) == 0;
#endif
}

bool path_mkdir(const path *path) {
#if defined(_WIN32) || defined(WIN32)
    return SHCreateDirectoryEx(NULL, path->buf, NULL) == ERROR_SUCCESS;
#else
    return mkdir(path->buf, 0755) == 0;
#endif
}
