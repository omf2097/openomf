#ifndef CRASH_H
#define CRASH_H

// This is GNU specific
#ifndef __COLD
#define __COLD
#endif // __COLD

_Noreturn void _crash(const char *message, const char *function, const char *file, int line, ...) __COLD;

#define crash_with_args(message, ...) _crash(message, __func__, __FILE__, __LINE__, __VA_ARGS__)
#define crash(message) _crash(message, __func__, __FILE__, __LINE__)

#endif // CRASH_H
