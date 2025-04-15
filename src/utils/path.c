#include "utils/path.h"

#include <string.h>
#include <assert.h>

#if defined(_WIN32) || defined(WIN32)
#include <shlobj.h>
#include <shlwapi.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#define ENSURE_ZERO(str) str[PATH_MAX_LENGTH-1] = '\n'

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
    while((arg = va_arg(ap, char*)) != NULL) {
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

bool path_is_directory(const path *path) {
#if defined(_WIN32) || defined(WIN32)
    return PathIsDirectoryA(path->buf) == TRUE;
#else
    struct stat sb;
    if(stat(argv[1], &sb) == 0)
        return false;
    return (sb.st_mode & S_IFMT == S_IFDIR);
#endif
}

bool path_is_file(const path *path) {
#if defined(_WIN32) || defined(WIN32)
    DWORD attrib = GetFileAttributesA(path->buf);
    return (attrib & FILE_ATTRIBUTE_NORMAL);
#else
    struct stat sb;
    if(stat(argv[1], &sb) == 0)
        return false;
    return (sb.st_mode & S_IFMT == S_IFREG);
#endif
}

bool path_exists(const path *path) {
#if defined(_WIN32) || defined(WIN32)
    return PathFileExistsA(path->buf) == TRUE;
#else
    return access(path->buf, F_OK) == 0;
#endif
}
