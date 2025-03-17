#include "audio/sources/xmp_source.h"
#include "utils/allocator.h"
#include "utils/log.h"

#include <xmp.h>

static void xmp_render(void *userdata, char *stream, int len) {
    xmp_context ctx = userdata;
    assert(ctx);
    xmp_play_buffer(ctx, stream, len, 0);
}

static void xmp_close(void *userdata) {
    xmp_context context = userdata;
    if(context != NULL) {
        xmp_end_player(context);
        xmp_release_module(context);
        xmp_free_context(context);
    }
}

bool xmp_load(music_source *src, int channels, int sample_rate, int resampler, const char *file) {
    xmp_context context;
    if((context = xmp_create_context()) == NULL) {
        log_error("Unable to initialize XMP context.");
        goto exit_0;
    }

    // Load the module file
    if(xmp_load_module(context, file) < 0) {
        log_error("Unable to open module file");
        goto exit_0;
    }

    // Show some information
    struct xmp_module_info mi;
    xmp_get_module_info(context, &mi);
    log_debug("Loaded music track %s (%s)", mi.mod->name, mi.mod->type);

    // Start the player
    int flags = 0;
    if(channels == 1)
        flags |= XMP_FORMAT_MONO;
    if(xmp_start_player(context, sample_rate, flags) != 0) {
        log_error("Unable to start module playback");
        goto exit_1;
    }
    if(xmp_set_player(context, XMP_PLAYER_INTERP, resampler) != 0) {
        log_error("Unable to set music resampler");
        goto exit_2;
    }
    if(xmp_set_player(context, XMP_PLAYER_VOLUME, 100) != 0) {
        log_error("Unable to set music volume");
        goto exit_2;
    }

    src->context = context;
    src->render = xmp_render;
    src->close = xmp_close;
    return true;

exit_2:
    xmp_end_player(context);
exit_1:
    xmp_release_module(context);
exit_0:
    return false;
}
