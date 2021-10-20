#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <errno.h>

#include "formats/internal/writer.h"
#include "utils/allocator.h"

struct sd_writer_t {
    FILE *handle;
    int sd_errno;
};

sd_writer* sd_writer_open(const char *file) {
    sd_writer *writer = omf_calloc(1, sizeof(sd_writer));

    writer->handle = fopen(file, "wb");
    writer->sd_errno = 0;
    if(!writer->handle) {
        free(writer);
        return 0;
    }

    return writer;
}

int sd_writer_errno(const sd_writer *writer) {
    return writer->sd_errno;
}

void sd_writer_close(sd_writer *writer) {
    fclose(writer->handle);
    free(writer);
}

long sd_writer_pos(sd_writer *writer) {
    long res = ftell(writer->handle);
    if (res == -1) {
        writer->sd_errno = errno;
    }
    return res;
}

int sd_writer_seek_start(const sd_writer *writer, long offset) {
    return fseek(writer->handle, offset, SEEK_SET);
}

int sd_writer_seek_cur(const sd_writer *writer, long offset) {
    return fseek(writer->handle, offset, SEEK_CUR);
}

int sd_writer_seek_end(const sd_writer *writer, long offset) {
    return fseek(writer->handle, offset, SEEK_END);
}

int sd_write_buf(sd_writer *writer, const char *buf, int len) {
    if(fwrite(buf, 1, len, writer->handle) != len) {
        writer->sd_errno = ferror(writer->handle);
        return 0;
    }
    return 1;
}

int sd_write_fprintf(const sd_writer *writer, const char *format, ...) {
    va_list argp;
    va_start(argp, format);
    int ret = vfprintf(writer->handle, format, argp);
    va_end(argp);
    return ret;
}

void sd_write_ubyte(sd_writer *writer, uint8_t data) {
    sd_write_buf(writer, (char*)&data, 1);
}

void sd_write_uword(sd_writer *writer, uint16_t data) {
    sd_write_buf(writer, (char*)&data, 2);
}

void sd_write_udword(sd_writer *writer, uint32_t data) {
    sd_write_buf(writer, (char*)&data, 4);
}

void sd_write_byte(sd_writer *writer, int8_t data) {
    sd_write_buf(writer, (char*)&data, 1);
}

void sd_write_word(sd_writer *writer, int16_t data) {
    sd_write_buf(writer, (char*)&data, 2);
}

void sd_write_dword(sd_writer *writer, int32_t data) {
    sd_write_buf(writer, (char*)&data, 4);
}

void sd_write_float(sd_writer *writer, float data) {
    sd_write_buf(writer, (char*)&data, sizeof(float));
}

void sd_write_fill(sd_writer *writer, char content, int len) {
    int left = len;
    int now = 0;
    char buffer[1024];

    memset(buffer, content, 1024);
    while(left > 0) {
        now = (left > 1024) ? 1024 : left;
        if (fwrite(buffer, 1, now, writer->handle) != now) {
            writer->sd_errno = ferror(writer->handle);
            return;
        }
        left -= now;
    }
}

void sd_write_variable_str(sd_writer *w, const char *str) {
    if(str == NULL) {
        sd_write_uword(w, 0);
        return;
    }
    uint16_t len = strlen(str) + 1;
    sd_write_uword(w, len);
    sd_write_buf(w, str, len);
}
