#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "formats/error.h"
#include "formats/internal/reader.h"
#include "formats/internal/writer.h"
#include "formats/pic.h"
#include "utils/allocator.h"

static void sd_pic_photo_free_cb(void *ptr) {
    sd_pic_photo *photo = ptr;
    if(photo->sprite) {
        sd_sprite_free(photo->sprite);
        omf_free(photo->sprite);
    }
}

void sd_pic_create(sd_pic_file *pic) {
    assert(pic != NULL);
    memset(pic, 0, sizeof(sd_pic_file));
    vector_create_cb(&pic->photos, sizeof(sd_pic_photo), sd_pic_photo_free_cb);
}

int sd_pic_load(sd_pic_file *pic, const path *filename) {
    int ret = SD_FILE_PARSE_ERROR;
    assert(pic != NULL);
    assert(filename != NULL);

    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    // The filesize should be at least 200 bytes.
    if(sd_reader_filesize(r) < 200) {
        goto error_0;
    }

    // Basic info
    const int photo_count = sd_read_dword(r);
    if(photo_count >= 256 || photo_count < 0) {
        goto error_0;
    }

    // Read offsets
    sd_reader_set(r, 200);
    int offset_list[256];
    memset(offset_list, 0, sizeof(offset_list));
    for(int i = 0; i < photo_count; i++) {
        offset_list[i] = sd_read_dword(r);
    }

    // Read data
    vector_reserve(&pic->photos, photo_count);
    for(int i = 0; i < photo_count; i++) {
        // Set correct position
        sd_reader_set(r, offset_list[i]);

        sd_pic_photo *photo = vector_append_ptr(&pic->photos);
        memset(photo, 0, sizeof(sd_pic_photo));

        // Read start bytes
        photo->is_player = sd_read_ubyte(r);
        photo->sex = sd_read_uword(r);

        // Read palette
        vga_palette_init(&photo->pal);
        palette_load_range(r, &photo->pal, 0, 48);

        // Nonzero if photo sprite data follows.
        photo->has_photo = sd_read_ubyte(r);

        // Sprite
        photo->sprite = omf_calloc(1, sizeof(sd_sprite));
        sd_sprite_create(photo->sprite);
        if((ret = sd_sprite_load(r, photo->sprite)) != SD_SUCCESS) {
            goto error_1;
        }

        // Fix length and width
        photo->sprite->height++;
        photo->sprite->width++;
    }

    sd_reader_close(r);
    return SD_SUCCESS;

error_1:
    // The vector's free callback frees each loaded photo's sprite.
    vector_clear(&pic->photos);

error_0:
    sd_reader_close(r);
    return ret;
}

int sd_pic_save(const sd_pic_file *pic, const path *filename) {
    assert(pic != NULL);
    assert(filename != NULL);

    sd_writer *w = sd_writer_open(filename);
    if(!w) {
        return SD_FILE_OPEN_ERROR;
    }

    // Write photo count, and then fill zero until 0xC8
    const int photo_count = vector_size(&pic->photos);
    sd_write_dword(w, photo_count);
    sd_write_fill(w, 0, 200 - sd_writer_pos(w));

    // Offset list goes here. For now, just write zero
    // We will fill this out later.
    sd_write_fill(w, 0, photo_count * 4);

    // Write photos and offsets
    for(int i = 0; i < photo_count; i++) {
        sd_pic_photo *photo = vector_get(&pic->photos, i);
        // Write offset to the catalog
        long pos = sd_writer_pos(w);
        if(pos < 0) {
            goto error;
        }
        if(sd_writer_seek_start(w, 200 + i * 4) < 0) {
            goto error;
        }
        sd_write_udword(w, (uint32_t)pos);
        if(sd_writer_seek_start(w, pos) < 0) {
            goto error;
        }

        // flags, palette, etc.
        sd_write_ubyte(w, photo->is_player);
        sd_write_uword(w, photo->sex);
        palette_save_range(w, &photo->pal, 0, 48);
        sd_write_ubyte(w, photo->has_photo);

        // Hackity hack. Sprite w and h should be n-1 for some reason.
        photo->sprite->height--;
        photo->sprite->width--;
        sd_sprite_save(w, photo->sprite);
        photo->sprite->height++;
        photo->sprite->width++;
    }

    if(sd_writer_errno(w)) {
        goto error;
    }

    sd_writer_close(w);
    return SD_SUCCESS;

error:
    path_unlink(filename);
    sd_writer_close(w);
    return SD_FILE_WRITE_ERROR;
}

const sd_pic_photo *sd_pic_get(const sd_pic_file *pic, int entry_id) {
    if(entry_id < 0 || entry_id >= (int)vector_size(&pic->photos)) {
        return NULL;
    }
    return vector_get(&pic->photos, entry_id);
}

void sd_pic_free(sd_pic_file *pic) {
    vector_free(&pic->photos);
}
