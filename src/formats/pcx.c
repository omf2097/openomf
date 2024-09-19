#include "formats/pcx.h"
#include "formats/error.h"
#include "formats/internal/reader.h"
#include "utils/allocator.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static unsigned decode_next_bytes(char *dest, sd_reader *reader) {
    uint8_t fst_byte = sd_read_ubyte(reader);
    uint8_t repeat = 0;
    if(fst_byte >= 192) {
        repeat = fst_byte - 192;
    }
    if(repeat == 0) {
        dest[0] = fst_byte;
        return 1;
    }
    uint8_t snd_byte = sd_read_ubyte(reader);
    for(unsigned i = 0; i < repeat; ++i) {
        dest[i] = snd_byte;
    }
    return repeat;
}

int pcx_load(pcx_file *pcx, const char *filename) {
    memset(pcx, 0, sizeof(*pcx));

    sd_reader *reader = sd_reader_open(filename);
    if(reader == NULL) {
        return SD_FILE_INVALID_TYPE;
    }

    // The header is 128 bytes.
    long filesize = sd_reader_filesize(reader);
    if(filesize <= 128) {
        sd_reader_close(reader);
        return SD_FILE_INVALID_TYPE;
    }

    pcx->manufacturer = sd_read_ubyte(reader);
    pcx->version = sd_read_ubyte(reader);
    pcx->encoding = sd_read_ubyte(reader);
    pcx->bits_per_plane = sd_read_ubyte(reader);

    pcx->window_x_min = sd_read_uword(reader);
    pcx->window_y_min = sd_read_uword(reader);
    pcx->window_x_max = sd_read_uword(reader);
    pcx->window_y_max = sd_read_uword(reader);

    pcx->horz_dpi = sd_read_uword(reader);
    pcx->vert_dpi = sd_read_uword(reader);

    if(sd_read_buf(reader, (void *)pcx->header_palette, 48) != 1) {
        sd_reader_close(reader);
        return SD_FILE_READ_ERROR;
    }
    pcx->reserved = sd_read_ubyte(reader);
    pcx->color_planes = sd_read_ubyte(reader);

    pcx->bytes_per_plane_line = sd_read_uword(reader);
    pcx->palette_info = sd_read_uword(reader);
    pcx->hor_scr_size = sd_read_uword(reader);
    pcx->ver_scr_size = sd_read_uword(reader);

    if(sd_reader_set(reader, 128) != 1) {
        sd_reader_close(reader);
        return SD_FILE_INVALID_TYPE;
    }

    int ret;
    if((ret = sd_vga_image_create(&(pcx->image), 320, 200)) != SD_SUCCESS) {
        return ret;
    }

    for(unsigned j = 0; j < 200; ++j) {
        for(unsigned i = 0; i < 320;) {
            i += decode_next_bytes(&(pcx->image.data)[(320 * j) + i], reader);
        }
    }

    char foo = sd_read_ubyte(reader);
    if(foo != 0x0c) {
        return SD_FILE_READ_ERROR;
    }

    if(sd_read_buf(reader, (void *)&(pcx->palette.data), 768) != 1) {
        sd_reader_close(reader);
        return SD_FILE_READ_ERROR;
    }

    sd_reader_close(reader);
    return SD_SUCCESS;
}

int pcx_load_font(pcx_font *font, const char *filename) {
    memset(font, 0, sizeof(pcx_font));
    int ret = SD_SUCCESS;
    if((ret = pcx_load(&font->pcx, filename)) != SD_SUCCESS) {
        return ret;
    }

    char corner_color = font->pcx.image.data[0];

    // check this is actually a font sheet
    if(font->pcx.image.data[1] != corner_color || font->pcx.image.data[320] != corner_color ||
       font->pcx.image.data[321] == corner_color) {
        return SD_FORMAT_NOT_SUPPORTED;
    }

    // figure out the height of the font, we can do this just once
    font->glyph_height = 0;
    for(int i = 1; i < 200; i++) {
        if(font->pcx.image.data[(320 * i) + 1] == corner_color) {
            break;
        }
        font->glyph_height++;
    }

    int glyph = 0;
    // we know the first row never has a font pixel in it, so start at the second line
    for(int i = 1; i < 200 && font->pcx.image.data[(320 * (i + font->glyph_height)) + 1] == corner_color;
        i += font->glyph_height + 1) {
        // glyph starts on the first column
        int width = 0;
        for(int j = 1; j < 320; j++) {
            if(font->pcx.image.data[(320 * i) + j] == corner_color) {
                font->glyphs[glyph].width = width;
                font->glyphs[glyph].x = j - width;
                font->glyphs[glyph].y = i;
                glyph++;
                width = 0;
            } else {
                width++;
            }
        }
    }

    font->glyph_count = glyph;

    return SD_SUCCESS;
}

int pcx_font_decode(const pcx_font *font, sd_rgba_image *o, uint8_t ch) {
    if(ch >= font->glyph_count || font == NULL || o == NULL) {
        return SD_INVALID_INPUT;
    }

    int k = 0;
    for(int i = font->glyphs[ch].y; i < font->glyphs[ch].y + font->glyph_height; i++) {
        int l = 0;
        for(int j = font->glyphs[ch].x; j < font->glyphs[ch].x + font->glyphs[ch].width; j++) {
            if(font->pcx.image.data[(i * 320) + j]) {
                o->data[(k * font->glyphs[ch].width * 4) + l] =
                    font->pcx.palette.data[(int)font->pcx.image.data[(i * 320) + j]][0];
                o->data[(k * font->glyphs[ch].width * 4) + l + 1] =
                    font->pcx.palette.data[(int)font->pcx.image.data[(i * 320) + j]][1];
                o->data[(k * font->glyphs[ch].width * 4) + l + 2] =
                    font->pcx.palette.data[(int)font->pcx.image.data[(i * 320) + j]][2];
                o->data[(k * font->glyphs[ch].width * 4) + l + 3] = (char)255;
            } else {
                o->data[(k * font->glyphs[ch].width * 4) + l] = 0;
                o->data[(k * font->glyphs[ch].width * 4) + l + 1] = 0;
                o->data[(k * font->glyphs[ch].width * 4) + l + 2] = 0;
                o->data[(k * font->glyphs[ch].width * 4) + l + 3] = 0;
            }

            l += 4;
        }
        k++;
    }
    return SD_SUCCESS;
}

void pcx_free(pcx_file *pcx) {
    if(pcx == NULL)
        return;
    sd_vga_image_free(&pcx->image);
    palette_free(&pcx->palette);
}

void pcx_font_free(pcx_font *font) {
    if(font == NULL)
        return;
    sd_vga_image_free(&font->pcx.image);
    palette_free(&font->pcx.palette);
}
