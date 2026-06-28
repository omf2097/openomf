/**
 * @file compat.h
 * @brief Platform compatibility definitions.
 * @details Provides type definitions and includes that may vary across platforms.
 *          Ensures consistent availability of char16_t and char32_t types.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef COMPAT_H
#define COMPAT_H

#include <string.h>

#ifdef __APPLE__
// MacOS X does not ship uchar.h
#include <stdint.h>
typedef uint_least16_t char16_t;
typedef uint_least32_t char32_t;
#else
#include <uchar.h>
#endif

// Tell the compiler that the marked a function takes printf-style arguments
#if defined(__GNUC__)
#define ATTR_FORMAT_PRINTF(fmt_idx, args_idx) __attribute__((format(printf, fmt_idx, args_idx)))
#else
#define ATTR_FORMAT_PRINTF(fmt_idx, args_idx)
#endif

#endif // COMPAT_H
