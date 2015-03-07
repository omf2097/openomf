#ifdef USE_MODPLUG

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define MODPLUG_STATIC
#include <libmodplug/modplug.h>
#include "audio/sources/modplug_source.h"
#include "utils/log.h"

typedef struct {
    ModPlugFile *renderer;
    char *data;
    size_t dsize;
    long vlen;
    long vpos;
} modplug_source;

int modplug_source_update(audio_source *src, char *buffer, int len) {
    modplug_source *local = source_get_userdata(src);
    return ModPlug_Read(local->renderer, buffer, len);
}

void modplug_source_close(audio_source *src) {
    modplug_source *local = source_get_userdata(src);
    ModPlug_Unload(local->renderer);
    free(local->data);
    free(local);
    DEBUG("Modplug Source: Closed.");
}

int modplug_source_init(audio_source *src, const char* file, int channels) {
    modplug_source *local = malloc(sizeof(modplug_source));

    // Read all data from file
    FILE *handle = fopen(file, "rb");
    if(handle == NULL) {
        PERROR("Modplug Source: Unable to open module file.");
        goto error_0;
    }
    fseek(handle, 0L, SEEK_END);
    local->dsize = ftell(handle);
    rewind(handle);
    local->data = malloc(local->dsize);
    fread(local->data, local->dsize, 1, handle);
    fclose(handle);

    // Settings
    ModPlug_Settings settings;
    ModPlug_GetSettings(&settings);
    settings.mResamplingMode = MODPLUG_RESAMPLE_FIR;
    settings.mChannels = channels;
    settings.mBits = 16;
    settings.mFrequency = 44100;
    settings.mLoopCount = (src->loop) ? -1 : 0;
    ModPlug_SetSettings(&settings);

    // Init rnederer
    local->renderer = ModPlug_Load(local->data, local->dsize);
    if(!local->renderer) {
        PERROR("Modplug Source: Error while loading module file!");
        goto error_1;
    }
    local->vlen = 0;
    local->vpos = 0;

    // Audio information
    source_set_frequency(src, 44100);
    source_set_bytes(src, 2);
    source_set_channels(src, channels);

    // Set callbacks
    source_set_userdata(src, local);
    source_set_update_cb(src, modplug_source_update);
    source_set_close_cb(src, modplug_source_close);

    // Some debug info
    DEBUG("Modplug Source: Loaded file '%s' succesfully.", file);

    // All done
    return 0;

error_1:
    free(local->data);

error_0:
    free(local);
    return 1;
}

#endif
