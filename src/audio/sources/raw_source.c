#include "audio/sources/raw_source.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include <stdlib.h>
#include <string.h>

typedef struct raw_source_t {
    char *buf;
    int len;
    int pos;
} raw_source;

int raw_source_update(audio_source *src, char *buffer, int len) {
    raw_source *local = source_get_userdata(src);
    int data_left = local->len - local->pos;
    if (data_left == 0) {
        return 0;
    }
    int real_len = (len > data_left) ? data_left : len;
    memcpy(buffer, local->buf + local->pos, real_len);
    local->pos += real_len;
    return real_len;
}

void raw_source_close(audio_source *src) {
    raw_source *local = source_get_userdata(src);
    omf_free(local);
    source_set_userdata(src, local);
}

int raw_source_init(audio_source *src, char *buffer, int len) {
    raw_source *local = omf_calloc(1, sizeof(raw_source));

    // Set data
    local->pos = 0;
    local->len = len;
    local->buf = buffer;

    // Audio information
    source_set_frequency(src, 8000);
    source_set_bytes(src, 1);
    source_set_channels(src, 1);

    // Set callbacks
    source_set_userdata(src, local);
    source_set_update_cb(src, raw_source_update);
    source_set_close_cb(src, raw_source_close);

    // All done
    return 0;
}
