#ifndef _SD_SOUNDS_H
#define _SD_SOUNDS_H

#ifdef __cplusplus 
extern "C" {
#endif

typedef struct {
    unsigned int len;
    char *data;
} sd_sound;

typedef struct {
    unsigned int sound_count;
    sd_sound **sounds;
} sd_sound_file;

int sd_sounds_create(sd_sound_file *sf);
void sd_sounds_free(sd_sound_file *sf);
int sd_sound_to_au(sd_sound *sound, const char *filename);
int sd_sounds_load(sd_sound_file *sf, const char *filename);
int sd_sounds_save(sd_sound_file *sf, const char *filename);

#ifdef __cplusplus
}
#endif

#endif // _SD_SOUNDS_H
