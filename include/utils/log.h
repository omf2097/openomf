#ifndef LOG_H
#define LOG_H

#include <stdlib.h>
#include <SDL_log.h>

#define DEBUG(...) SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#define PERROR(...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#define INFO(...) SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)

void log_tick(unsigned int tick);
int log_init(const char *filename);
void log_close();

#endif // LOG_H
