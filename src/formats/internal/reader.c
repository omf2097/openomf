#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "formats/internal/reader.h"
#include "utils/allocator.h"

struct sd_reader {
    FILE *handle;
    long filesize;
    int sd_errno;
};

sd_reader *sd_reader_open(const char *file) {
    sd_reader *reader = omf_calloc(1, sizeof(sd_reader));

    reader->sd_errno = 0;

    // Attempt to open file (note: Binary mode!)
    reader->handle = fopen(file, "rb");
    if (!reader->handle) {
        omf_free(reader);
        return 0;
    }

    // Find file size
    if (fseek(reader->handle, 0, SEEK_END) == -1) {
        goto error;
    }
    reader->filesize = ftell(reader->handle);
    if (reader->filesize == -1) {
        goto error;
    }
    if (fseek(reader->handle, 0, SEEK_SET) == -1) {
        goto error;
    }

    // All done.
    return reader;

error:
    fclose(reader->handle);
    omf_free(reader);
    return 0;
}

long sd_reader_filesize(const sd_reader *reader) { return reader->filesize; }

int sd_reader_errno(const sd_reader *reader) { return reader->sd_errno; }

void sd_reader_close(sd_reader *reader) {
    fclose(reader->handle);
    omf_free(reader);
}

int sd_reader_set(sd_reader *reader, long offset) {
    if (fseek(reader->handle, offset, SEEK_SET) != 0) {
        reader->sd_errno = errno;
        return 0;
    }
    return 1;
}

int sd_reader_ok(const sd_reader *reader) {
    if (feof(reader->handle)) {
        return 0;
    }
    return 1;
}

long sd_reader_pos(sd_reader *reader) {
    long res = ftell(reader->handle);
    if (res == -1) {
        reader->sd_errno = errno;
    }
    return res;
}

int sd_read_buf(sd_reader *reader, char *buf, int len) {
    if (fread(buf, 1, len, reader->handle) != len) {
        reader->sd_errno = ferror(reader->handle);
        return 0;
    }
    return 1;
}

int sd_peek_buf(sd_reader *reader, char *buf, int len) {
    if (sd_read_buf(reader, buf, len)) {
        return 0;
    }
    if (fseek(reader->handle, ftell(reader->handle) - len, SEEK_SET) == -1) {
        reader->sd_errno = errno;
    }
    return 1;
}

uint8_t sd_read_ubyte(sd_reader *reader) {
    uint8_t d = 0;
    sd_read_buf(reader, (char *)&d, 1);
    return d;
}

uint16_t sd_read_uword(sd_reader *reader) {
    uint16_t d = 0;
    sd_read_buf(reader, (char *)&d, 2);
    return d;
}

uint32_t sd_read_udword(sd_reader *reader) {
    uint32_t d = 0;
    sd_read_buf(reader, (char *)&d, 4);
    return d;
}

int8_t sd_read_byte(sd_reader *reader) {
    int8_t d = 0;
    sd_read_buf(reader, (char *)&d, 1);
    return d;
}

int16_t sd_read_word(sd_reader *reader) {
    int16_t d = 0;
    sd_read_buf(reader, (char *)&d, 2);
    return d;
}

int32_t sd_read_dword(sd_reader *reader) {
    int32_t d = 0;
    sd_read_buf(reader, (char *)&d, 4);
    return d;
}

float sd_read_float(sd_reader *reader) {
    float f = 0;
    sd_read_buf(reader, (char *)&f, 4);
    return f;
}

uint8_t sd_peek_ubyte(sd_reader *reader) {
    uint8_t d = 0;
    sd_peek_buf(reader, (char *)&d, 1);
    return d;
}

uint16_t sd_peek_uword(sd_reader *reader) {
    uint16_t d = 0;
    sd_peek_buf(reader, (char *)&d, 2);
    return d;
}

uint32_t sd_peek_udword(sd_reader *reader) {
    uint32_t d = 0;
    sd_peek_buf(reader, (char *)&d, 4);
    return d;
}

int8_t sd_peek_byte(sd_reader *reader) {
    int8_t d = 0;
    sd_peek_buf(reader, (char *)&d, 1);
    return d;
}

int16_t sd_peek_word(sd_reader *reader) {
    int16_t d = 0;
    sd_peek_buf(reader, (char *)&d, 2);
    return d;
}

int32_t sd_peek_dword(sd_reader *reader) {
    int32_t d = 0;
    sd_peek_buf(reader, (char *)&d, 4);
    return d;
}

float sd_peek_float(sd_reader *reader) {
    float f = 0;
    sd_peek_buf(reader, (char *)&f, 4);
    return f;
}

int sd_match(sd_reader *reader, const char *buf, unsigned int nbytes) {
    char t[nbytes];
    if (sd_peek_buf(reader, t, nbytes) == 0 && memcmp(t, buf, nbytes) == 0) {
        return 1;
    }
    return 0;
}

void sd_skip(sd_reader *reader, unsigned int nbytes) {
    if (fseek(reader->handle, nbytes, SEEK_CUR) == -1) {
        reader->sd_errno = errno;
    }
}

int sd_read_scan(const sd_reader *reader, const char *format, ...) {
    va_list argp;
    va_start(argp, format);
    int ret = fscanf(reader->handle, format, argp);
    va_end(argp);
    return ret;
}

int sd_read_line(const sd_reader *reader, char *buffer, int maxlen) {
    if (fgets(buffer, maxlen, reader->handle) == NULL) {
        return 1;
    }
    return 0;
}

char *sd_read_variable_str(sd_reader *r) {
    uint16_t len = sd_read_uword(r);
    char *str = NULL;
    if (len > 0) {
        str = (char *)omf_calloc(len, 1);
        sd_read_buf(r, str, len);
    }
    return str;
}

void sd_read_str(sd_reader *r, str *dst) {
    uint16_t len = sd_read_uword(r);
    if (len > 0) {
        char *buf = omf_calloc(1, len + 1);
        sd_read_buf(r, buf, len);
        str_from_c(dst, buf);
        omf_free(buf);
    } else {
        str_create(dst);
    }
}
