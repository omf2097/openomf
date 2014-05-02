#include <string.h>
#include <stdlib.h>
#include "resources/palette.h"
#include "resources/ids.h"
#include "resources/pathmanager.h"
#include "utils/log.h"

sd_altpal_file *altpals = NULL;

int altpals_init() {
    // Get filename
    const char *filename = pm_get_resource_path(DAT_ALTPALS);

    altpals = sd_altpal_create();
    if(sd_altpals_load(altpals, filename)) {
        sd_altpal_delete(altpals);
        PERROR("Unable to load altpals file '%s'!", filename);
        return 1;
    }
    INFO("Loaded altpals file '%s'.", filename);
    return 0;
}

void altpals_close() {
    if(altpals != NULL) {
        sd_altpal_delete(altpals);
    }
}

void palette_set_player_color(palette *palette, int player, int srccolor, int dstcolor) {
    int dst = dstcolor * 16 + player * 48;
    int src = srccolor * 16;
    char iz[3];
    memcpy(iz, palette->data, 3);
    memcpy(palette->data + dst, altpals->palettes[0].data + src, 16 * 3);
    memcpy(palette->data, iz, 3);
}

palette* palette_copy(palette *src) {
    palette *new = malloc(sizeof(palette));
    memcpy(new->data, src->data, 256*3);
    memcpy(new->remaps, src->remaps, 19*256);
    return new;
}
