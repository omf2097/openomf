/**
 * @file io.h
 * @brief Basic file I/O utilities.
 * @details Provides simple wrapper functions for common file operations.
 *          These are lower-level than the path.h functions and work directly
 *          with C strings and FILE handles.
 */

#ifndef UTILS_IO_H
#define UTILS_IO_H

#include <stdbool.h>
#include <stdio.h>

/**
 * @brief Open a file for reading or writing.
 * @param file_name Path to the file
 * @param mode File open mode (see fopen() for options)
 * @return FILE handle on success, NULL on failure
 */
FILE *file_open(const char *file_name, const char *mode);

/**
 * @brief Get the size of an open file.
 * @details The file position is preserved after this call.
 * @param handle Open file handle
 * @return File size in bytes, or -1 on error
 */
long file_size(FILE *handle);

/**
 * @brief Read data from a file.
 * @param handle Open file handle
 * @param buffer Buffer to read data into
 * @param size Number of bytes to read
 * @return true if all bytes were read, false on error or short read
 */
bool file_read(FILE *handle, char *buffer, long size);

/**
 * @brief Close a file handle.
 * @param handle File handle to close
 */
void file_close(FILE *handle);

/**
 * @brief Check if a file exists.
 * @param file_name Path to the file
 * @return true if the file exists and is accessible, false otherwise
 */
bool file_exists(const char *file_name);

#endif // UTILS_IO_H
