#include "utils/scandir.h"
#include <string.h>

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#else
#include <dirent.h>
#endif

int scan_directory(list *dir_list, const char *dir) {
#if defined(_WIN32) || defined(WIN32)

    WIN32_FIND_DATAA entry;
    HANDLE hFind;
    if((hFind = FindFirstFileA(dir, &entry)) == INVALID_HANDLE_VALUE) {
        return 1;
    }
    while(FindNextFileA(hFind, &entry) != FALSE) {
        list_append(dir_list, entry.cFileName, strlen(entry.cFileName) + 1);
    }
    FindClose(hFind);
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
#if defined(_WIN32) || defined(WIN32)

    WIN32_FIND_DATAA entry;
    HANDLE hFind;
    if((hFind = FindFirstFileA(dir, &entry)) == INVALID_HANDLE_VALUE) {
        return 1;
    }
    size_t prefix_len = strlen(prefix);
    while(FindNextFileA(hFind, &entry) != FALSE) {
        size_t filename_len = strlen(entry.cFileName);
        if(filename_len >= prefix_len && memcmp(entry.cFileName, prefix, prefix_len) == 0) {
            list_append(dir_list, entry.cFileName, filename_len + 1);
        }
    }
    FindClose(hFind);
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
#if defined(_WIN32) || defined(WIN32)

    WIN32_FIND_DATAA entry;
    HANDLE hFind;
    if((hFind = FindFirstFileA(dir, &entry)) == INVALID_HANDLE_VALUE) {
        return 1;
    }
    size_t suffix_len = strlen(suffix);
    while(FindNextFileA(hFind, &entry) != FALSE) {
        size_t filename_len = strlen(entry.cFileName);
        if(filename_len >= suffix_len && memcmp(entry.cFileName + filename_len - suffix_len, suffix, suffix_len) == 0) {
            list_append(dir_list, entry.cFileName, filename_len + 1);
        }
    }
    FindClose(hFind);
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
