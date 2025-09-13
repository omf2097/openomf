#include "utils/scandir.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/str.h"
#include <assert.h>
#include <string.h>

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#else
#include <dirent.h>
#include <libgen.h>
#endif

int scan_directory(list *dir_list, const char *dir) {
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

bool scan_directory_for_file(char *path, size_t path_size) {
#if defined(_WIN32) || defined(WIN32)
    return true;
#else
    char *path_dup = omf_strdup(path);
    char *path_dup2 = omf_strdup(path);
    char *fn = basename(path_dup);
    char *directory = dirname(path_dup2);

    list dir_list;
    list_create(&dir_list);
    if(scan_directory(&dir_list, directory) != 0) {
        list_free(&dir_list);
        omf_free(path_dup);
        omf_free(path_dup2);
        return false;
    }
    iterator it;
    list_iter_begin(&dir_list, &it);
    const char *iter_fn;
    size_t fn_len = strlen(fn);
    bool found = false;
    foreach(it, iter_fn) {
        if(strlen(iter_fn) == fn_len && omf_strncasecmp(fn, iter_fn, fn_len) == 0) {
            snprintf(path, path_size, "%s/%s", directory, iter_fn);
            found = true;
            break;
        }
    }
    omf_free(path_dup);
    omf_free(path_dup2);
    list_free(&dir_list);
    return found;
#endif
}
