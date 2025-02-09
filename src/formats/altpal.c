#include <stdlib.h>
#include <string.h>

#include "formats/altpal.h"
#include "formats/error.h"
#include "formats/internal/reader.h"
#include "formats/internal/writer.h"
#include "formats/palette.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"

altpal_file *altpals = NULL; // Extern

int altpals_init(void) {
    // Get filename
    const char *filename = pm_get_resource_path(DAT_ALTPALS);

    altpals = omf_calloc(1, sizeof(altpal_file));
    if(altpal_create(altpals) != SD_SUCCESS) {
        goto error_0;
    }
    if(altpals_load(altpals, filename) != SD_SUCCESS) {
        log_error("Unable to load altpals file '%s'!", filename);
        goto error_1;
    }
    log_info("Loaded altpals file '%s'.", filename);
    return 0;

error_1:
    altpal_free(altpals);
error_0:
    omf_free(altpals);
    return 1;
}

void altpals_close(void) {
    if(altpals != NULL) {
        altpal_free(altpals);
        omf_free(altpals);
    }
}

int altpal_create(altpal_file *ap) {
    if(ap == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(ap, 0, sizeof(altpal_file));
    return SD_SUCCESS;
}

int altpals_load(altpal_file *ap, const char *filename) {
    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    for(uint8_t i = 0; i < ALTPALS_PALETTES; i++) {
        palette_load_range(r, &ap->palettes[i], 0, 256);
    }

    // Close & return
    sd_reader_close(r);
    return SD_SUCCESS;
}

int altpals_save(altpal_file *ap, const char *filename) {
    sd_writer *w = sd_writer_open(filename);
    if(!w) {
        return SD_FILE_OPEN_ERROR;
    }

    for(uint8_t i = 0; i < ALTPALS_PALETTES; i++) {
        palette_save_range(w, &ap->palettes[i], 0, 256);
    }

    // All done.
    sd_writer_close(w);
    return SD_SUCCESS;
}

void altpal_free(altpal_file *ap) {
}
