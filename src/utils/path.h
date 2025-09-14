#ifndef PATH_H
#define PATH_H

#include "list.h"

#include <stdbool.h>
#include <stdio.h>

#include "utils/str.h"

#define PATH_MAX_LENGTH 256

#define PATH_NARGS(...) (sizeof((const char *[]){__VA_ARGS__}) / sizeof(const char *))

typedef struct path {
    char buf[PATH_MAX_LENGTH];
} path;

/**
 * Create a path from C string.
 * @param path Path to write into. Contents will be replaced!
 * @param src C string object to read.
 */
void path_from_c(path *path, const char *src);

/**
 * Create a path from str object.
 * @param path Path to write into. Contents will be replaced!
 * @param src String object to read.
 */
void path_from_str(path *path, const str *src);

/**
 * Create a path from components.
 * @param path Path to write into. Contents will be replaced!
 * @param nargs Number of arguments
 * @param ... List of C strings to be used as path components.
 */
void _path_from_parts(path *path, int nargs, ...);

/**
 * Create a path from components.
 * @param path Path to write into. Contents will be replaced!
 * @param ... List of C strings to be used as path components.
 */
#define path_from_parts(path, ...) _path_from_parts(path, PATH_NARGS(__VA_ARGS__), __VA_ARGS__)

/**
 * Create a randomly named temporary directory and get its path
 * @param path Path to write into. Contents will be replaced!
 */
bool path_create_tmpdir(path *path);

/**
 * Return a C string pointer to the path. This will be valid as long as the path exists.
 * @param path Path to operate on
 * @return C string pointer to the path data.
 */
const char *path_c(const path *path);

/**
 * Clear path. After this, path_is_set() will return false.
 * @param path Path to check
 */
void path_clear(path *path);

/**
 * Checks if path has a value.
 * @param path Path to check
 * @return True if path has something, false if not.
 */
bool path_is_set(const path *path);

/**
 * Get the extension of the last component (if any). If there are multiple extensions, only the last will be returned.
 * @param path Path to check
 * @param dst Target string. Note that this will be allocated!
 */
void path_ext(const path *path, str *dst);

/**
 * Get path parents (path without the filename and extension)
 * @param path Path to check
 * @param dst Target string. Note that this will be allocated!
 */
void path_parents(const path *path, str *dst);

/**
 * Get path stem (last component without the file extension)
 * @param path Path to check
 * @param dst Target string. Note that this will be allocated!
 */
void path_stem(const path *path, str *dst);

/**
 * Get path filename (last component with the file extension)
 * @param path Path to check
 * @param dst Target string. Note that this will be allocated!
 */
void path_filename(const path *path, str *dst);

/**
 * Checks if path is a directory
 * @param p Path to check
 * @return True if path is a directory, otherwise false.
 */
bool path_is_directory(const path *p);

/**
 * Checks if path is a file
 * @param p Path to check
 * @return True if path is a file, otherwise false.
 */
bool path_is_file(const path *p);

/**
 * Checks if path exists (is file or directory)
 * @param p Path to check
 * @return True if path exists, otherwise false.
 */
bool path_exists(const path *p);

/**
 * Create a file.
 * @param p File path to create
 * @return True if operation succeeded, false if not.
 */
bool path_touch(const path *p);

/**
 * Delete a file.
 * @param p File path to delete
 * @return True if operation succeeded, false if not.
 */
bool path_unlink(const path *p);

/**
 * Create a directory.
 * @param p Directory path to create
 * @return True if operation succeeded, false if not.
 */
bool path_mkdir(const path *p);

/**
 * Delete a directory.
 * @param p Directory path to delete
 * @return True if operation succeeded, false if not.
 */
bool path_rmdir(const path *p);

/**
 * Find files from the path using a glob pattern
 * Note! Only a single '*' supported!
 * @param dir Directory path to scan
 * @param results Result list. This must be preallocated, and results are appended to an existing list.
 * @param pattern Pattern to search for, e.g. '*.TXT'.
 * @return True if operation succeeded, false if not.
 */
bool path_glob(const path *dir, list *results, const char *pattern);

/**
 * Set new file extension for the file path. Old file extension (if it exists) will be replaced.
 * @param path File path to modify
 * @param ext New file extension
 */
void path_set_ext(path *path, const char *ext);

/**
 * Convert filename into DOS format (replace anything odd with underscore, and capitalize everything)
 * @param path Path to modify
 */
void path_dossify_filename(path *path);

/**
 * Append components to an existing path
 * @param path Path to append into.
 * @param nargs Number of arguments
 * @param ... List of C strings to be used as path components.
 */
void _path_append(path *path, int nargs, ...);

/**
 * Append components to an existing path
 * @param path Path to append into.
 * @param ... List of C strings to be used as path components.
 */
#define path_append(path, ...) _path_append(path, PATH_NARGS(__VA_ARGS__), __VA_ARGS__)

/**
 * Open path as a file using given mode.
 * @param file File path to open
 * @param mode Mode to use (see fopen() for reference).
 * @return File handle on success or NULL on failure.
 */
FILE *path_fopen(const path *file, const char *mode);

#endif // PATH_H
