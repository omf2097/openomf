#ifdef USE_OGGVORBIS

#include <stdio.h>
#include <stdlib.h>
#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include "audio/sources/vorbis_source.h"
#include "utils/allocator.h"
#include "utils/log.h"

typedef struct vorbis_source_t {
    OggVorbis_File src_file;
    int current_section;
} vorbis_source;

const char* vorbis_text_error(int id) {
    switch(id) {
        case OV_EREAD: return "OV_EREAD";
        case OV_ENOTVORBIS: return "OV_ENOTVORBIS";
        case OV_EVERSION: return "OV_EVERSION";
        case OV_EBADHEADER: return "OV_EBADHEADER";
        case OV_EFAULT: return "OV_EFAULT";
        case OV_HOLE: return "OV_HOLE";
        case OV_EBADLINK: return "OV_EBADLINK";
        case OV_EINVAL: return "OV_EINVAL";
        default:
            return "unknown error";
    }
}

int vorbis_stream(audio_source *src, char *buffer, int len) {
    vorbis_source *local = source_get_userdata(src);

    int read = 0;
    int ret;
    while(read < len) {
        ret = ov_read(
                &local->src_file,
                buffer+read, len-read, // Output buffer & length
                0, 2, 1, // endian byte packing, word size (2), signedness (signed)
                &local->current_section); // number of the current logical bitstream
        read += ret;
        if(ret < 0) {
            DEBUG("Vorbis Source: Error %d while streaming: %s.", ret, vorbis_text_error(ret));
            return read;
        } else if(ret == 0) {
            return read;
        }
    }
    return read;
}

int vorbis_source_update(audio_source *src, char *buffer, int len) {
    vorbis_source *local = source_get_userdata(src);
    int got = vorbis_stream(src, buffer, len);
    if(got == 0 && src->loop) {
        ov_raw_seek(&local->src_file, 0);
        return vorbis_stream(src, buffer, len);
    }
    return got;
}

void vorbis_source_close(audio_source *src) {
    vorbis_source *local = source_get_userdata(src);
    ov_clear(&local->src_file);
    omf_free(local);
    source_set_userdata(src, local);
    DEBUG("Vorbis Source: Closed.");
}

int vorbis_source_init(audio_source *src, const char* file) {
    vorbis_source *local;
    int ret;

    // Init local struct
    local = omf_calloc(1, sizeof(vorbis_source));

    // Try to open up the audio file
    ret = ov_fopen(file, &local->src_file);
    if(ret != 0) {
        PERROR("Vorbis Source: File '%s' could not be opened: ", file, vorbis_text_error(ret));
        goto error_1;
    }

    // Get file information
    vorbis_info *vi = ov_info(&local->src_file, -1);
    char **comment_ptr = ov_comment(&local->src_file, -1)->user_comments;

    // Audio information
    source_set_frequency(src, vi->rate);
    source_set_bytes(src, 2);
    source_set_channels(src, vi->channels);

    // Set callbacks
    source_set_userdata(src, local);
    source_set_update_cb(src, vorbis_source_update);
    source_set_close_cb(src, vorbis_source_close);

    // Some debug info
    DEBUG("Vorbis Source: Loaded file '%s' succesfully (%d Hz, %d ch).",
        file, vi->rate, vi->channels);
    while(*comment_ptr) {
        DEBUG(" * Comment: %s", *comment_ptr);
        ++comment_ptr;
    }

    // All done
    return 0;
error_1:
    omf_free(local);
    return 1;
}

#endif // USE_OGGVORBIS
