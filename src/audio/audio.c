#include <stdlib.h>
#include "audio/audio.h"
#include "audio/sink.h"
#include "audio/sinks/openal_sink.h"
#include "utils/log.h"

#define SINK_COUNT 1

audio_sink *_global_sink = NULL;

struct sink_info_t {
    int (*sink_init_fn)(audio_sink *sink);
    const char* name;
} sinks[] = {
    {openal_sink_init, "openal"},
};

int audio_get_sink_count() {
    return SINK_COUNT;
}

const char* audio_get_sink_name(int sink_id) {
    // Get sink
    if(sink_id < 0 || sink_id >= SINK_COUNT) {
        return NULL;
    }
    struct sink_info_t si = sinks[sink_id];

    return si.name;
}

void audio_render() {
    sink_render(_global_sink);
}

int audio_init(int sink_id) {
    // Get sink
    if(sink_id < 0 || sink_id >= SINK_COUNT) {
        return 1;
    }
    struct sink_info_t si = sinks[sink_id];

    // Inform user
    INFO("Initializing audio sink '%s'.", audio_get_sink_name(sink_id));

    // Init sink
    _global_sink = malloc(sizeof(audio_sink));
    sink_init(_global_sink);
    if(si.sink_init_fn(_global_sink) != 0) {
        free(_global_sink);
        _global_sink = NULL;
        return 1;
    }

    // Success
    return 0;
}

void audio_close() {
    if(_global_sink != NULL) {
        sink_free(_global_sink);
    }
    free(_global_sink);
    _global_sink = NULL;
    INFO("Audio deinit.");
}

audio_sink* audio_get_sink() {
    return _global_sink;
}
