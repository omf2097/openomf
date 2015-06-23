#ifdef USE_XMP

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <xmp.h>
#include "audio/sources/xmp_source.h"
#include "utils/log.h"

typedef struct {
    xmp_context ctx;
} xmp_source;

audio_source_freq xmp_freqs[] = {
    {11025, "11025Hz"},
    {22050, "22050Hz"},
    {44100, "44100Hz"},
    {0,0} // Guard
};

audio_source_resampler xmp_resamplers[] = {
    {XMP_INTERP_NEAREST, "Nearest"},
    {XMP_INTERP_LINEAR,  "Linear"},
    {XMP_INTERP_SPLINE,  "Cubic"},
    {0,0}
};

audio_source_freq* xmp_get_freqs() {
    return xmp_freqs;
}

audio_source_resampler* xmp_get_resamplers() {
    return xmp_resamplers;
}

int xmp_source_update(audio_source *src, char *buffer, int len) {
    xmp_source *local = source_get_userdata(src);
    int ret = xmp_play_buffer(local->ctx, buffer, len, (src->loop == 1) ? 999999 : 1);
    return (ret != 0) ? 0 : len;
}

void xmp_source_close(audio_source *src) {
    xmp_source *local = source_get_userdata(src);
    xmp_end_player(local->ctx);
    xmp_release_module(local->ctx);
    free(local);
    DEBUG("XMP Source: Closed.");
}

int xmp_source_init(audio_source *src, const char* file, int channels, int freq, int resampler) {
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
    if(xmp_start_player(local->ctx, freq, flags) != 0) {
        PERROR("XMP Source: Unable to open module file.");
        goto error_1;
    }
    if(xmp_set_player(local->ctx, XMP_PLAYER_INTERP, resampler) != 0) {
        PERROR("XMP Source: Unable to set resampler.");
        goto error_1;
    }

    // Audio information
    source_set_frequency(src, freq);
    source_set_bytes(src, 2);
    source_set_channels(src, channels);
    source_set_resampler(src, resampler);

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
