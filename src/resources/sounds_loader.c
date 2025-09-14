#include "resources/sounds_loader.h"

#include "formats/error.h"
#include "formats/sounds.h"
#include "resource_files.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include <stdlib.h>

static sd_sound_file *sound_data = NULL;

bool sounds_loader_init(void) {
    path filename = get_resource_filename("SOUNDS.DAT");

    // Load sounds
    sound_data = omf_calloc(1, sizeof(sd_sound_file));
    if(sd_sounds_create(sound_data) != SD_SUCCESS) {
        goto error_0;
    }
    if(sd_sounds_load(sound_data, &filename)) {
        log_error("Unable to load sounds file '%s'!", path_c(&filename));
        goto error_1;
    }
    log_info("Loaded sounds file '%s'.", path_c(&filename));
    return true;

error_1:
    sd_sounds_free(sound_data);
error_0:
    omf_free(sound_data);
    return false;
}

bool sounds_loader_get(int id, char **buffer, int *len, int *freq) {
    // Make sure the data is ok and sound exists
    if(sound_data == NULL)
        return false;

    const sd_sound *sample = sd_sounds_get(sound_data, id);
    if(sample == NULL) {
        log_error("Requested sound %d does not exist!", id);
        return false;
    }
    *buffer = sample->data;
    *len = sample->len;
    *freq = 1000000 / (256 - sample->unknown);
    return true;
}

void sounds_loader_close(void) {
    if(sound_data != NULL) {
        sd_sounds_free(sound_data);
        omf_free(sound_data);
    }
}
