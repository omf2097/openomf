#ifndef LOG_H
#define LOG_H

#include <stdlib.h>

#ifdef DEBUGMODE
#define DEBUG(...) log_print('D', __func__, __VA_ARGS__)
#define PERROR(...) log_print('E', __func__, __VA_ARGS__)
#define INFO(...) log_print('I', __func__, __VA_ARGS__)
#else
#define DEBUG(...) log_hide('D', NULL, __VA_ARGS__)
#define PERROR(...) log_print('E', NULL, __VA_ARGS__)
#define INFO(...) log_print('I', NULL, __VA_ARGS__)
#endif

#define LOGTICK(x) _log_tick = x;
extern unsigned int _log_tick;

void log_hide(char mode, const char *fn, const char *fmt, ...); // no-op
void log_print(char mode, const char *fn, const char *fmt, ...);
int log_init(const char *filename);
void log_close();

#endif // LOG_H
