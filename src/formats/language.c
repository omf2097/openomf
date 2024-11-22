#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "formats/error.h"
#include "formats/internal/memreader.h"
#include "formats/internal/memwriter.h"
#include "formats/internal/reader.h"
#include "formats/internal/writer.h"
#include "formats/language.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"

int sd_language_create(sd_language *language) {
    if(language == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(language, 0, sizeof(sd_language));
    return SD_SUCCESS;
}

void sd_language_free(sd_language *language) {
    if(language == NULL)
        return;
    if(language->strings != 0) {
        for(unsigned i = 0; i < language->count; i++) {
            omf_free(language->strings[i].data);
        }
        omf_free(language->strings);
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
    uint32_t file_size = sd_reader_filesize(r);
    while(sd_read_udword(r) < file_size) {
        sd_skip(r, 32);
        string_count++;
    }
    sd_reader_set(r, 0);

    // There should be at least one string
    if(string_count <= 0) {
        sd_reader_close(r);
        return SD_FILE_INVALID_TYPE;
    }

    // Some variables etc.
    unsigned int offset = 0;
    unsigned int *offsets = omf_calloc(string_count + 1, sizeof(unsigned int));
    language->strings = omf_calloc(string_count, sizeof(sd_lang_string));
    language->count = string_count;

    // Read titles and offsets
    unsigned int pos = 0;
    while((offset = sd_read_udword(r)) < file_size && pos < string_count) {
        sd_read_buf(r, language->strings[pos].description, 32);
        language->strings[pos].description[31] = 0;
        offsets[pos] = offset;
        pos++;
    }

    // Valid file with no content
    if(pos <= 0) {
        omf_free(offsets);
        sd_reader_close(r);
        return SD_SUCCESS;
    }

    offsets[pos] = file_size;

    // Read real titles
    for(unsigned i = 0; i < pos; i++) {
        sd_reader_set(r, offsets[i]);
        unsigned int len = offsets[i + 1] - offsets[i];

        language->strings[i].data = omf_calloc(len + 1, 1);
        memset(language->strings[i].data, 0, len + 1);

        // Read string
        memreader *mr = memreader_open_from_reader(r, len);
        memreader_xor(mr, len & 0xFF);
        memread_buf(mr, language->strings[i].data, len);
        memreader_close(mr);
    }

    // All done.
    omf_free(offsets);
    sd_reader_close(r);
    return SD_SUCCESS;
}

const sd_lang_string *sd_language_get(const sd_language *language, unsigned num) {
    if(language == NULL || num >= language->count) {
        return NULL;
    }
    return &language->strings[num];
}

void sd_language_append(sd_language *language, const char *description, const char *data) {
    assert(strlen(description) < 32);
    language->count++;

    language->strings = omf_realloc(language->strings, language->count * sizeof(sd_lang_string));
    strncpy(language->strings[language->count - 1].description, description, 32);
    language->strings[language->count - 1].data = omf_strdup(data);
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
    for(unsigned i = 0; i < language->count; i++) {
        sd_write_dword(w, 0); // For now
        sd_write_buf(w, language->strings[i].description, 32);
    }

    // Write strings
    for(unsigned i = 0; i < language->count; i++) {
        // Write catalog offset
        long offset = sd_writer_pos(w);
        if(offset < 0) {
            goto error;
        }
        if(sd_writer_seek_start(w, 36 * i) < 0) {
            goto error;
        }
        sd_write_udword(w, offset);
        if(sd_writer_seek_start(w, offset) < 0) {
            goto error;
        }

        // write string
        memwriter *mw = memwriter_open();
        size_t str_len = strlen(language->strings[i].data);
        memwrite_buf(mw, language->strings[i].data, str_len);
        memwriter_xor(mw, str_len & 0xFF);
        memwriter_save(mw, w);
        memwriter_close(mw);
    }

    sd_writer_close(w);
    return SD_SUCCESS;

error:
    sd_writer_close(w);
    return SD_FILE_WRITE_ERROR;
}
