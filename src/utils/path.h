#ifndef PATH_H
#define PATH_H

#include <stdbool.h>
#include <stdio.h>

#include "utils/iterator.h"
#include "utils/str.h"

#define PATH_MAX_LENGTH 1024

#define PATH_NARGS(...) (sizeof((const char*[]){__VA_ARGS__})/sizeof(const char*))

typedef struct path {
    char buf[PATH_MAX_LENGTH];
} path;

// Normalize separator to "/"
void path_from_c(path *path, const char *src);
void _path_from_parts(path *path, int nargs, ...);
void path_from_str(path *path, str *src);

#define path_from_parts(path, ...) _path_from_parts(path, PATH_NARGS(__VA_ARGS__), __VA_ARGS__)

const char *path_c(const path *path);
void path_clear(path *path);
bool path_is_set(const path *path);

bool path_is_directory(const path *path);
bool path_is_file(const path *path);
bool path_exists(const path *path);

/** Get the extension of the last component (if any) */
bool path_ext(const path *path, char *buf, size_t len); // Suffix of the final path component (if any)

/** Get stem of the final component (last component without ext) */
bool path_stem(const path *path, char *buf, size_t len);

/** Get full path, excluding the last component.*/
bool path_parents(const path *path, char *buf, size_t len);

bool path_unlink(const path *path);
bool path_mkdir(const path *path);

/** Iterate only directories */
void path_iter_dirs(const path *path, iterator *iter);

/** Iterate only files */
void path_iter_files(const path *path, iterator *iter);

/** Iterate using glob pattern */
void path_iter_glob(const path *path, const char *pattern, iterator *iter);

void path_append(path *path, ...); // Append new element(s)
void path_pop(path *path);         // Drop last element
void path_absolute(path *path);

FILE *path_fopen(const path *path, const char *mode);

#endif // PATH_H
