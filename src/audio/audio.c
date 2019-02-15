#include <stdlib.h>
#include <string.h>
#include "audio/audio.h"
#include "audio/sink.h"
#include "audio/sinks/openal_sink.h"
#include "utils/log.h"

audio_sink *_global_sink = NULL;

struct sink_info_t {
    int (*sink_init_fn)(audio_sink *sink);
    const char* name;
} sinks[] = {
#ifdef USE_OPENAL
    {openal_sink_init, "openal"},
#endif // USE_OPENAL
    {NULL, NULL}
};

int audio_get_sink_count() {
    int count = 0;
    while(sinks[count].name != NULL) {
        count++;
    }
    return count;
}

const char* audio_get_sink_name(int sink_id) {
    // Get sink
    if(sink_id < 0 || sink_id >= audio_get_sink_count()) {
        return NULL;
    }
    return sinks[sink_id].name;
}

const char* audio_get_first_sink_name() {
    if(audio_get_sink_count() > 0) {
        return sinks[0].name;
    }
    return NULL;
}

int audio_is_sink_available(const char* sink_name) {
    for(int i = 0; i < audio_get_sink_count(); i++) {
        if(strcmp(sink_name, sinks[i].name) == 0) {
            return 1;
        }
    }
    return 0;
}

void audio_render() {
    if(_global_sink != NULL) {
        sink_render(_global_sink);
    }
}

int audio_init(const char* sink_name) {
    struct sink_info_t si;
    int found = 0;

    // If null sink given, disable audio
    if(sink_name == NULL || strlen(sink_name) <= 0) {
        INFO("Audio sink NOT initialized; audio not available.");
        return 0;
    }

    // Find requested sink
    int c = 0;
    while(sinks[c].name != NULL) {
        if(strcmp(sink_name, sinks[c].name) == 0) {
            si = sinks[c];
            found = 1;
            break;
        }
        c++;
    }
    if(!found) {
        PERROR("Requested audio sink was not found!");
        return 1;
    }

    // Inform user
    INFO("Using audio sink '%s'.", si.name);

    // Init sink
    _global_sink = malloc(sizeof(audio_sink));
    sink_init(_global_sink);
    if(si.sink_init_fn(_global_sink) != 0) {
        free(_global_sink);
        _global_sink = NULL;
        return 1;
    }

    // Success
    INFO("Audio system initialized.");
    return 0;
}

void audio_close() {
    if(_global_sink != NULL) {
        sink_free(_global_sink);
        free(_global_sink);
        _global_sink = NULL;
        INFO("Audio system closed.");
    }
}

audio_sink* audio_get_sink() {
    return _global_sink;
}
