#include "sounds.h"
#include "internal/reader.h"
#include "internal/writer.h"
#include <stdlib.h>

sd_sound_file* sd_sounds_load(const char *filename) {
    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return 0;
    }

    // Read header
    sd_skip(r, 4);
    uint32_t header_size = sd_read_udword(r);
    int data_block_count = header_size / 4 - 2;

    // Find block sizes
    uint32_t data_block_offsets[data_block_count];
    for(int i = 0; i < data_block_count; i++) {
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
            sd_skip(r, 2);
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

int sd_sounds_save(const char* filename, sd_sound_file *sf) {
    sd_writer *w = sd_writer_open(filename);
    if(!w) {
        return 0;
    }

    // TODO: Writer

    sd_writer_close(w);
    return 1;
}

void sd_sound_to_au(sd_sound *sound, const char* file) {
    sd_writer *w = sd_writer_open(file);
    if(!w) return;

    // Write AU header
    sd_write_udword(w, 0x2e736e64); // Magic number (".snd")
    sd_write_udword(w, 32); // Data start
    sd_write_udword(w, 0xffffffff); // Data size
    sd_write_udword(w, 2); // Type (8bit signed pcm)
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
}

void sd_sounds_delete(sd_sound_file *sf) {
    for(int i = 0; i < sf->sound_count; i++) {
        if(sf->sounds[i]) {
            free(sf->sounds[i]);
        }
    }
    free(sf->sounds);
}
