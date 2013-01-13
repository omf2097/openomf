#ifndef _AUDIO_H
#define _AUDIO_H

int audio_init();
void audio_render(int dt);
void audio_play(audio_stream *stream);
void audio_close();
audio_stream* audio_get_music();

#endif // _AUDIO_H
