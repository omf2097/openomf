#include "game/fps_counter.h"
#include "game/gui/text_render.h"

#include <stdio.h>

#define FPS_CACHE_SIZE 100

static double fps_cache[FPS_CACHE_SIZE];
static int fps_index = 0;
static char buffer[16];
static int buffer_len = 0;
static text_settings text;

void fps_counter_init(void) {
    text.font = FONT_SMALL;
    text.cforeground = TEXT_BRIGHT_GREEN;
    for(int i = 0; i < FPS_CACHE_SIZE; i++) {
        fps_cache[i] = 0.0f;
    }
    buffer[0] = 0;
}

static inline int average_fps(void) {
    double avg = 0;
    for (int i = 0; i < FPS_CACHE_SIZE; i++) {
        avg += fps_cache[i];
    }
    avg /= FPS_CACHE_SIZE;
    return avg;
}

void fps_counter_add_measurement(double measurement) {
    fps_cache[fps_index] = 1.0 / measurement;
    fps_index = (fps_index + 1) % FPS_CACHE_SIZE;
}

void fps_counter_render(void) {
    if (fps_index == FPS_CACHE_SIZE - 1) {
        buffer_len = snprintf(buffer, 16, "%d", average_fps());
    }
    for(int m = 0; m < buffer_len; m++) {
        text_render_char(&text, TEXT_DEFAULT, 1 + 6 * m, 1, buffer[m]);
    }
}
