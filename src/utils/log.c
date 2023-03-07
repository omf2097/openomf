#include "utils/log.h"
#include "utils/str.h"
#include <stdio.h>

typedef enum {
    TARGET_NONE,
    TARGET_FILE,
    TARGET_CON,
} log_target;

static unsigned int current_tick = 0;
static log_target init_mode = TARGET_NONE;

// Copied from SDL_log.c
static const char *SDL_priority_prefixes[SDL_NUM_LOG_PRIORITIES] = {
    NULL,
    "VERBOSE",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "CRITICAL"
};

void log_to_file(void *userdata, int category, SDL_LogPriority priority, const char *message) {
    FILE *handle = userdata;
    fprintf(handle, "%s: [%7u] %s\n", SDL_priority_prefixes[priority], current_tick, message);
    fflush(handle);
}

void log_to_std(void *userdata, int category, SDL_LogPriority priority, const char *message) {
    SDL_LogOutputFunction cb = userdata;
    char tmp[1024];
    snprintf(tmp, 1024, "[%7u] %s", current_tick, message);
    cb(userdata, category, priority, tmp);
}

int log_init(const char *filename) {
    if(init_mode != TARGET_NONE) {
        // Already initialized
        return 0;
    }

#ifdef DEBUGMODE
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
#else
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);
#endif

    if(filename == NULL) {
        SDL_LogOutputFunction default_output_function = NULL;
        SDL_LogGetOutputFunction(&default_output_function, NULL);
        SDL_LogSetOutputFunction(log_to_std, default_output_function);
        init_mode = TARGET_CON;
    } else {
        FILE *handle = fopen(filename, "w");
        if(handle == NULL) {
            return 1;
        }
        SDL_LogSetOutputFunction(log_to_file, handle);
        init_mode = TARGET_FILE;
    }
    return 0;
}

void log_tick(unsigned int tick) {
    current_tick = tick;
}

void log_close() {
    if(init_mode == TARGET_FILE) {
        FILE *handle = NULL;
        SDL_LogGetOutputFunction(NULL, (void*)handle);
        fclose(handle);
    }
}
