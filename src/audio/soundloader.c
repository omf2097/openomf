#include "audio/sound.h"
#include "utils/log.h"
#include <shadowdive/shadowdive.h>
#include "audio/sound_state.h"
#include "audio/soundloader.h"
#include "resources/ids.h"

sd_sound_file *sdf = 0;

int soundloader_init() {
    // Get filename
    char filename[64];
    get_filename_by_id(DAT_SOUNDS, filename);

    // Load sounds
    sdf = sd_sounds_create();
    if(sd_sounds_load(sdf, filename)) {
        sd_sounds_delete(sdf);
        PERROR("Unable to load sounds file '%s'!", filename);
        return 1;
    }
    DEBUG("Loaded sounds file '%s'.", filename);
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
