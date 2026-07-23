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

void sd_lang_string_free(void *ptr) {
    sd_lang_string *entry = ptr;
    str_free(&entry->description);
    str_free(&entry->data);
}

void sd_language_create(sd_language *language) {
    assert(language != NULL);
    memset(language, 0, sizeof(sd_language));
    vector_create_cb(&language->strings, sizeof(sd_lang_string), sd_lang_string_free);
}

void sd_language_free(sd_language *language) {
    if(language == NULL) {
        return;
    }
    vector_free(&language->strings);
}

int sd_language_load(sd_language *language, const path *filename, bool load_descriptions) {
    assert(language != NULL);
    assert(filename != NULL);

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
    vector_reserve(&language->strings, string_count);

    // Read titles and offsets
    unsigned int pos = 0;
    while((offset = sd_read_udword(r)) < file_size && pos < string_count) {
        sd_lang_string *entry = vector_append_ptr(&language->strings);
        if(load_descriptions) {
            sd_read_fixed_str(r, &entry->description, 32);
        } else {
            sd_skip(r, 32); // skip over the description field, leave it empty
            str_create(&entry->description);
        }
        str_create(&entry->data); // filled in below
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
        const unsigned int len = offsets[i + 1] - offsets[i];
        if(len == 0) {
            continue;
        }

        // Read XOR-encoded string into a buffer, then into the str.
        char *buf = omf_calloc(len + 1, 1);
        memreader *mr = memreader_open_from_reader(r, len);
        memreader_xor(mr, len & 0xFF);
        memread_buf(mr, buf, len);
        memreader_close(mr);
        sd_lang_string *entry = vector_get(&language->strings, i);
        str_set_c(&entry->data, buf);
        omf_free(buf);
    }

    // All done.
    omf_free(offsets);
    sd_reader_close(r);
    return SD_SUCCESS;
}

const sd_lang_string *sd_language_get(const sd_language *language, unsigned num) {
    if(language == NULL) {
        return NULL;
    }
    return vector_get(&language->strings, num);
}

void sd_language_append(sd_language *language, const char *description, const char *data) {
    assert(strlen(description) < 32);
    sd_lang_string *entry = vector_append_ptr(&language->strings);
    str_from_c(&entry->description, description);
    str_from_c(&entry->data, data);
}

int sd_language_save(const sd_language *language, const path *filename) {
    assert(language != NULL);
    assert(filename != NULL);

    sd_writer *w = sd_writer_open(filename);
    if(!w) {
        return SD_FILE_OPEN_ERROR;
    }

    // Write descriptors
    for(unsigned i = 0; i < vector_size(&language->strings); i++) {
        const sd_lang_string *entry = vector_get(&language->strings, i);
        sd_write_dword(w, 0); // For now
        sd_write_fixed_str(w, &entry->description, 32);
    }

    // Write strings
    for(unsigned i = 0; i < vector_size(&language->strings); i++) {
        // Write catalog offset
        const long offset = sd_writer_pos(w);
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
        const sd_lang_string *entry = vector_get(&language->strings, i);
        memwriter *mw = memwriter_open();
        const size_t str_len = str_size(&entry->data);
        memwrite_buf(mw, str_c(&entry->data), str_len);
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
