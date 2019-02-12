#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "formats/internal/reader.h"
#include "formats/internal/memreader.h"

sd_mreader* sd_mreader_open(char *buf, long len) {
    sd_mreader *reader = malloc(sizeof(sd_mreader));
    reader->buf = buf;
    reader->pos = 0;
    reader->len = len;
    reader->owned = 0;
    return reader;
}

sd_mreader* sd_mreader_open_from_reader(sd_reader *reader, int len) {
    char *buf = malloc(len);
    sd_read_buf(reader, buf, len);
    sd_mreader *mreader = sd_mreader_open(buf, len);
    mreader->owned = 1;
    return mreader;
}

void sd_mreader_xor(sd_mreader *reader, uint8_t key) {
    for(long k = 0; k < reader->len; k++) {
        reader->buf[k] = key++ ^ reader->buf[k];
    }
}

long sd_mreader_size(const sd_mreader *reader) {
    return reader->len;
}

long sd_mreader_pos(const sd_mreader *reader) {
    return reader->pos;
}

void sd_mreader_close(sd_mreader *reader) {
    if(reader->owned) {
        free(reader->buf);
    }
    free(reader);
}

int sd_mread_buf(sd_mreader *reader, char *buf, int len) {
    if(reader->pos + len > reader->len)
        return 0;
    memcpy(buf, reader->buf + reader->pos, len);
    reader->pos += len;
    return 1;
}

uint8_t sd_mread_ubyte(sd_mreader *reader) {
    uint8_t r;
    memcpy(&r, reader->buf + reader->pos, sizeof(r));
    reader->pos += sizeof(r);
    return r;
}

uint16_t sd_mread_uword(sd_mreader *reader) {
    uint16_t r;
    memcpy(&r, reader->buf + reader->pos, sizeof(r));
    reader->pos += sizeof(r);
    return r;
}

uint32_t sd_mread_udword(sd_mreader *reader) {
    uint32_t r;
    memcpy(&r, reader->buf + reader->pos, sizeof(r));
    reader->pos += sizeof(r);
    return r;
}

int8_t sd_mread_byte(sd_mreader *reader) {
    int8_t r;
    memcpy(&r, reader->buf + reader->pos, sizeof(r));
    reader->pos += sizeof(r);
    return r;
}

int16_t sd_mread_word(sd_mreader *reader) {
    int16_t r;
    memcpy(&r, reader->buf + reader->pos, sizeof(r));
    reader->pos += sizeof(r);
    return r;
}

int32_t sd_mread_dword(sd_mreader *reader) {
    int32_t r;
    memcpy(&r, reader->buf + reader->pos, sizeof(r));
    reader->pos += sizeof(r);
    return r;
}

float sd_mread_float(sd_mreader *reader) {
    float r;
    memcpy(&r, reader->buf + reader->pos, sizeof(r));
    reader->pos += sizeof(r);
    return r;
}

void sd_mskip(sd_mreader *reader, unsigned int nbytes) {
    reader->pos += nbytes;
}
