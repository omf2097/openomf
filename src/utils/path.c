#include "utils/path.h"

#include <assert.h>
#include <dirent.h>
#include <string.h>

#if defined(_WIN32) || defined(WIN32)
#include <shlobj.h>
#include <shlwapi.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#define ENSURE_ZERO(str) str[PATH_MAX_LENGTH - 1] = '0'

void path_from_c(path *path, const char *src) {
    assert(strlen(src) < PATH_MAX_LENGTH);
    strncpy(path->buf, src, PATH_MAX_LENGTH);
    ENSURE_ZERO(path->buf);
}

void _path_from_parts(path *path, int nargs, ...) {
    str tmp;
    str_create(&tmp);

    va_list ap;
    va_start(ap, nargs);
    for (int i = 0; i < nargs; i++) {
        const char *arg = va_arg(ap, char *);
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

const char *path_c(const path *path) {
    return path->buf;
}

void path_clear(path *path) {
    memset(path->buf, 0, PATH_MAX_LENGTH);
}

bool path_is_set(const path *path) {
    return path->buf[0] != 0;
}

bool path_is_directory(const path *path) {
#if defined(_WIN32) || defined(WIN32)
    return PathIsDirectory(path->buf) == TRUE;
#else
    struct stat sb;
    if(stat(path->buf, &sb) != 0)
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

FILE *path_fopen(const path *path, const char *mode) {
    return fopen(path->buf, mode);
}

