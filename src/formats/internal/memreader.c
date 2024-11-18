#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "formats/internal/memreader.h"
#include "formats/internal/reader.h"
#include "utils/allocator.h"

memreader *memreader_open(char *buf, long len) {
    memreader *reader = omf_calloc(1, sizeof(memreader));
    reader->buf = buf;
    reader->pos = 0;
    reader->len = len;
    reader->owned = 0;
    return reader;
}

memreader *memreader_open_from_reader(sd_reader *reader, int len) {
    char *buf = omf_calloc(1, len);
    sd_read_buf(reader, buf, len);
    memreader *mreader = memreader_open(buf, len);
    mreader->owned = 1;
    return mreader;
}

void memreader_xor(memreader *reader, uint8_t key) {
    for(long k = 0; k < reader->len; k++) {
        reader->buf[k] = key++ ^ reader->buf[k];
    }
}

long memreader_size(const memreader *reader) {
    return reader->len;
}

long memreader_pos(const memreader *reader) {
    return reader->pos;
}

void memreader_close(memreader *reader) {
    if(reader->owned) {
        omf_free(reader->buf);
    }
    omf_free(reader);
}

int memread_buf(memreader *reader, char *buf, int len) {
    if(reader->pos + len > reader->len) {
        return 0;
    }
    memcpy(buf, reader->buf + reader->pos, len);
    reader->pos += len;
    return 1;
}

uint8_t memread_ubyte(memreader *reader) {
    uint8_t r;
    memcpy(&r, reader->buf + reader->pos, sizeof(r));
    reader->pos += sizeof(r);
    return r;
}

uint16_t memread_uword(memreader *reader) {
    uint16_t r;
#ifdef BIG_ENDIAN_BUILD
    r = __builtin_bswap16(*((uint16_t *)(reader->buf + reader->pos)));
#else
    memcpy(&r, reader->buf + reader->pos, sizeof(r));
#endif
    reader->pos += sizeof(r);
    return r;
}

uint32_t memread_udword(memreader *reader) {
    uint32_t r;
#ifdef BIG_ENDIAN_BUILD
    r = __builtin_bswap32(*((uint32_t *)(reader->buf + reader->pos)));
#else
    memcpy(&r, reader->buf + reader->pos, sizeof(r));
#endif
    reader->pos += sizeof(r);
    return r;
}

int8_t memread_byte(memreader *reader) {
    int8_t r;
    memcpy(&r, reader->buf + reader->pos, sizeof(r));
    reader->pos += sizeof(r);
    return r;
}

int16_t memread_word(memreader *reader) {
    int16_t r;
#ifdef BIG_ENDIAN_BUILD
    r = __builtin_bswap16(*((uint16_t *)(reader->buf + reader->pos)));
#else
    memcpy(&r, reader->buf + reader->pos, sizeof(r));
#endif
    reader->pos += sizeof(r);
    return r;
}

int32_t memread_dword(memreader *reader) {
    int32_t r;
#ifdef BIG_ENDIAN_BUILD
    r = __builtin_bswap32(*((int32_t *)(reader->buf + reader->pos)));
#else
    memcpy(&r, reader->buf + reader->pos, sizeof(r));
#endif
    reader->pos += sizeof(r);
    return r;
}

float memread_float(memreader *reader) {
    float r;
#ifdef BIG_ENDIAN_BUILD
    uint32_t fl;
    memcpy(&fl, reader->buf + reader->pos, sizeof(fl));
    fl = __builtin_bswap32(fl);
    reader->pos += sizeof(fl);
#else
    memcpy(&r, reader->buf + reader->pos, sizeof(r));
    reader->pos += sizeof(r);
#endif
    return r;
}

void sd_mskip(memreader *reader, unsigned int nbytes) {
    reader->pos += nbytes;
}
