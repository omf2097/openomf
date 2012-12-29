#ifndef _SOUNDS_H
#define _SOUNDS_H

typedef struct sd_writer_t sd_writer;
typedef struct sd_reader_t sd_reader;

typedef struct sd_sound_t {
    unsigned int len;
    char *data;
} sd_sound;

typedef struct sd_soundarray_t {
    unsigned int len;
    sd_sound **sounds;
} sd_soundarray;



#endif // _SOUNDS_H
