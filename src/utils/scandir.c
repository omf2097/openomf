#include "utils/scandir.h"
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

int scan_directory(list *dlist, const char *dir) {
    DIR *dp;
    struct dirent *entry;
    if((dp = opendir(dir)) == NULL) {
        return 1;
    }
    while((entry = readdir(dp)) != NULL) {
        list_append(dlist, entry->d_name, strlen(entry->d_name)+1);
    }
    closedir(dp);
    return 0;
}

int scan_directory_prefix(list *dlist, const char *dir, const char *prefix) {
    DIR *dp;
    struct dirent *entry;
    if((dp = opendir(dir)) == NULL) {
        return 1;
    }
    while((entry = readdir(dp)) != NULL) {
        if(strlen(entry->d_name) >= strlen(prefix)) {
            if(strncmp(entry->d_name, prefix, strlen(prefix)) == 0) {
                list_append(dlist, entry->d_name, strlen(entry->d_name)+1);
            }
        }
    }
    closedir(dp);
    return 0;
}
