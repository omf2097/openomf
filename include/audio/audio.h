#ifndef _AUDIO_H
#define _AUDIO_H

typedef struct audio_stream_t audio_stream;

int audio_init();
void audio_render(int dt);
void audio_play(audio_stream *stream);
void audio_close();

#endif // _AUDIO_H