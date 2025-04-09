#ifndef PATH_H
#define PATH_H

#include "list.h"

#include <stdbool.h>
#include <stdio.h>

#include "utils/iterator.h"
#include "utils/str.h"

#define PATH_MAX_LENGTH 1024

#define PATH_NARGS(...) (sizeof((const char *[]){__VA_ARGS__}) / sizeof(const char *))

typedef struct path {
    char buf[PATH_MAX_LENGTH];
} path;

// Normalize separator to "/"
void path_from_c(path *path, const char *src);
void path_from_str(path *path, str *src);
void _path_from_parts(path *path, int nargs, ...);
#define path_from_parts(path, ...) _path_from_parts(path, PATH_NARGS(__VA_ARGS__), __VA_ARGS__)

const char *path_c(const path *path);
void path_clear(path *path);
bool path_is_set(const path *path);

bool path_is_directory(const path *path);
bool path_is_file(const path *path);
bool path_exists(const path *path);

/**
 * Get the extension of the last component (if any)
 * @param path Path to check
 * @param dst Target string. Note that this will be allocated!
 * @return True if there was an extension, false if not.
 */
bool path_ext(const path *path, str *dst);      // Suffix of the final path component (if any)
void path_set_ext(path *path, const char *ext); // Change or set suffix to something e.

/** Convert filename into DOS format (replace anything odd with underscore, and capitalize everything) */
void path_dossify_filename(path *path);

/** Get path stem (last component without the file extension) */
void path_stem(const path *path, str *dst);

/** Get path filename (last component with the file extension) */
void path_filename(const path *path, str *dst);

bool path_unlink(const path *path);
bool path_mkdir(const path *path);

/** Iterate using glob pattern */
bool path_glob(const path *dir, list *results, const char *pattern);

void _path_append(path *path, int nargs, ...);
#define path_append(path, ...) _path_append(path, PATH_NARGS(__VA_ARGS__), __VA_ARGS__)
void path_pop(path *path); // Drop last element
void path_absolute(path *path);

FILE *path_fopen(const path *path, const char *mode);

#endif // PATH_H
