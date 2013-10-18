#ifndef _AUDIO_H
#define _AUDIO_H

#include "audio/stream.h"

int audio_init();
void audio_render(int dt);
void audio_play(audio_stream *stream);
void audio_close();
audio_stream* audio_get_music();
void audio_set_volume(int type, float vol);

#endif // _AUDIO_H
