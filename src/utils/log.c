#include "utils/log.h"

#include <SDL_mutex.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "utils/allocator.h"

#define MAX_TARGETS 3

typedef struct log_target {
    FILE *fp;
    log_level level;
    bool colors;
    bool close;
    SDL_mutex *lock;
} log_target;

typedef struct log_state {
    bool colors;
    log_level level;
    log_target targets[MAX_TARGETS];
    int target_count;
    uint32_t tick;
} log_state;

static const char *level_names[] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
};

static const char *level_colors[] = {
    "\x1b[36m",
    "\x1b[32m",
    "\x1b[33m",
    "\x1b[31m",
};

static log_state *state = NULL;

void log_init(void) {
    assert(state == NULL);
    state = omf_calloc(1, sizeof(log_state));
    state->level = LOG_DEBUG;
    state->colors = false;
    state->target_count = 0;
}

static void close_targets(void) {
    assert(state != NULL);
    log_target *target;
    for(int i = 0; i < state->target_count; i++) {
        target = &state->targets[i];
        if(target->close) {
            fclose(target->fp);
        }
        if(target->lock) {
            SDL_DestroyMutex(target->lock);
        }
    }
    state->target_count = 0;
}

void log_close(void) {
    if(state != NULL) {
        close_targets();
        omf_free(state);
    }
}

void log_set_level(log_level level) {
    assert(state != NULL);
    state->level = level;
}

void log_set_colors(bool toggle) {
    assert(state != NULL);
    state->colors = toggle;
}

static void log_add_fp(FILE *fp, bool close, log_level level, bool colors) {
    assert(state != NULL);
    assert(state->target_count < MAX_TARGETS - 1);
    log_target *target = &state->targets[state->target_count++];
    target->close = close;
    target->fp = fp;
    target->level = level;
    target->colors = colors;
    target->lock = SDL_CreateMutex();
}

void log_add_stderr(log_level level, bool colors) {
    log_add_fp(stderr, false, level, colors);
}

void log_add_file(const char *filename, log_level level) {
    FILE *fp = fopen(filename, "w");
    if(fp) {
        log_add_fp(fp, true, level, false);
    }
}

static void format_timestamp(char *buffer, size_t len) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(buffer, len, "%H:%M:%S", tm);
    buffer[len - 1] = 0;
}

void log_msg(log_level level, const char *fmt, ...) {
    assert(state != NULL);
    char dt[16];
    va_list args;
    log_target *target;
    const char *color = level_colors[level];
    const char *name = level_names[level];

    if(level < state->level) {
        return;
    }

    format_timestamp(dt, 16);
    for(int i = 0; i < state->target_count; i++) {
        target = &state->targets[i];
        if(level < target->level) {
            continue;
        }
        if(SDL_LockMutex(target->lock) != 0) {
            continue;
        }
        if(state->colors && target->colors) {
            fprintf(target->fp, "%s %s%-5s\x1b[0m \x1b[0m ", dt, color, name);
        } else {
            fprintf(target->fp, "%s %-5s ", dt, name);
        }
        va_start(args, fmt);
        vfprintf(target->fp, fmt, args);
        va_end(args);
        fprintf(target->fp, "\n");
        fflush(target->fp);
        SDL_UnlockMutex(target->lock);
    }
}
