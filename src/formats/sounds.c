#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "formats/internal/reader.h"
#include "formats/internal/writer.h"
#include "formats/error.h"
#include "formats/sounds.h"
#include "utils/allocator.h"

int sd_sounds_create(sd_sound_file *sf) {
    if(sf == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(sf, 0, sizeof(sd_sound_file));
    return SD_SUCCESS;
}

int sd_sounds_load(sd_sound_file *sf, const char *filename) {
    if(sf == NULL || filename == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    // Read header
    uint32_t first_udword = sd_read_udword(r);
    if(first_udword != 0) {
        sd_reader_close(r);
        return SD_FILE_INVALID_TYPE;
    }

    uint32_t header_size = sd_read_udword(r);
    int data_block_count = header_size / 4 - 2;

    // Find block sizes
    for(int i = 0; i < data_block_count; i++) {
        sd_read_udword(r);
    }

    // Read blocks
    for(int i = 0; i <= data_block_count; i++) {
        sf->sounds[i].len = sd_read_uword(r);
        if(sf->sounds[i].len > 0) {
            sf->sounds[i].unknown = sd_read_ubyte(r);
            sf->sounds[i].data = omf_calloc(sf->sounds[i].len, 1);
            sd_read_buf(r, sf->sounds[i].data, sf->sounds[i].len);
        }
    }

    sd_reader_close(r);
    return SD_SUCCESS;
}

int sd_sounds_save(const sd_sound_file *sf, const char* filename) {
    if(sf == NULL || filename == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_writer *w = sd_writer_open(filename);
    if(!w) {
        return SD_FILE_OPEN_ERROR;
    }

    // Write header. Zero start + data block start.
    size_t s_offset = 4 * SD_SOUNDS_MAX + 4;
    sd_write_udword(w, 0);

    // First pass -- write offsets
    size_t ptr = 0;
    for(int i = 0; i < SD_SOUNDS_MAX; i++) {
        sd_write_udword(w, ptr + s_offset);
        ptr += sf->sounds[i].len;
    }

    // Second pass -- Write sounds
    for(int i = 0; i < SD_SOUNDS_MAX; i++) {
        sd_write_uword(w, sf->sounds[i].len);
        if(sf->sounds[i].len > 0) {
            sd_write_ubyte(w, sf->sounds[i].unknown);
            sd_write_buf(w, sf->sounds[i].data, sf->sounds[i].len);
        }
    }

    sd_writer_close(w);
    return SD_SUCCESS;
}

const sd_sound* sd_sounds_get(const sd_sound_file *sf, int id) {
    if(id < 0 || id >= SD_SOUNDS_MAX) {
        return NULL;
    }
    return &sf->sounds[id];
}

int sd_sound_from_au(sd_sound_file *sf, int num, const char *filename) {
    int ret = SD_SUCCESS;
    if(sf == NULL || filename == NULL || num < 0 || num >= 299) {
        return SD_INVALID_INPUT;
    }

    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    // Make sure the file seems right
    uint32_t magic_number = sd_read_udword(r);
    if(magic_number != 0x2e736e64) {
        ret = SD_FILE_INVALID_TYPE;
        goto error_0;
    }

    // Header data
    uint32_t data_start = sd_read_udword(r);
    uint32_t data_size = sd_read_udword(r);
    uint32_t data_type = sd_read_udword(r);
    uint32_t data_freq = sd_read_udword(r);
    uint32_t data_channels = sd_read_udword(r);

    // Check data format
    if(data_type != 2 || data_freq != 8000 || data_channels != 1) {
        ret = SD_FILE_INVALID_TYPE;
        goto error_0;
    }

    // Skip annotation field and jump to data start
    if(sd_reader_set(r, data_start) != 0) {
        ret = SD_FILE_INVALID_TYPE;
        goto error_0;
    }

    // Size to read
    size_t read_size = 0;
    if(data_size != 0xffffffff) {
        read_size = data_size;
    } else {
        read_size = sd_reader_filesize(r) - sd_reader_pos(r);
    }

    // Free if exists.
    if(sf->sounds[num].data) {
        omf_free(sf->sounds[num].data);
    }

    // Allocate
    sf->sounds[num].len = read_size;
    sf->sounds[num].data = omf_calloc(read_size, 1);

    // Read data
    for(size_t i = 0; i < read_size; i++) {
        sf->sounds[num].data[i] = sd_read_byte(r) + 128;
    }

error_0:
    sd_reader_close(r);
    return ret;
}

int sd_sound_to_au(const sd_sound_file *sf, int num, const char *filename) {
    if(sf == NULL || filename == NULL || num < 0 || num >= 299) {
        return SD_INVALID_INPUT;
    }

    sd_writer *w = sd_writer_open(filename);
    if(!w) {
        return SD_FILE_OPEN_ERROR;
    }

    // Write AU header
    sd_write_udword(w, 0x2e736e64); // Magic number (".snd")
    sd_write_udword(w, 32); // Data start
    sd_write_udword(w, 0xffffffff); // Data size
    sd_write_udword(w, 2); // Type (8bit signed pcm), needs conversion from our unsigned
    sd_write_udword(w, 8000); // Freq
    sd_write_udword(w, 1); // Channels

    // Annotation field (terminate with 0)
    sd_write_buf(w, "omf2097", 7);
    sd_write_ubyte(w, 0);

    // Write data
    int8_t sample = 0;
    for(int i = 0; i < sf->sounds[num].len; i++) {
        sample = sf->sounds[num].data[i] - 128;
        sd_write_byte(w, sample);
    }

    // All done!
    sd_writer_close(w);
    return SD_SUCCESS;
}

void sd_sounds_free(sd_sound_file *sf) {
    if(sf == NULL) return;
    for(int i = 0; i < SD_SOUNDS_MAX; i++) {
        omf_free(sf->sounds[i].data);
    }
}
