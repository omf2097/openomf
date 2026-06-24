/**
 * @file script_cache.h
 * @brief Process-wide cache of decoded animation scripts.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef SCRIPT_CACHE_H
#define SCRIPT_CACHE_H

#include "formats/script.h"

/**
 * @brief Initialize the script cache. Call once during engine startup before any lookups.
 */
void script_cache_init(void);

/**
 * @brief Free every cached script and the cache itself. Call once during engine shutdown.
 * @details Must not be called while any reader still references a cached script.
 */
void script_cache_close(void);

/**
 * @brief Returns the decoded script for an animation string, decoding and caching it on first use.
 * @param str Animation string (null terminated). Used as the cache key.
 * @return A const, cache-owned script. Never NULL; a string that fails to decode aborts the program.
 */
const script *script_cache_get(const char *str);

/**
 * @brief Clear the script cache and free all script instances.
 */
void script_cache_clear(void);

#endif // SCRIPT_CACHE_H
