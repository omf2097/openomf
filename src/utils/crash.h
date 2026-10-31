/**
 * @file crash.h
 * @brief Fatal error handling and crash reporting.
 * @details Provides macros for terminating the program with a diagnostic message.
 *          Used throughout the codebase to handle unrecoverable errors.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef CRASH_H
#define CRASH_H

#include "utils/compat.h"

// This is GNU specific
#ifndef __COLD
#define __COLD
#endif // __COLD

/**
 * @internal
 * @brief Internal implementation - use crash() or crash_with_args() macros instead.
 * @see crash
 * @see crash_with_args
 */
_Noreturn void _crash(const char *function, const char *file, int line, const char *message, ...) __COLD
    ATTR_FORMAT_PRINTF(4, 5);

/**
 * @brief Crash the program with a formatted error message.
 * @details Terminates the program immediately with a diagnostic message that includes
 *          the file name, line number, and function name.
 * @param message Printf-style format string
 * @param ... Format string arguments
 */
#define crash_with_args(message, ...) _crash(__func__, __FILE__, __LINE__, message, __VA_ARGS__)

/**
 * @brief Crash the program with an error message.
 * @details Terminates the program immediately with a diagnostic message that includes
 *          the file name, line number, and function name.
 * @param message Error message string
 */
#define crash(message) _crash(__func__, __FILE__, __LINE__, message)

#endif // CRASH_H
