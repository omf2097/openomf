#ifndef _SOUNDS_H
#define _SOUNDS_H

typedef struct sd_writer_t sd_writer;
typedef struct sd_reader_t sd_reader;

typedef struct sd_sound_t {
    unsigned int len;
    char *data;
} sd_sound;

typedef struct sd_sound_file_t {
    unsigned int sound_count;
    sd_sound **sounds;
} sd_sound_file;

sd_sound_file* sd_af_load(const char *filename);
int sd_af_save(const char* filename, sd_sound_file *sf);
void sd_af_delete(sd_sound_file *sf);

#endif // _SOUNDS_H
