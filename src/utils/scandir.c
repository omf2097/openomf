#include "utils/scandir.h"
#include <dirent.h>
#include <string.h>

int scan_directory(list *dir_list, const char *dir) {
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
}

int scan_directory_prefix(list *dir_list, const char *dir, const char *prefix) {
    DIR *dp;
    struct dirent *entry;
    if((dp = opendir(dir)) == NULL) {
        return 1;
    }
    while((entry = readdir(dp)) != NULL) {
        if(strlen(entry->d_name) >= strlen(prefix)) {
            if(strncmp(entry->d_name, prefix, strlen(prefix)) == 0) {
                list_append(dir_list, entry->d_name, strlen(entry->d_name) + 1);
            }
        }
    }
    closedir(dp);
    return 0;
}


int scan_directory_suffix(list *dir_list, const char *dir, const char *suffix) {
    DIR *dp;
    struct dirent *entry;
    if((dp = opendir(dir)) == NULL) {
        return 1;
    }
    while((entry = readdir(dp)) != NULL) {
        if(strlen(entry->d_name) >= strlen(suffix)) {
            if(strncmp(entry->d_name + strlen(entry->d_name) - strlen(suffix) , suffix, strlen(suffix)) == 0) {
                list_append(dir_list, entry->d_name, strlen(entry->d_name) + 1);
            }
        }
    }
    closedir(dp);
    return 0;
}
