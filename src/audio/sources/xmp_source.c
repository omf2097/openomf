#ifdef USE_XMP

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <xmp.h>
#include "audio/sources/xmp_source.h"
#include "utils/log.h"

typedef struct {
    xmp_context ctx;
    int loop;
} xmp_source;

int xmp_source_update(audio_source *src, char *buffer, int len) {
    xmp_source *local = source_get_userdata(src);
    int ret = xmp_play_buffer(local->ctx, buffer, len, (local->loop == 1) ? 999999 : 1);
    return (ret != 0) ? 0 : len;
}

void xmp_source_close(audio_source *src) {
    xmp_source *local = source_get_userdata(src);
    xmp_end_player(local->ctx);
    xmp_release_module(local->ctx);
    free(local);
    DEBUG("XMP Source: Closed.");
}

int xmp_source_init(audio_source *src, const char* file, int channels) {
    xmp_source *local = malloc(sizeof(xmp_source));

    // Create a libxmp context
    local->ctx = xmp_create_context();
    if(local->ctx == NULL) {
        PERROR("XMP Source: Unable to initialize xmp context.");
        goto error_0;
    }

    // Load the module file
    if(xmp_load_module(local->ctx, (char*)file) < 0) {
        PERROR("XMP Source: Unable to open module file.");
        goto error_1;
    }

    // Show some information
    struct xmp_module_info mi;
    xmp_get_module_info(local->ctx, &mi);
    DEBUG("XMP Source: Track is %s (%s)", mi.mod->name, mi.mod->type);

    // Start the player
    int flags = 0;
    if(channels == 1) {
        flags |= XMP_FORMAT_MONO;
        DEBUG("XMP Source: Setting to MONO.");
    }
    if(xmp_start_player(local->ctx, 44100, flags) != 0) {
        PERROR("XMP Source: Unable to open module file.");
        goto error_1;
    }
    local->loop = src->loop;

    // Audio information
    source_set_frequency(src, 44100);
    source_set_bytes(src, 2);
    source_set_channels(src, channels);

    // Set callbacks
    source_set_userdata(src, local);
    source_set_update_cb(src, xmp_source_update);
    source_set_close_cb(src, xmp_source_close);

    // Some debug info
    DEBUG("XMP Source: Loaded file '%s' succesfully.", file);

    // All done
    return 0;

error_1:
    xmp_release_module(local->ctx);

error_0:
    free(local);
    return 1;
}

#endif // USE_XMP
