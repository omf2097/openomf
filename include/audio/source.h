#ifndef SOURCE_H
#define SOURCE_H

typedef struct audio_source_t audio_source;

typedef int (*source_update_cb)(audio_source *src, char *buffer, int len);
typedef void (*source_close_cb)(audio_source *src);

struct audio_source_t {
    int frequency;
    int resampler;
    int channels;
    int bytes;
    int loop;
    void *userdata;
    source_update_cb update;
    source_close_cb close;
};

typedef struct {
    int internal_id;
    int is_default;
    const char *name;
} audio_source_resampler;

typedef struct {
    int freq;
    int is_default;
    const char *name;
} audio_source_freq;

void source_init(audio_source *src);
int source_update(audio_source *src, char *buffer, int len);
void source_free(audio_source *src);

void source_set_channels(audio_source *src, int channels);
void source_set_bytes(audio_source *src, int bytes);
void source_set_frequency(audio_source *src, int frequency);
void source_set_loop(audio_source *src, int loop);
void source_set_resampler(audio_source *src, int resampler);

int source_get_channels(audio_source *src);
int source_get_bytes(audio_source *src);
int source_get_frequency(audio_source *src);
int source_get_loop(audio_source *src);
int source_get_resampler(audio_source *src);

void source_set_userdata(audio_source *src, void *userdata);
void *source_get_userdata(audio_source *src);
void source_set_update_cb(audio_source *src, source_update_cb cbfunc);
void source_set_close_cb(audio_source *src, source_close_cb cbfunc);

#endif // SOURCE_H
