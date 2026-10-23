/* SDL2 subset for MIN_LIB_BUILD. Not a full SDL. */
#ifndef SDL_STDINC_H
#define SDL_STDINC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t Uint8;
typedef int8_t Sint8;
typedef uint16_t Uint16;
typedef int16_t Sint16;
typedef uint32_t Uint32;
typedef int32_t Sint32;
typedef uint64_t Uint64;
typedef int64_t Sint64;

typedef int SDL_bool;
#define SDL_TRUE 1
#define SDL_FALSE 0

#define SDL_malloc malloc
#define SDL_calloc calloc
#define SDL_free free
#define SDL_realloc realloc
#define SDL_memcpy memcpy
#define SDL_memset memset
#define SDL_zero(x) memset(&(x), 0, sizeof(x))

#endif /* SDL_STDINC_H */
