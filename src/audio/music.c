#include "audio/music.h"
#include "audio/stream.h"
#include "audio/audio.h"
#include "utils/log.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <dumb/dumb.h>

typedef struct music_file_t {
    DUH_SIGRENDERER *renderer;
    DUH *data;
    float delta;
    int bps;
} music_file;

int music_update(audio_stream *stream, char *buf, int len) {
    music_file *mf = (music_file*)stream->userdata;
    return duh_render(mf->renderer, 16, 0, 1.0f, mf->delta, len / mf->bps, buf) * mf->bps;
}

void music_close(audio_stream *stream) {
    music_file *mf = (music_file*)stream->userdata;
    duh_end_sigrenderer(mf->renderer);
    unload_duh(mf->data);
    free(mf);
}

int music_play(const char *filename) {
    DUH *data = dumb_load_psm(filename, 0);
    if(!data) {
        PERROR("Error while loading PSM file!");
        return 1;
    }
    
    // Open renderer
    DUH_SIGRENDERER *renderer = duh_start_sigrenderer(data, 0, 2, 0);
    
    // Music file struct for userdata
    music_file *mf = malloc(sizeof(music_file));
    mf->data = data;
    mf->renderer = renderer;
    mf->delta = 65536.0f / 44100;
    mf->bps = 2 * 2;
    
    // Create stream
    audio_stream stream;
    stream.frequency = 44100;
    stream.channels = 2;
    stream.bytes = 2;
    stream.type = TYPE_MUSIC;
    stream.userdata = (void*)mf;
    stream.preupdate = NULL;
    stream.update = music_update;
    stream.close = music_close;
    
    // Create openal stream
    if(audio_stream_create(&stream)) {
        unload_duh(mf->data);
        free(mf);
        return 1;
    }
    
    // Play
    audio_play(&stream);
    
    // All done
    return 0;
}

void music_stop() {
    audio_stream *stream = audio_get_music();
    if(stream) {
        audio_stream_stop(stream);
    }
}

int music_playing() {
    audio_stream *stream = audio_get_music();
    if(!stream) return 0;
    return audio_stream_playing(stream);
}