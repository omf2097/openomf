#include <stdlib.h>
#include "formats/sounds.h"
#include "formats/error.h"
#include "resources/sounds_loader.h"
#include "resources/ids.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"

sd_sound_file *sound_data = NULL;

int sounds_loader_init() {
    // Get filename
    const char *filename = pm_get_resource_path(DAT_SOUNDS);

    // Load sounds
    sound_data = omf_calloc(1, sizeof(sd_sound_file));
    if(sd_sounds_create(sound_data) != SD_SUCCESS) {
        goto error_0;
    }
    if(sd_sounds_load(sound_data, filename)) {
        PERROR("Unable to load sounds file '%s'!", filename);
        goto error_1;
    }
    INFO("Loaded sounds file '%s'.", filename);
    return 0;

error_1:
    sd_sounds_free(sound_data);
error_0:
    free(sound_data);
    sound_data = NULL;
    return 1;
}

int sounds_loader_get(int id, char **buffer, int *len) {
    // Make sure the data is ok and sound exists
    if(sound_data == NULL) return 1;

    // Get sound
    const sd_sound *sample = sd_sounds_get(sound_data, id);
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
        sd_sounds_free(sound_data);
        free(sound_data);
        sound_data = NULL;
    }
}
