#ifndef LOG_H
#define LOG_H

#include <stdbool.h>
#include <stdlib.h>

typedef enum log_level
{
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} log_level;

#define log_debug(...) log_msg(LOG_DEBUG, __VA_ARGS__)
#define log_info(...) log_msg(LOG_INFO, __VA_ARGS__)
#define log_warn(...) log_msg(LOG_WARN, __VA_ARGS__)
#define log_error(...) log_msg(LOG_ERROR, __VA_ARGS__)

void log_init(void);
void log_close(void);

void log_set_level(log_level level);
void log_set_colors(bool toggle);
void log_add_stderr(log_level level, bool colors);
void log_add_file(const char *filename, log_level level);

void log_msg(log_level level, const char *fmt, ...);

#endif // LOG_H
