#include "utils/path.h"
#include "utils/c_string_util.h"
#include "utils/log.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32) || defined(WIN32)
#include <shlobj.h>
#include <shlwapi.h>
#include <windows.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif

#define ENSURE_ZERO(str) str[PATH_MAX_LENGTH - 1] = '\0'

#if defined(_WIN32) || defined(WIN32)
static void normalize_slashes(path *p) {
    char *str = p->buf;
    while(*str != '\0') {
        if(*str == '\\') {
            *str = '/';
        }
        str++;
    }
}
#endif

void path_from_c(path *path, const char *src) {
    strncpy_or_abort(path->buf, src, PATH_MAX_LENGTH);
    ENSURE_ZERO(path->buf);
#if defined(_WIN32) || defined(WIN32)
    normalize_slashes(path);
#endif
}

void _path_from_parts(path *path, const int nargs, ...) {
    str tmp;
    str_create(&tmp);

    va_list ap;
    va_start(ap, nargs);
    for(int i = 0; i < nargs; i++) {
        const char *arg = va_arg(ap, char *);
        str_append_c(&tmp, arg);
#if defined(_WIN32) || defined(WIN32)
        str_replace(&tmp, "\\", "/", -1);
#endif
        if(!str_ends_with(&tmp, "/")) {
            str_append_char(&tmp, '/');
        }
    }
    va_end(ap);

    str_cut(&tmp, 1);
    path_from_str(path, &tmp);
    ENSURE_ZERO(path->buf);
    str_free(&tmp);
}

void path_from_str(path *path, const str *src) {
    strncpy_or_abort(path->buf, str_c(src), PATH_MAX_LENGTH);
    ENSURE_ZERO(path->buf);
#if defined(_WIN32) || defined(WIN32)
    normalize_slashes(path);
#endif
}

#if defined(_WIN32) || defined(WIN32)
static void generate_noise(str *component, size_t size) {
    char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    size_t len = strlen(characters);
    for(size_t i = 0; i < size; i++) {
        str_append_char(component, characters[rand() % len]);
    }
}
#endif

bool path_create_tmpdir(path *path) {
#if defined(_WIN32) || defined(WIN32)
    char buf[PATH_MAX_LENGTH];
    const int ret = GetTempPathA(PATH_MAX_LENGTH - 1, buf);
    buf[PATH_MAX_LENGTH - 1] = '\0';
    if(ret == 0 || ret > PATH_MAX_LENGTH) {
        return false;
    }
    path_from_c(path, buf);

    str component;
    str_from_c(&component, "openomf.");
    generate_noise(&component, 16);
    path_append(path, str_c(&component));
    str_free(&component);

    if(!path_mkdir(path)) {
        return false;
    }
#else
    char template[] = "/tmp/openomf.XXXXXX"; // Will be modified by mkdtemp
    const char *dir_name = mkdtemp(template);
    if(dir_name == NULL) {
        return false;
    }
    path_from_c(path, dir_name);
#endif
    return true;
}

const char *path_c(const path *path) {
    return path->buf;
}

#if defined(_WIN32) || defined(WIN32)
#define realpath(N, R) _fullpath((R), (N), PATH_MAX_LENGTH)
#endif

bool path_resolve(path *p) {
    char *resolved = realpath(p->buf, NULL);
    if(resolved == NULL) {
        return false;
    }
    strncpy_or_abort(p->buf, resolved, PATH_MAX_LENGTH);
    free(resolved);
    return true;
}

void path_clear(path *path) {
    memset(path->buf, 0, PATH_MAX_LENGTH);
}

bool path_is_set(const path *path) {
    return path->buf[0] != 0;
}

bool path_is_directory(const path *p) {
#if defined(_WIN32) || defined(WIN32)
    return PathIsDirectoryA(p->buf) == FILE_ATTRIBUTE_DIRECTORY;
#else
    struct stat sb;
    if(stat(p->buf, &sb) != 0) {
        return false;
    }
    return (sb.st_mode & S_IFMT) == S_IFDIR;
#endif
}

bool path_is_file(const path *p) {
#if defined(_WIN32) || defined(WIN32)
    DWORD attrib = GetFileAttributesA(p->buf);
    return (attrib & FILE_ATTRIBUTE_DIRECTORY) == 0;
#else
    struct stat sb;
    if(stat(p->buf, &sb) != 0) {
        return false;
    }
    return (sb.st_mode & S_IFMT) == S_IFREG;
#endif
}

bool path_exists(const path *p) {
#if defined(_WIN32) || defined(WIN32)
    return PathFileExistsA(p->buf) == TRUE;
#else
    return access(p->buf, F_OK) == 0;
#endif
}

static bool find_ext_start_end(const str *tmp, size_t *start, size_t *end) {
    size_t spos, dpos;
    if(str_last_of(tmp, '/', &spos)) {
        spos += 1;
    } else {
        spos = 0;
    }
    if(!str_last_of(tmp, '.', &dpos)) {
        return false;
    }
    if(spos >= dpos) {
        return false;
    }
    *start = dpos;
    *end = str_size(tmp);
    return true;
}

