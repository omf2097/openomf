#include "utils/blob.h"
#include "allocator.h"

static void blob_init(blob *b, const size_t size) {
    b->data = omf_calloc(1, size);
    b->size = size;
}

void blob_resize(blob *b, const size_t size) {
    if(b->size != size) {
        b->data = omf_realloc(b->data, size);
        b->size = size;
    }
}

blob *blob_create(const size_t size) {
    blob *b = omf_malloc(sizeof(blob));
    blob_init(b, size);
    return b;
}

void blob_set(blob *b, const char *src, const size_t len) {
    blob_resize(b, len);
    memcpy(b->data, src, len);
}

void blob_free(blob *b) {
    free(b->data);
    free(b);
}
