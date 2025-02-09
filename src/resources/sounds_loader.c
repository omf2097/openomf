#include "resources/sounds_loader.h"
#include "formats/error.h"
#include "formats/sounds.h"
#include "resources/ids.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include <stdlib.h>

static sd_sound_file *sound_data = NULL;

bool sounds_loader_init(void) {
    const char *filename = pm_get_resource_path(DAT_SOUNDS);

    // Load sounds
    sound_data = omf_calloc(1, sizeof(sd_sound_file));
    if(sd_sounds_create(sound_data) != SD_SUCCESS) {
        goto error_0;
    }
    if(sd_sounds_load(sound_data, filename)) {
        log_error("Unable to load sounds file '%s'!", filename);
        goto error_1;
    }
    log_info("Loaded sounds file '%s'.", filename);
    return true;

error_1:
    sd_sounds_free(sound_data);
error_0:
    omf_free(sound_data);
    return false;
}

bool sounds_loader_get(int id, char **buffer, int *len) {
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
    return true;
}

void sounds_loader_close(void) {
    if(sound_data != NULL) {
        sd_sounds_free(sound_data);
        omf_free(sound_data);
    }
}
