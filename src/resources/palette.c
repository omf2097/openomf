#include <string.h>
#include <stdlib.h>
#include "resources/palette.h"
#include "resources/ids.h"
#include "utils/log.h"

sd_altpal_file *altpals = NULL;

int altpals_init() {
    // Get filename
    char filename[64];
    get_filename_by_id(DAT_ALTPALS, filename);

    altpals = sd_altpal_create();
    if(sd_altpals_load(altpals, filename)) {
        sd_altpal_delete(altpals);
        PERROR("Unable to load altpals file '%s'!", filename);
        return 1;
    }
    DEBUG("Loaded altpals file '%s'.", filename);
    return 0;
}

void altpals_close() {
    if(altpals != NULL) {
        sd_altpal_delete(altpals);
    }
}

void palette_set_player_color(palette *palette, int sourcecolor, int destcolor) {
    DEBUG("copying 16 bytes into palette at %d", destcolor*16);
    memcpy(palette->data+destcolor*16, altpals->palettes[0].data+(sourcecolor*16), 16*3);
}

palette* palette_copy(palette *src) {
    palette *new = malloc(sizeof(palette));
    memcpy(new->data, src->data, 256*3);
    memcpy(new->remaps, src->remaps, 19*256);
    return new;
}
