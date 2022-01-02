#ifdef USE_DUMB

#include <stdlib.h>
#include <string.h>
#ifdef __linux__
#include <strings.h> // strcasecmp
#endif // __linux__
#include <dumb.h>
#include "audio/sources/dumb_source.h"
#include "utils/allocator.h"
#include "utils/log.h"

typedef struct dumb_source_t {
    DUH_SIGRENDERER *renderer;
    sample_t **sig_samples;
    long sig_samples_size;
    DUH *data;
    long vlen;
    long vpos;
} dumb_source;

audio_source_freq dumb_freqs[] = {
    {11025, 0, "11025Hz"},
    {22050, 0, "22050Hz"},
    {44100, 1, "44100Hz"},
    {48000, 1, "48000Hz"},
    {0,0,0} // Guard
};

audio_source_resampler dumb_resamplers[] = {
    {DUMB_RQ_ALIASING, 0, "Aliasing"},
    {DUMB_RQ_BLEP, 0, "BLEP"},
    {DUMB_RQ_LINEAR, 1, "Linear"},
    {DUMB_RQ_BLAM, 0, "BLIP"},
    {DUMB_RQ_CUBIC, 0, "Cubic"},
    {DUMB_RQ_FIR, 0, "FIR"},
    {0,0,0} // Guard
};

audio_source_freq* dumb_get_freqs() {
    return dumb_freqs;
}

audio_source_resampler* dumb_get_resamplers() {
    return dumb_resamplers;
}

int dumb_source_update(audio_source *src, char *buffer, int len) {
    dumb_source *local = source_get_userdata(src);

    // Get deltatime and bitrate
    float delta = 65536.0f / source_get_frequency(src);
    int bps = source_get_channels(src) * source_get_bytes(src);

    // If looping is off, and if we have played the whole file, stop here.
    long pos = duh_sigrenderer_get_position(local->renderer);
    if(pos < local->vpos && !src->loop) {
        return 0;
    }
    local->vpos = pos;

    // ... otherwise get more data.
    int ret = duh_render_int(
            local->renderer,
            &local->sig_samples,
            &local->sig_samples_size,
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
    destroy_sample_buffer(local->sig_samples);
    omf_free(local);
    source_set_userdata(src, local);
    dumb_exit();  // Free libdumb memory
    DEBUG("Libdumb Source: Closed.");
}

int dumb_source_init(audio_source *src, const char* file, int channels, int freq, int resampler) {
    dumb_source *local = omf_calloc(1, sizeof(dumb_source));

    // Make sure libdumb is initialized
    dumb_register_stdfiles();

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
    local->renderer = duh_start_sigrenderer(local->data, 0, channels, 0);
    local->vlen = duh_get_length(local->data);
    local->vpos = 0;
    local->sig_samples = NULL;
    local->sig_samples_size = 0;

    // Set resampler here
    dumb_it_set_resampling_quality(duh_get_it_sigrenderer(local->renderer), resampler);

    // Audio information
    source_set_frequency(src, freq);
    source_set_bytes(src, 2);
    source_set_channels(src, channels);
    source_set_resampler(src, resampler);

    // Set callbacks
    source_set_userdata(src, local);
    source_set_update_cb(src, dumb_source_update);
    source_set_close_cb(src, dumb_source_close);

    // Some debug info
    DEBUG("Libdumb Source: Loaded file '%s' succesfully.", file);

    // All done
    return 0;

error_0:
    omf_free(local);
    return 1;
}

#endif // USE_DUMB