void path_ext(const path *path, str *dst) {
    str tmp;
    size_t start, end;
    str_from_c(&tmp, path->buf);
    if(find_ext_start_end(&tmp, &start, &end)) {
        str_from_slice(dst, &tmp, start, end);
    } else {
        str_create(dst);
    }
    str_free(&tmp);
}

void path_set_ext(path *path, const char *ext) {
    str tmp;
    size_t start, end;
    str_from_c(&tmp, path->buf);
    if(find_ext_start_end(&tmp, &start, &end)) {
        str_cut(&tmp, str_size(&tmp) - start);
    }
    str_append_c(&tmp, ext);
    path_from_str(path, &tmp);
    ENSURE_ZERO(path->buf);
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
    ENSURE_ZERO(path->buf);
    str_free(&tmp);
}

void path_stem(const path *path, str *dst) {
    str tmp;
    size_t start, end;
    str_from_c(&tmp, path->buf);
    if(str_last_of(&tmp, '/', &start)) {
        start += 1;
    } else {
        start = 0;
    }
    if(!str_last_of(&tmp, '.', &end) || end <= start) {
        end = str_size(&tmp);
    }
    if(start < end) {
        str_from_slice(dst, &tmp, start, end);
    } else {
        str_create(dst);
    }
    str_free(&tmp);
}

void path_parents(const path *path, str *dst) {
    str tmp;
    size_t end;
    str_from_c(&tmp, path->buf);
    if(str_last_of(&tmp, '/', &end) && end > 1) {
        str_from_slice(dst, &tmp, 0, end);
    } else {
        str_create(dst);
    }
    str_free(&tmp);
}

void path_filename(const path *path, str *dst) {
    str tmp;
    size_t start;
    str_from_c(&tmp, path->buf);
    const size_t end = str_size(&tmp);
    if(str_last_of(&tmp, '/', &start)) {
        start += 1;
    } else {
        start = 0;
    }
    if(start < end) {
        str_from_slice(dst, &tmp, start, end);
    } else {
        str_create(dst);
    }
    str_free(&tmp);
}

bool path_unlink(const path *p) {
#if defined(_WIN32) || defined(WIN32)
    return DeleteFile(p->buf) != 0;
#else
    return unlink(p->buf) == 0;
#endif
}

bool path_mkdir(const path *p) {
#if defined(_WIN32) || defined(WIN32)
    return SHCreateDirectoryEx(NULL, p->buf, NULL) == ERROR_SUCCESS;
#else
    return mkdir(p->buf, 0755) == 0;
#endif
}

bool path_rmdir(const path *p) {
#if defined(_WIN32) || defined(WIN32)
    return RemoveDirectoryA(p->buf);
#else
    return rmdir(p->buf) == 0;
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
#if defined(_WIN32) || defined(WIN32)
        str_replace(&tmp, "\\", "/", -1);
#endif
        if(!str_ends_with(&tmp, "/")) {
            str_append_char(&tmp, '/');
        }
    }
    va_end(ap);

    str_cut(&tmp, 1);
    path_from_str(path, &tmp);
    ENSURE_ZERO(path->buf);
    str_free(&tmp);
}

bool path_glob(const path *dir, list *results, const char *pattern) {
    str name;
    path test;
#if defined(_WIN32) || defined(WIN32)
    str glob;
    str_from_format(&glob, "%s/*", path_c(dir));
    str_replace(&glob, "/", "\\", -1);
    WIN32_FIND_DATAA entry;
    HANDLE hFind;
    if((hFind = FindFirstFileA(str_c(&glob), &entry)) == INVALID_HANDLE_VALUE) {
        str_free(&glob);
        return false;
    }
    str_free(&glob);
    while(FindNextFileA(hFind, &entry) != FALSE) {
        path_from_parts(&test, dir->buf, entry.cFileName);
        path_filename(&test, &name);
        if(str_imatch(&name, pattern)) {
            list_append(results, &test, sizeof(path));
        }
        str_free(&name);
    }
    FindClose(hFind);
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

FILE *path_fopen(const path *file, const char *mode) {
    return fopen(file->buf, mode);
}

bool path_filesize(const path *file, size_t *size) {
    struct stat info;
    if(stat(file->buf, &info) != 0) {
        return false;
    }
    if((info.st_mode & S_IFMT) != S_IFREG) {
        return false;
    }
    *size = (size_t)info.st_size;
    return true;
}

bool path_read_file(const path *file, char *buffer, size_t size) {
    FILE *handle = fopen(file->buf, "rb");
    if(handle == NULL) {
        return false;
    }
    bool success = (fread(buffer, 1, size, handle) == size);
    fclose(handle);
    return success;
}
