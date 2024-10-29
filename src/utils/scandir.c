#include "utils/scandir.h"
#include "utils/str.h"
#include <assert.h>
#include <string.h>

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#else
#include <dirent.h>
#endif

static void check_valid_directory(char const *dir) {
#if !NDEBUG
    assert(dir != NULL);
    assert(dir[0] != '\0');
    size_t end = strlen(dir) - 1;
#if _WIN32
    assert(dir[end] == '\\');
#else
    assert(dir[end] == '/');
#endif
#endif
}

int scan_directory(list *dir_list, const char *dir) {
    check_valid_directory(dir);
#if defined(_WIN32) || defined(WIN32)

    str glob;
    str_from_format(&glob, "%s*", dir);
    WIN32_FIND_DATAA entry;
    HANDLE hFind;
    if((hFind = FindFirstFileA(str_c(&glob), &entry)) == INVALID_HANDLE_VALUE) {
        str_free(&glob);
        return 1;
    }
    while(FindNextFileA(hFind, &entry) != FALSE) {
        list_append(dir_list, entry.cFileName, strlen(entry.cFileName) + 1);
    }
    FindClose(hFind);
    str_free(&glob);
    return 0;

#else

    DIR *dp;
    struct dirent *entry;
    if((dp = opendir(dir)) == NULL) {
        return 1;
    }
    while((entry = readdir(dp)) != NULL) {
        list_append(dir_list, entry->d_name, strlen(entry->d_name) + 1);
    }
    closedir(dp);
    return 0;

#endif
}

int scan_directory_prefix(list *dir_list, const char *dir, const char *prefix) {
    check_valid_directory(dir);
#if defined(_WIN32) || defined(WIN32)

    str glob;
    str_from_format(&glob, "%s%s*", prefix, dir);
    WIN32_FIND_DATAA entry;
    HANDLE hFind;
    if((hFind = FindFirstFileA(str_c(&glob), &entry)) == INVALID_HANDLE_VALUE) {
        str_free(&glob);
        return 1;
    }
    while(FindNextFileA(hFind, &entry) != FALSE) {
        list_append(dir_list, entry.cFileName, strlen(entry.cFileName) + 1);
    }
    FindClose(hFind);
    str_free(&glob);
    return 0;

#else

    DIR *dp;
    struct dirent *entry;
    if((dp = opendir(dir)) == NULL) {
        return 1;
    }
    size_t prefix_len = strlen(prefix);
    while((entry = readdir(dp)) != NULL) {
        size_t filename_len = strlen(entry->d_name);
        if(filename_len >= prefix_len && memcmp(entry->d_name, prefix, prefix_len) == 0) {
            list_append(dir_list, entry->d_name, filename_len + 1);
        }
    }
    closedir(dp);
    return 0;

#endif
}

int scan_directory_suffix(list *dir_list, const char *dir, const char *suffix) {
    check_valid_directory(dir);
#if defined(_WIN32) || defined(WIN32)

    str glob;
    str_from_format(&glob, "%s*%s", dir, suffix);
    WIN32_FIND_DATAA entry;
    HANDLE hFind;
    if((hFind = FindFirstFileA(str_c(&glob), &entry)) == INVALID_HANDLE_VALUE) {
        str_free(&glob);
        return 1;
    }
    while(FindNextFileA(hFind, &entry) != FALSE) {
        list_append(dir_list, entry.cFileName, strlen(entry.cFileName) + 1);
    }
    FindClose(hFind);
    str_free(&glob);
    return 0;

#else

    DIR *dp;
    struct dirent *entry;
    if((dp = opendir(dir)) == NULL) {
        return 1;
    }
    size_t suffix_len = strlen(suffix);
    while((entry = readdir(dp)) != NULL) {
        size_t filename_len = strlen(entry->d_name);
        if(filename_len >= suffix_len && memcmp(entry->d_name + filename_len - suffix_len, suffix, suffix_len) == 0) {
            list_append(dir_list, entry->d_name, filename_len + 1);
        }
    }
    closedir(dp);
    return 0;

#endif
}
