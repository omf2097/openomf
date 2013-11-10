#include <stdlib.h>
#include "audio/source.h"

void source_init(audio_source *src) {
    src->frequency = 0;
    src->channels = 0;
    src->bytes = 0;
    src->loop = 0;
    src->userdata = NULL;
    src->update = NULL;
    src->close = NULL;
}

int source_update(audio_source *src, char *buffer, int len) {
    if(src->update != NULL) {
        return src->update(src, buffer, len);
    }
    return 0;
}

void source_free(audio_source *src) {
    if(src->close != NULL) {
        src->close(src);
    }
}

void source_set_channels(audio_source *src, int channels) { src->channels = channels; }
void source_set_bytes(audio_source *src, int bytes) { src->bytes = bytes; }
void source_set_frequency(audio_source *src, int frequency) { src->frequency = frequency; }
void source_set_loop(audio_source *src, int loop) { src->loop = loop; }

int source_get_channels(audio_source *src) { return src->channels; }
int source_get_bytes(audio_source *src) { return src->bytes; }
int source_get_frequency(audio_source *src) { return src->frequency; }
int source_get_loop(audio_source *src) { return src->loop; }

void source_set_userdata(audio_source *src, void *userdata) {
    src->userdata = userdata;
}

void* source_get_userdata(audio_source *src) {
    return src->userdata;
}

void source_set_update_cb(audio_source *src, source_update_cb cbfunc) {
    src->update = cbfunc;
}

void source_set_close_cb(audio_source *src, source_close_cb cbfunc) {
    src->close = cbfunc;
}
