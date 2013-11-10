#include <stdlib.h>
#include <shadowdive/shadowdive.h>
#include "resources/sounds_loader.h"
#include "resources/ids.h"
#include "utils/log.h"

sd_sound_file *sound_data = NULL;

int sounds_loader_init() {
    // Get filename
    char filename[64];
    get_filename_by_id(DAT_SOUNDS, filename);

    // Load sounds
    sound_data = sd_sounds_create();
    if(sd_sounds_load(sound_data, filename)) {
        sd_sounds_delete(sound_data);
        PERROR("Unable to load sounds file '%s'!", filename);
        return 1;
    }
    DEBUG("Loaded sounds file '%s'.", filename);
    return 0;
}

int sounds_loader_get(int id, char **buffer, int *len) {
    // Make sure the data is ok and sound exists
    if(sound_data == NULL) return 1;
    if(id > sound_data->sound_count) return 1;
    
    // Get sound
    sd_sound *sample = sound_data->sounds[id];
    if(sample == NULL) {
        PERROR("Requested sound %d does not exist!", id);
        return 1;
    }

    // Get much data!
    *buffer = sample->data;
    *len = sample->len;

    return 0; // Success
}

void sounds_loader_close() {
    if(sound_data != NULL) {
        sd_sounds_delete(sound_data);
        sound_data = NULL;
    }
}
