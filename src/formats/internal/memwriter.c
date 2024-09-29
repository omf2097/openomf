#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "formats/internal/memwriter.h"
#include "formats/internal/writer.h"
#include "utils/allocator.h"

#define GROW 64

#define CHECK_SIZE                                                                                                     \
    if(writer->real_len < writer->data_len + len) {                                                                    \
        size_t newsize = writer->real_len + len + GROW;                                                                \
        writer->buf = omf_realloc(writer->buf, newsize);                                                               \
        writer->real_len = newsize;                                                                                    \
    }

memwriter *memwriter_open(void) {
    memwriter *mwriter = omf_calloc(1, sizeof(memwriter));
    mwriter->buf = omf_calloc(GROW, 1);
    mwriter->real_len = GROW;
    return mwriter;
}

void memwriter_save(const memwriter *src, sd_writer *dst) {
    sd_write_buf(dst, src->buf, src->data_len);
}

void memwriter_close(memwriter *writer) {
    if(writer == NULL) {
        return;
    }
    omf_free(writer->buf);
    omf_free(writer);
}

long memwriter_pos(const memwriter *writer) {
    return writer->data_len;
}

void memwriter_xor(memwriter *writer, uint8_t key) {
    for(long k = 0; k < writer->data_len; k++) {
        writer->buf[k] = key++ ^ writer->buf[k];
    }
}

void memwrite_buf(memwriter *writer, const char *buf, int len) {
    CHECK_SIZE
    memcpy(writer->buf + writer->data_len, buf, len);
    writer->data_len += len;
}

void memwrite_ubyte(memwriter *writer, uint8_t value) {
    memwrite_buf(writer, (char *)&value, 1);
}

void memwrite_uword(memwriter *writer, uint16_t value) {
    memwrite_buf(writer, (char *)&value, 2);
}

void memwrite_udword(memwriter *writer, uint32_t value) {
    memwrite_buf(writer, (char *)&value, 4);
}

void memwrite_byte(memwriter *writer, int8_t value) {
    memwrite_buf(writer, (char *)&value, 1);
}

void memwrite_word(memwriter *writer, int16_t value) {
    memwrite_buf(writer, (char *)&value, 2);
}

void memwrite_dword(memwriter *writer, int32_t value) {
    memwrite_buf(writer, (char *)&value, 4);
}

void memwrite_float(memwriter *writer, float value) {
    memwrite_buf(writer, (char *)&value, sizeof(float));
}

void memwrite_fill(memwriter *writer, char content, int len) {
    CHECK_SIZE
    memset(writer->buf + writer->data_len, content, len);
    writer->data_len += len;
}
