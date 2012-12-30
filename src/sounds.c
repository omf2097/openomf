#include "sounds.h"
#include "internal/reader.h"
#include "internal/writer.h"
#include <stdlib.h>

sd_sound_file* sd_af_load(const char *filename) {
    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return 0;
    }

    // Read header
    sd_skip(r, 4);
    uint32_t header_size = sd_read_udword(r);
    int data_block_count = header_size / 4;

    // Find block sizes
    uint32_t data_block_offsets[data_block_count+1];
    for(int i = 0; i < data_block_count - 2; i++) {
        data_block_offsets[i] = sd_read_udword(r);
    }
    data_block_offsets[data_block_count] = sd_reader_filesize(r);

    // Allocate structures
    sd_sound_file *sf = (sd_sound_file*)malloc(sizeof(sd_sound_file));
    sf->sounds = malloc(sizeof(sd_sound*) * data_block_count);
    sf->sound_count = data_block_count;

    // Read blocks
    sd_sound *sound;
    unsigned int offset = 0;
    for(int i = 0; i < data_block_count; i++) {
        offset = data_block_offsets[i];

        if((offset - sd_reader_pos(r)) <= 2) {
            sf->sounds[i] = 0;
        } else {
            sound = (sd_sound*)malloc(sizeof(sd_sound));
            sound->len = offset - sd_reader_pos(r);
            sound->data = (char*)malloc(sound->len);
            sd_read_buf(r, sound->data, sound->len);
            sf->sounds[i] = sound;
        }
    }

    sd_reader_close(r);
    return sf;
}

int sd_af_save(const char* filename, sd_sound_file *sf) {
    sd_writer *w = sd_writer_open(filename);
    if(!w) {
        return 0;
    }

    // TODO: Writer

    sd_writer_close(w);
    return 1;
}

void sd_af_delete(sd_sound_file *sf) {
    for(int i = 0; i < sf->sound_count; i++) {
        if(sf->sounds[i]) {
            free(sf->sounds[i]);
        }
    }
    free(sf->sounds);
}
