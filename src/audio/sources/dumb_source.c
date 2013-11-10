#include <stdlib.h>
#include <string.h>
#include <dumb/dumb.h>
#include "audio/sources/dumb_source.h"
#include "utils/log.h"

typedef struct dumb_source_t {
    DUH_SIGRENDERER *renderer;
    DUH *data;
} dumb_source;

int dumb_source_update(audio_source *src, char *buffer, int len) {
    dumb_source *local = source_get_userdata(src);
    float delta = 65536.0f / source_get_frequency(src);
    int bps = source_get_channels(src) * source_get_bytes(src);
    int ret = duh_render(
            local->renderer, 
            source_get_bytes(src) * 8, // Bits
            0,  // Unsign
            1.0f, // Volume
            delta,
            len / bps, // Size
            buffer
        ) * bps;
    return ret;
}

void dumb_source_close(audio_source *src) {
    dumb_source *local = source_get_userdata(src);
    duh_end_sigrenderer(local->renderer);
    unload_duh(local->data);
    free(local);
    DEBUG("Libdumb Source: Closed.");
}

int dumb_source_init(audio_source *src, const char* file) {
    dumb_source *local = malloc(sizeof(dumb_source));

    // Load file and initialize renderer
    char *ext = strrchr(file, '.') + 1;
    if(strcasecmp(ext, "psm") == 0) {
        local->data = dumb_load_psm(file, 0);
    } else if(strcasecmp(ext, "s3m") == 0) {
        local->data = dumb_load_s3m(file);
    } else if(strcasecmp(ext, "mod") == 0) {
        local->data = dumb_load_mod(file, 0);
    } else if(strcasecmp(ext, "it") == 0) {
        local->data = dumb_load_it(file);
    } else if(strcasecmp(ext, "xm") == 0) {
        local->data = dumb_load_xm(file);
    } else {
        PERROR("Libdumb Source: No suitable module decoder found.");
        goto error_0;
    }
    if(!local->data) {
        PERROR("Libdumb Source: Error while loading module file!");
        goto error_0;
    }
    local->renderer = duh_start_sigrenderer(local->data, 0, 2, 0);
    
    // Audio information
    source_set_frequency(src, 44100);
    source_set_bytes(src, 2);
    source_set_channels(src, 2);

    // Set callbacks
    source_set_userdata(src, local);
    source_set_update_cb(src, dumb_source_update);
    source_set_close_cb(src, dumb_source_close);

    // Some debug info
    DEBUG("Libdumb Source: Loaded file '%s' succesfully.", file);

    // All done
    return 0;

error_0:
    free(local);
    return 1;
}