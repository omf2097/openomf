#ifndef MINLIB_WIN32_HEADERS_H
#define MINLIB_WIN32_HEADERS_H

/* winsock2.h must come first. Without WIN32_LEAN_AND_MEAN, windows.h also
 * includes winsock.h, which collides with the winsock2 headers used elsewhere.
 * The warning push covers SDK headers that use #if on macros they do not define. */
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4668)
#endif
#include <winsock2.h>

#include <windows.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif /* MINLIB_WIN32_HEADERS_H */
