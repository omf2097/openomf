#include "audio/soundloader.h"
#include "audio/sound.h"
#include "utils/log.h"
#include <shadowdive/shadowdive.h>

sd_sound_file *sdf = 0;

int soundloader_init(const char *filename) {
    sdf = sd_sounds_create();
    if(sd_sounds_load(sdf, filename)) {
        sd_sounds_delete(sdf);
        PERROR("Unable to load sounds file!");
        return 1;
    }
    DEBUG("Soundloader loaded '%s' !", filename);
    return 0;
}

void soundloader_play(unsigned int sound, sound_state *ss) {
    // Make sure sound exists
    if(sound > sdf->sound_count) {
        return;
    }
    
    // Get sound
    sd_sound *sample = sdf->sounds[sound];
    if(sample == 0) {
        return;
    }
    
    // Play sound
    sound_play(sample->data, sample->len, ss);
}

void soundloader_close() {
    sd_sounds_delete(sdf);
    sdf = 0;
}