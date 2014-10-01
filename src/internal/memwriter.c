#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

#include "shadowdive/internal/writer.h"
#include "shadowdive/internal/memwriter.h"

#define GROW 64

#define CHECK_SIZE \
    if(writer->real_len < writer->data_len + len) { \
        size_t newsize = writer->real_len + len + GROW; \
        writer->buf = realloc(writer->buf, newsize); \
        writer->real_len = newsize; \
    }

sd_mwriter* sd_mwriter_open() {
    sd_mwriter *mwriter = malloc(sizeof(sd_mwriter));
    memset(mwriter, 0, sizeof(sd_mwriter));
    mwriter->buf = malloc(GROW);
    mwriter->real_len = GROW;
    return mwriter;
}

void sd_mwriter_save(const sd_mwriter *src, sd_writer *dst) {
    sd_write_buf(dst, src->buf, src->data_len);
}

void sd_mwriter_close(sd_mwriter *writer) {
    if(writer == NULL) return;
    free(writer->buf);
    free(writer);
}

long sd_mwriter_pos(const sd_mwriter *writer) {
    return writer->data_len;
}

void sd_mwriter_xor(sd_mwriter *writer, uint8_t key) {
    for(long k = 0; k < writer->data_len; k++) {
        writer->buf[k] = key++ ^ writer->buf[k];
    }
}

void sd_mwrite_buf(sd_mwriter *writer, const char *buf, int len) {
    CHECK_SIZE
    memcpy(writer->buf + writer->data_len, buf, len);
    writer->data_len += len;
}

void sd_mwrite_ubyte(sd_mwriter *writer, uint8_t value) {
    sd_mwrite_buf(writer, (char*)&value, 1);
}

void sd_mwrite_uword(sd_mwriter *writer, uint16_t value) {
    sd_mwrite_buf(writer, (char*)&value, 2);
}

void sd_mwrite_udword(sd_mwriter *writer, uint32_t value) {
    sd_mwrite_buf(writer, (char*)&value, 4);
}

void sd_mwrite_byte(sd_mwriter *writer, int8_t value) {
    sd_mwrite_buf(writer, (char*)&value, 1);
}

void sd_mwrite_word(sd_mwriter *writer, int16_t value) {
    sd_mwrite_buf(writer, (char*)&value, 2);
}

void sd_mwrite_dword(sd_mwriter *writer, int32_t value) {
    sd_mwrite_buf(writer, (char*)&value, 4);
}

void sd_mwrite_float(sd_mwriter *writer, float value) {
    sd_mwrite_buf(writer, (char*)&value, sizeof(float));
}

void sd_mwrite_fill(sd_mwriter *writer, char content, int len) {
    CHECK_SIZE
    memset(writer->buf + writer->data_len, content, len);
    writer->data_len += len;
}
