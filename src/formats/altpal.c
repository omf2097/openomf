#include <stdlib.h>
#include <string.h>

#include "resources/pathmanager.h"
#include "formats/internal/reader.h"
#include "formats/internal/writer.h"
#include "formats/error.h"
#include "formats/palette.h"
#include "formats/altpal.h"
#include "utils/allocator.h"
#include "utils/log.h"

sd_altpal_file *altpals = NULL;

int altpals_init() {
    // Get filename
    const char *filename = pm_get_resource_path(DAT_ALTPALS);

    altpals = omf_calloc(1, sizeof(sd_altpal_file));
    if(sd_altpal_create(altpals) != SD_SUCCESS) {
        goto error_0;
    }
    if(sd_altpals_load(altpals, filename) != SD_SUCCESS) {
        PERROR("Unable to load altpals file '%s'!", filename);
        goto error_1;
    }
    INFO("Loaded altpals file '%s'.", filename);
    return 0;

error_1:
    sd_altpal_free(altpals);
error_0:
    omf_free(altpals);
    return 1;
}

void altpals_close() {
    if(altpals != NULL) {
        sd_altpal_free(altpals);
        omf_free(altpals);
    }
}

int sd_altpal_create(sd_altpal_file *ap) {
    if(ap == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(ap, 0, sizeof(sd_altpal_file));
    return SD_SUCCESS;
}

int sd_altpals_load(sd_altpal_file *ap, const char *filename) {
    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    for(uint8_t i = 0; i < SD_ALTPALS_PALETTES; i++) {
        palette_create(&ap->palettes[i]);
        palette_load_range(r, &ap->palettes[i], 0, 256);
    }

    // Close & return
    sd_reader_close(r);
    return SD_SUCCESS;
}

int sd_altpals_save(sd_altpal_file *ap, const char *filename) {
    sd_writer *w = sd_writer_open(filename);
    if(!w) {
        return SD_FILE_OPEN_ERROR;
    }

    for(uint8_t i = 0; i < SD_ALTPALS_PALETTES; i++) {
        palette_save_range(w, &ap->palettes[i], 0, 256);
    }

    // All done.
    sd_writer_close(w);
    return SD_SUCCESS;
}

void sd_altpal_free(sd_altpal_file *ap) {
}

