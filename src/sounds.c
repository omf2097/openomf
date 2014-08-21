#include <stdlib.h>
#include <string.h>

#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/error.h"
#include "shadowdive/sounds.h"

#include <stdio.h>

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
        return SD_FILE_INVALID_TYPE;
    }

    uint32_t header_size = sd_read_udword(r);
    int data_block_count = header_size / 4 - 2;

    // Find block sizes
    uint32_t data_block_offsets[data_block_count+1];
    for(int i = 0; i < data_block_count; i++) {
        data_block_offsets[i] = sd_read_udword(r);
        printf("%d = %d\n", i, data_block_offsets[i]);
    }
    data_block_offsets[data_block_count] = sd_reader_filesize(r);

    // Read blocks
    for(int i = 0; i <= data_block_count; i++) {
        sf->sounds[i].len = data_block_offsets[i] - sd_reader_pos(r);
        sf->sounds[i].data = malloc(sf->sounds[i].len);
        sd_read_buf(r, sf->sounds[i].data, sf->sounds[i].len);
    }

    sd_reader_close(r);
    return SD_SUCCESS;
}

int sd_sounds_save(sd_sound_file *sf, const char* filename) {
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
        sd_write_buf(w, sf->sounds[i].data, sf->sounds[i].len);
    }

    sd_writer_close(w);
    return SD_SUCCESS;
}

sd_sound* sd_sounds_get(sd_sound_file *sf, int id) {
    if(id < 0 || id >= SD_SOUNDS_MAX) {
        return NULL;
    }
    return &sf->sounds[id];
}

int sd_sound_from_au(sd_sound *sound, const char *filename) {
    if(sound == NULL || filename == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    // Make sure the file seems right
    uint32_t magic_number = sd_read_udword(r);
    if(magic_number != 0x2e736e64) {
        return SD_FILE_INVALID_TYPE;
    }

    // Header data
    uint32_t data_start = sd_read_udword(r);
    uint32_t data_size = sd_read_udword(r);
    uint32_t data_type = sd_read_udword(r);
    uint32_t data_freq = sd_read_udword(r);
    uint32_t data_channels = sd_read_udword(r);

    // Check data format
    if(data_type != 2 || data_freq != 8000 || data_channels != 1) {
        return SD_FILE_INVALID_TYPE;
    }

    // Skip annotation field and jump to data start
    sd_reader_set(r, data_start);

    // Size to read
    size_t read_size = 0;
    if(data_size != 0xffffffff) {
        read_size = data_size;
    } else {
        read_size = sd_reader_filesize(r) - sd_reader_pos(r);
    }

    // Allocate
    sound->len = read_size;
    sound->data = malloc(read_size);

    // Read data
    for(size_t i = 0; i < read_size; i++) {
        sound->data[i] = sd_read_byte(r) + 128;
    }

    sd_reader_close(r);
    return SD_SUCCESS;
}

int sd_sound_to_au(sd_sound *sound, const char *filename) {
    if(sound == NULL || filename == NULL) {
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
    for(int i = 0; i < sound->len; i++) {
        sample = sound->data[i] - 128;
        sd_write_byte(w, sample);
    }

    // All done!
    sd_writer_close(w);
    return SD_SUCCESS;
}

void sd_sounds_free(sd_sound_file *sf) {
    if(sf == NULL) return;
    for(int i = 0; i < SD_SOUNDS_MAX; i++) {
        free(sf->sounds[i].data);
    }
}
