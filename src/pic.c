#include <stdlib.h>
#include <string.h>

#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/error.h"
#include "shadowdive/pic.h"

int sd_pic_create(sd_pic_file *pic) {
    if(pic == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(pic, 0, sizeof(sd_pic_file));
    return SD_SUCCESS;
}

void free_photos(sd_pic_file *pic) {
    if(pic == NULL) return;
    for(int i = 0; i < MAX_PIC_PHOTOS; i++) {
        if(pic->photos[i]) {
            if(pic->photos[i]->sprite) {
                sd_sprite_free(pic->photos[i]->sprite);
            }
            free(pic->photos[i]);
            pic->photos[i] = NULL;
        }
    }
}

int sd_pic_load(sd_pic_file *pic, const char *filename) {
    int ret = SD_FILE_PARSE_ERROR;
    if(pic == NULL || filename == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    // The filesize should be at least 200 bytes.
    if(sd_reader_filesize(r) < 200) {
        goto error_0;
    }

    // Basic info
    pic->photo_count = sd_read_dword(r);
    if(pic->photo_count >= 256 || pic->photo_count < 0) {
        goto error_0;
    }

    // Read offsets
    sd_reader_set(r, 200);
    int offset_list[256];
    memset(offset_list, 0, sizeof(offset_list));
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
        sd_palette_create(&pic->photos[i]->pal);
        sd_palette_load_range(r, &pic->photos[i]->pal, 0, 48);

        // This byte is probably an "is there image data" flag
        // TODO: Find out what this does
        pic->photos[i]->unk_flag = sd_read_ubyte(r);

        // Sprite
        pic->photos[i]->sprite = malloc(sizeof(sd_sprite));
        sd_sprite_create(pic->photos[i]->sprite);
        if((ret = sd_sprite_load(r, pic->photos[i]->sprite)) != SD_SUCCESS) {
            goto error_1;
        }

        // Fix length and width
        pic->photos[i]->sprite->height++;
        pic->photos[i]->sprite->width++;
    }

    sd_reader_close(r);
    return SD_SUCCESS;

error_1:
    free_photos(pic);

error_0:
    sd_reader_close(r);
    return ret;
}

int sd_pic_save(const sd_pic_file *pic, const char *filename) {
    if(pic == NULL || filename == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_writer *w = sd_writer_open(filename);
    if(!w) {
        return SD_FILE_OPEN_ERROR;
    }

    // Write photo count, and then fill zero until 0xC8
    sd_write_dword(w, pic->photo_count);
    sd_write_fill(w, 0, 200 - sd_writer_pos(w));

    // Offset list goes here. For now, just write zero
    // We will fill this out later.
    sd_write_fill(w, 0, pic->photo_count * 4);

    // Write photos and offsets
    for(int i = 0; i < pic->photo_count; i++) {
        // Write offset to the catalog
        uint32_t pos = sd_writer_pos(w);
        sd_writer_seek_start(w, 200 + i * 4);
        sd_write_udword(w, pos);
        sd_writer_seek_start(w, pos);

        // flags, palette, etc.
        sd_write_ubyte(w, pic->photos[i]->is_player);
        sd_write_uword(w, pic->photos[i]->sex);
        sd_palette_save_range(w, &pic->photos[i]->pal, 0, 48);
        sd_write_ubyte(w, pic->photos[i]->unk_flag);

        // Hackity hack. Sprite w and h should be n-1 for some reason.
        pic->photos[i]->sprite->height--;
        pic->photos[i]->sprite->width--;
        sd_sprite_save(w, pic->photos[i]->sprite);
        pic->photos[i]->sprite->height++;
        pic->photos[i]->sprite->width++;
    }

    sd_writer_close(w);
    return SD_SUCCESS;
}

const sd_pic_photo* sd_pic_get(const sd_pic_file *pic, int entry_id) {
    if(entry_id < 0 || entry_id > pic->photo_count) {
        return NULL;
    }
    return pic->photos[entry_id];
}

void sd_pic_free(sd_pic_file *pic) {
    free_photos(pic);
}

