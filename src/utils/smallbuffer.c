#include "utils/smallbuffer.h"

#include "utils/allocator.h"
#include <limits.h>
#include <stdbool.h>

/*
 * smallbuffer implementation notes!
 *
 * SBO = small buffer optimization.
 *
 * priv_cap is signed. negative values indicate SBO!
 * This is important for smallbuffer_realloc_with_custom_selfsize.
 */

inline static bool is_sb_allocated(smallbuffer *sb) {
    return sb->priv_cap > 0;
}

void smallbuffer_free(smallbuffer *sb) {
    if(is_sb_allocated(sb)) {
        omf_free(sb->priv_alloc);
    }
    sb->priv_cap = 0;
}

void smallbuffer_realloc(smallbuffer *sb, size_t capacity) {
    smallbuffer_realloc_with_custom_selfsize(sb, capacity, sizeof(smallbuffer));
}

void smallbuffer_realloc_with_custom_selfsize(smallbuffer *sb, size_t capacity, size_t sizeof_smallbuffer) {
    assert(capacity < PTRDIFF_MAX);
    assert(sizeof_smallbuffer >= sizeof(smallbuffer));

    size_t const sizeof_inline = sizeof_smallbuffer - sizeof(smallbuffer) + sizeof(sb->priv_inline);

    size_t old_capacity = smallbuffer_capacity(sb);
    bool is_alloc = is_sb_allocated(sb);
    bool wants_alloc = capacity > sizeof_inline;
    char *data;

    // update allocation
    if(is_alloc && wants_alloc) {
        sb->priv_alloc = omf_realloc(sb->priv_alloc, capacity);
        data = sb->priv_alloc;
    } else if(!is_alloc && wants_alloc) {
        data = omf_malloc(capacity);
        assert(old_capacity < capacity);
        memcpy(data, sb->priv_inline, old_capacity);
        sb->priv_alloc = data;
    } else if(is_alloc && !wants_alloc) {
        assert(capacity < old_capacity);
        data = sb->priv_alloc;
        memcpy(sb->priv_inline, data, capacity);
        omf_free(data);
        data = sb->priv_inline;
    } else { // !is_alloc && !wants_alloc
        data = sb->priv_inline;
    }

    // pad with nul bytes
    if(capacity > old_capacity) {
        memset(data + old_capacity, 0, capacity - old_capacity);
    }

    // update priv_cap
    if(wants_alloc) {
        sb->priv_cap = capacity;
    } else {
        // negative priv_cap indicates small buffer optimization.
        sb->priv_cap = -(ptrdiff_t)capacity;
    }
}

char *smallbuffer_data(smallbuffer *sb) {
    return is_sb_allocated(sb) ? sb->priv_alloc : sb->priv_inline;
}

size_t smallbuffer_capacity(smallbuffer *sb) {
    return is_sb_allocated(sb) ? sb->priv_cap : -sb->priv_cap;
}
