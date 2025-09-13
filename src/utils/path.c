#include "utils/path.h"

#include "c_string_util.h"
#include "log.h"

#include <assert.h>
#include <string.h>

#if defined(_WIN32) || defined(WIN32)
#include <shlobj.h>
#include <shlwapi.h>
#include <windows.h>
#else
#include <dirent.h>
#include <libgen.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#define ENSURE_ZERO(str) str[PATH_MAX_LENGTH - 1] = '0'

void path_from_c(path *path, const char *src) {
    assert(strlen(src) < PATH_MAX_LENGTH);
    strncpy(path->buf, src, PATH_MAX_LENGTH);
    ENSURE_ZERO(path->buf);
}

void _path_from_parts(path *path, const int nargs, ...) {
    str tmp;
    str_create(&tmp);

    va_list ap;
    va_start(ap, nargs);
    for(int i = 0; i < nargs; i++) {
        const char *arg = va_arg(ap, char *);
        str_append_c(&tmp, arg);
        if(str_ends_with(&tmp, "\\")) {
            str_cut(&tmp, 1);
        }
        if(!str_ends_with(&tmp, "/")) {
            str_append_char(&tmp, '/');
        }
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
    if(stat(path->buf, &sb) != 0)
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

static bool find_ext_dot(const str *tmp, size_t *pos) {
    size_t spos, dpos;
    if(!str_last_of(tmp, '/', &spos)) {
        return false;
    }
    if(!str_last_of(tmp, '.', &dpos)) {
        return false;
    }
    if(spos > dpos) {
        return false;
    }
    *pos = dpos;
    return true;
}

bool path_ext(const path *path, str *dst) {
    str tmp;
    size_t pos;
    str_from_c(&tmp, path->buf);
    if(find_ext_dot(&tmp, &pos)) {
        str_from_slice(dst, &tmp, pos, str_size(&tmp));
    } else {
        str_create(dst);
    }
    str_free(&tmp);
    return str_size(dst) > 0;
}

void path_set_ext(path *path, const char *ext) {
    str tmp;
    size_t pos;
    ;
    str_from_c(&tmp, path->buf);
    if(find_ext_dot(&tmp, &pos)) {
        str_cut(&tmp, str_size(&tmp) - pos);
    }
    str_append_c(&tmp, ext);
    path_from_str(path, &tmp);
    str_free(&tmp);
}

void path_dossify_filename(path *path) {
    str tmp;
    size_t pos = 0;
    str_from_c(&tmp, path->buf);
    str_last_of(&tmp, '/', &pos);
    for(size_t i = pos + 1; i < str_size(&tmp); i++) {
        const char c = str_at(&tmp, i);
        if(c == '.') {
            continue;
        }
        str_set_at(&tmp, i, isalnum(c) ? toupper(c) : '_');
    }
    path_from_str(path, &tmp);
    str_free(&tmp);
}

void path_stem(const path *path, str *dst) {
    str tmp;
    size_t start, end;
    str_from_c(&tmp, path->buf);
    if(str_last_of(&tmp, '/', &start) && str_last_of(&tmp, '.', &end) && start + 1 < end) {
        str_from_slice(dst, &tmp, start + 1, end);
    } else {
        str_create(dst);
    }
    str_free(&tmp);
}

void path_filename(const path *path, str *dst) {
    str tmp;
    size_t start;
    str_from_c(&tmp, path->buf);
    if(str_last_of(&tmp, '/', &start)) {
        str_from_slice(dst, &tmp, start + 1, str_size(&tmp));
    } else {
        str_create(dst);
    }
    str_free(&tmp);
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

void _path_append(path *path, const int nargs, ...) {
    str tmp;
    str_from_c(&tmp, path_c(path));
    if(!str_ends_with(&tmp, "/")) {
        str_append_char(&tmp, '/');
    }

    va_list ap;
    va_start(ap, nargs);
    for(int i = 0; i < nargs; i++) {
        const char *arg = va_arg(ap, char *);
        str_append_c(&tmp, arg);
        if(str_ends_with(&tmp, "\\")) {
            str_cut(&tmp, 1);
        }
        if(!str_ends_with(&tmp, "/")) {
            str_append_char(&tmp, '/');
        }
    }
    va_end(ap);

    str_cut(&tmp, 1);
    path_from_str(path, &tmp);
    str_free(&tmp);
}

bool path_glob(const path *dir, list *results, const char *pattern) {
    str name;
    path test;
#if defined(_WIN32) || defined(WIN32)
    str glob;
    str_from_format(&glob, "%s*", dir);
    WIN32_FIND_DATAA entry;
    HANDLE hFind;
    if((hFind = FindFirstFileA(str_c(&glob), &entry)) == INVALID_HANDLE_VALUE) {
        str_free(&glob);
        return false;
    }
    while(FindNextFileA(hFind, &entry) != FALSE) {
        path_from_parts(&test, dir->buf, entry.cFileName);
        path_filename(&test, &name);
        if(str_imatch(&name, pattern)) {
            list_append(results, &test, sizeof(path));
        }
        str_free(&name);
    }
    FindClose(hFind);
    str_free(&glob);
    return true;
#else
    DIR *dp;
    struct dirent *entry;
    if((dp = opendir(dir->buf)) == NULL) {
        return false;
    }
    while((entry = readdir(dp)) != NULL) {
        path_from_parts(&test, dir->buf, entry->d_name);
        path_filename(&test, &name);
        if(str_imatch(&name, pattern)) {
            list_append(results, &test, sizeof(path));
        }
        str_free(&name);
    }
    closedir(dp);
    return true;
#endif
}

FILE *path_fopen(const path *path, const char *mode) {
    return fopen(path->buf, mode);
}
