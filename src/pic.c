#include <stdlib.h>
#include <string.h>

#include "shadowdive/error.h"
#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/pic.h"


sd_pic_file* sd_pic_create() {
    sd_pic_file *pic = malloc(sizeof(sd_pic_file));
    for(int i = 0; i < MAX_PIC_PHOTOS; i++) {
        pic->photos[i] = NULL;
    }
    return pic;
}

void free_photos(sd_pic_file *pic) {
    for(int i = 0; i < MAX_PIC_PHOTOS; i++) {
        if(pic->photos[i]) {
            if(pic->photos[i]->sprite)
                sd_sprite_delete(pic->photos[i]->sprite);
            free(pic->photos[i]);
            pic->photos[i] = NULL;
        }
    }
}

int sd_pic_load(sd_pic_file *pic, const char *filename) {
    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    if(sd_reader_filesize(r) < 200) {
        goto error_0;
    }

    // Basic info
    pic->photo_count = sd_read_dword(r);

    // Read offsets
    sd_reader_set(r, 200);
    int offset_list[256];
    for(int i = 0; i < pic->photo_count; i++) {
        offset_list[i] = sd_read_dword(r);
    }

    // Clear photos
    for(int i = 0; i < MAX_PIC_PHOTOS; i++) {
        pic->photos[i] = NULL;
    }

    // Read data
    for(int i = 0; i < pic->photo_count; i++) {
        // Set correct position
        sd_reader_set(r, offset_list[i]);

        // Reserve mem
        pic->photos[i] = malloc(sizeof(sd_pic_photo));

        // Read start bytes
        pic->photos[i]->is_player = sd_read_ubyte(r);
        pic->photos[i]->sex = sd_read_uword(r);

        // Read palette
        memset((void*)&pic->photos[i]->pal, 0, sizeof(sd_palette));
        char d[3];
        for(int k = 0; k < 48; k++) {
            sd_read_buf(r, d, 3);
            pic->photos[i]->pal.data[k][0] = ((d[0] << 2) | (d[0] >> 4));
            pic->photos[i]->pal.data[k][1] = ((d[1] << 2) | (d[1] >> 4));
            pic->photos[i]->pal.data[k][2] = ((d[2] << 2) | (d[2] >> 4));
        }

        // This byte is probably an "is there image data" flag
        // TODO: Find out what this does
        sd_skip(r, 1);

        // Sprite
        pic->photos[i]->sprite = sd_sprite_create();
        if(sd_sprite_load(r, pic->photos[i]->sprite) != SD_SUCCESS) {
            goto error_1;
        }

        // Fix length and width
        pic->photos[i]->sprite->img->h++;
        pic->photos[i]->sprite->img->w++;
    }

    sd_reader_close(r);
    return SD_SUCCESS;

error_1:
    free_photos(pic);

error_0:
    sd_reader_close(r);
    return SD_FILE_PARSE_ERROR;
}

int sd_pic_save(sd_pic_file *pic, const char *filename) {
    return SD_FILE_OPEN_ERROR;
}

void sd_pic_delete(sd_pic_file *pic) {
    free_photos(pic);
    free(pic);
}

