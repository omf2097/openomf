#include <stdlib.h>
#include <string.h>

#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/memreader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/internal/memwriter.h"
#include "shadowdive/error.h"
#include "shadowdive/language.h"

int sd_language_create(sd_language *language) {
    if(language == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(language, 0, sizeof(sd_language));
    return SD_SUCCESS;
}

void sd_language_free(sd_language *language) {
    if(language == NULL) return;
    if(language->strings != 0) {
        for(int i = 0; i < language->count; i++) {
            free(language->strings[i].data);
        }
        free(language->strings);
    }
}

int sd_language_load(sd_language *language, const char *filename) {
    if(language == NULL || filename == NULL) {
        return SD_INVALID_INPUT;
    }

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

    if (!string_count) {
        sd_reader_close(r);
        return SD_FILE_INVALID_TYPE;
    }
    
    // Some variables etc.
    unsigned int offsets[string_count+1];
    language->strings = malloc(sizeof(sd_lang_string) * string_count);
    language->count = string_count;
    
    // Read titles and offsets
    unsigned int pos = 0;
    while((offset = sd_read_udword(r)) < file_size) {
        sd_read_buf(r, language->strings[pos].description, 32);
        language->strings[pos].description[31] = 0;
        offsets[pos] = offset;
        pos++;
    }

    offsets[pos] = file_size;
    
    // Read real titles
    for(int i = 0; i < string_count; i++) {
        sd_reader_set(r, offsets[i]);
        unsigned int len = offsets[i+1] - offsets[i];

        language->strings[i].data = malloc(len + 1);
        memset(language->strings[i].data, 0, len + 1);

        // Read string
        sd_mreader *mr = sd_mreader_open_from_reader(r, len);
        sd_mreader_xor(mr, len & 0xFF);
        sd_mread_buf(mr, language->strings[i].data, len);
        sd_mreader_close(mr);
    }

    // All done.
    sd_reader_close(r);
    return SD_SUCCESS;
}

const sd_lang_string* sd_language_get(const sd_language *language, int num) {
    if(language == NULL || num < 0 || num >= language->count) {
        return NULL;
    }
    return &language->strings[num];
}

int sd_language_save(sd_language *language, const char *filename) {
    if(language == NULL || filename == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_writer *w = sd_writer_open(filename);
    if(!w) {
        return SD_FILE_OPEN_ERROR;
    }

    // Write descriptors
    for(int i = 0; i < language->count; i++) {
        sd_write_dword(w, 0); // For now
        sd_write_buf(w, language->strings[i].description, 32);
    }

    // Write strings
    for(int i = 0; i < language->count; i++) {
        // Write catalog offset
        uint32_t offset = sd_writer_pos(w);
        sd_writer_seek_start(w, 36 * i);
        sd_write_udword(w, offset);
        sd_writer_seek_start(w, offset);

        // write string
        sd_mwriter *mw = sd_mwriter_open();
        size_t str_len = strlen(language->strings[i].data);
        sd_mwrite_buf(mw, language->strings[i].data, str_len);
        sd_mwriter_xor(mw, str_len & 0xFF);
        sd_mwriter_save(mw, w);
        sd_mwriter_close(mw);
    }

    
    sd_writer_close(w);
    return SD_SUCCESS;
}
