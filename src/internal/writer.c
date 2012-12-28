#include "internal/writer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct sd_writer_t {
    FILE *handle;
    char err[64];
} sd_writer;

sd_writer* sd_writer_open(const char *file) {
    sd_writer *writer = malloc(sizeof(sd_writer));

    writer->handle = fopen(file, "w");
    if(!writer->handle) {
        free(writer);
        return 0;
    }

    return writer;
}

void sd_writer_close(sd_writer *writer) {
    fclose(writer->handle);
    free(writer);
}

long sd_writer_pos(sd_writer *writer) {
    return ftell(writer->handle);
}

int sd_write_buf(sd_writer *writer, char *buf, int len) {
    if(fwrite(buf, 1, len, writer->handle) != len) {
        strcpy(writer->err, "Error: Wrote less than requested amount of bytes.");
        return 0;
    }
    return 1;
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

