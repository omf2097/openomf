#include "palette.h"
#include "internal/reader.h"
#include "internal/writer.h"
#include <stdint.h>

int sd_palette_load(sd_reader *reader, sd_palette *palette) {
    char d[3];
    for(int i = 0; i < 256; i++) {
        sd_read_buf(reader, d, 3);
        palette->data[i][0] = ((d[0] << 2) | (d[0] >> 4));
        palette->data[i][1] = ((d[1] << 2) | (d[1] >> 4));
        palette->data[i][2] = ((d[2] << 2) | (d[2] >> 4));
    }
    sd_read_buf(reader, (char*)palette->remaps, 19*256);
    return sd_reader_ok(reader);
}

void sd_palette_save(sd_writer *writer, sd_palette *palette) {
    char *d;
    for(int i = 0; i < 256; i++) {
        d = palette->data[i];
        // for some reason, we need to mask off the high bits or the bitshift doesn't work
        sd_write_ubyte(writer, (d[0] & 0xff) >> 2);
        sd_write_ubyte(writer, (d[1] & 0xff) >> 2);
        sd_write_ubyte(writer, (d[2] & 0xff) >> 2);
    }
    sd_write_buf(writer, (char*)palette->remaps, 19*256);
}
