#include "internal/writer.h"

#include <stdlib.h>
#include <stdio.h>

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
