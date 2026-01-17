/**
 * @file time_fmt.h
 * @brief Time formatting utilities.
 * @details Provides functions for formatting timestamps for display.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef TIME_FMT_H
#define TIME_FMT_H

#include <string.h>

/**
 * @brief Format the current time as a human-readable string.
 * @details Returns a timestamp string suitable for logging.
 *          The returned pointer points to a static buffer that is
 *          overwritten on each call.
 * @return Formatted time string (do not free)
 */
char *format_time(void);

#endif // TIME_FMT_H
