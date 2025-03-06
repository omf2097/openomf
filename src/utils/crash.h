#ifndef CRASH_H
#define CRASH_H

void _crash(const char *message, const char *function, const char *file, int line);

#define crash(message) _crash(message, __func__, __FILE__, __LINE__)

#endif // CRASH_H
