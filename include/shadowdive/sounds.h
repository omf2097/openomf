#ifndef _SD_SOUNDS_H
#define _SD_SOUNDS_H

#ifdef __cplusplus 
extern "C" {
#endif

#define SD_SOUNDS_MAX 299

typedef struct {
    unsigned int len;
    char *data;
} sd_sound;

typedef struct {
    sd_sound sounds[SD_SOUNDS_MAX];
} sd_sound_file;

int sd_sounds_create(sd_sound_file *sf);
void sd_sounds_free(sd_sound_file *sf);
sd_sound* sd_sounds_get(sd_sound_file *sf, int id);
int sd_sound_to_au(sd_sound *sound, const char *filename);
int sd_sound_from_au(sd_sound *sound, const char *filename);
int sd_sounds_load(sd_sound_file *sf, const char *filename);
int sd_sounds_save(sd_sound_file *sf, const char *filename);

#ifdef __cplusplus
}
#endif

#endif // _SD_SOUNDS_H
