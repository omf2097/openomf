#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/error.h"
#include "shadowdive/palette.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

int main(int argc, char **argv) {
    char buf[256];

    if (argc != 2) {
        printf("Usage %s <input>\n", argv[0]);
        return 1;
    }
 
    sd_reader *r = sd_reader_open((const char *)argv[1]);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    sd_palette **palettes = malloc(11 * sizeof(sd_palette*));
    for(uint8_t i = 0; i < 11; i++) {
        palettes[i] = (sd_palette*)malloc(sizeof(sd_palette));
        sd_palette_load(r, palettes[i]);
        sprintf(buf, "altpal%d.pal", i);
        sd_palette_to_gimp_palette(palettes[i], buf);
    }
    return 0;
}
