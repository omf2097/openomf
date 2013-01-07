#include "shadowdive/language.h"
#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/error.h"
#include <stdlib.h>

sd_language* sd_language_create() {
    sd_language *language = (sd_language*)malloc(sizeof(sd_language));
    language->count = 0;
    language->strings = 0;
    return language;
}

void sd_language_delete(sd_language *language) {
    if(language) {
        if(language->strings != 0) {
            for(int i = 0; i < language->count; i++) {
                free(language->strings[i].data);
            }
        }
        free(language);
    }
}

int sd_language_load(sd_language *language, const char *filename) {
    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }
    
    // Find out how many strings there are in the file
    unsigned int string_count = 0;
    unsigned int offset = 0;
    long file_size = sd_reader_filesize(r);
    while((offset = sd_read_udword(r)) < file_size) {
        sd_skip(r, 32);
        string_count++;
    }
    sd_reader_set(r, 0);
    
    // Some variables etc.
    unsigned int offsets[string_count+1];
    language->strings = malloc(sizeof(sd_lang_string) * string_count);
    language->count = string_count;
    
    // Read titles and offsets
    unsigned int pos = 0;
    while((offset = sd_read_udword(r)) < file_size) {
        sd_read_buf(r, language->strings[pos].description, 32);
        language->strings[pos].description[32] = 0;
        offsets[pos] = offset;
        pos++;
    }
    offsets[pos] = file_size;
    
    // Read real titles
    unsigned int len = 0;
    uint8_t xorkey = 0;
    for(int i = 0; i < string_count; i++) {
        sd_reader_set(r, offsets[i]);
        len = offsets[i+1] - offsets[i];
        xorkey = len & 0xFF;
        language->strings[i].data = malloc(len+1);
        language->strings[i].data[len] = 0;
        
        // Read & XOR
        for(int k = 0; k < len; k++) {
            language->strings[i].data[k] = sd_read_byte(r) ^ xorkey++;
        }
    }

    // All done.
    sd_reader_close(r);
    return SD_SUCCESS;
}

int sd_language_save(sd_language *language, const char *filename) {
    sd_writer *w = sd_writer_open(filename);
    if(!w) {
        return SD_FILE_OPEN_ERROR;
    }

    // TODO: Implement this
    
    sd_writer_close(w);
    return SD_SUCCESS;
}
