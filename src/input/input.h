/**
 * @file input.h
 * @brief Public input subsystem API
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

/**
 * @brief Probe the built-in backends. Must be called once before input_init.
 */
void input_scan_backends(void);

/**
 * @brief Look up an available backend by index.
 * @param index Backend index, 0 ... input_get_backend_count() - 1.
 * @param name Out: short name. May be NULL.
 * @param description Out: human-readable description. May be NULL.
 * @return true on success, false if index is out of range.
 */
bool input_get_backend_info(int index, const char **name, const char **description);

/**
 * @brief Number of available backends.
 */
int input_get_backend_count(void);

/**
 * @brief Initialize the input subsystem.
 * @param try_name Backend name to use, or NULL/empty to auto-pick the first available.
 * @return true on success.
 */
bool input_init(const char *try_name);

/**
 * @brief Close the input subsystem.
 */
void input_close(void);

#endif // INPUT_H
