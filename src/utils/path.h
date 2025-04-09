#ifndef PATH_H
#define PATH_H

#include <stdbool.h>
#include <stdio.h>

#include "utils/iterator.h"

typedef struct path {
    char path[1024]; // overkill
} path;

// Normalize separator to "/"
void path_from_c(path *path, const char *str);
void path_from_parts(path *path, ...);
void path_preferences_dir(path *dst);  // Init with preferences directory (users writeable directory)
void path_resources_dir(path *dst); // Init with resources directory (read-only game data)

bool path_is_directory(const path *path);
bool path_is_file(const path *path);
bool path_exists(const path *path);

const char *path_ext(const path *path);
const char *path_basename(const path *path);
const char *path_dirname(const path *path);

bool path_touch(const path *path);
bool path_unlink(const path *path);
bool path_mkdir(const path *path);
bool path_rmdir(const path *path);

void path_glob_begin(const path *path, iterator *iter);
void path_glob_iter_begin(const path *path, const char *pattern, iterator *iter);

void path_append(path *path, ...); // Append new elements
void path_parent(path *path);  // Drop last element
void path_absolute(path *path);

bool path_file_length(path *path, size_t *length);
bool path_read_file(path *path, char *buffer, size_t length);

#endif // PATH_H
