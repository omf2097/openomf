#include <xmp.h>

#ifndef MIN_LIB_BUILD
#error "xmp_stub.c is part of MIN_LIB_BUILD"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* PSM playback needs libxmp, which is not part of MIN_LIB_BUILD.
   This stand-in opens the file and feeds silence so the game still runs. */

typedef struct xmp_stub {
    struct xmp_module mod;
    int volume;
    int loaded;
} xmp_stub;

xmp_context xmp_create_context(void) {
    xmp_stub *ctx = (xmp_stub *)calloc(1, sizeof(xmp_stub));
    if(ctx == NULL) {
        return NULL;
    }
    ctx->volume = 100;
    memcpy(ctx->mod.name, "silence", 8);
    memcpy(ctx->mod.type, "MIN_LIB_BUILD", 14);
    return (xmp_context)ctx;
}

void xmp_free_context(xmp_context context) {
    free(context);
}

int xmp_load_module(xmp_context context, const char *path) {
    xmp_stub *ctx = (xmp_stub *)context;
    FILE *fp;
    if(ctx == NULL || path == NULL) {
        return -1;
    }
    fp = fopen(path, "rb");
    if(fp == NULL) {
        return -1;
    }
    fclose(fp);
    ctx->loaded = 1;
    return 0;
}

void xmp_release_module(xmp_context context) {
    xmp_stub *ctx = (xmp_stub *)context;
    if(ctx != NULL) {
        ctx->loaded = 0;
    }
}

void xmp_get_module_info(xmp_context context, struct xmp_module_info *info) {
    xmp_stub *ctx = (xmp_stub *)context;
    if(info == NULL) {
        return;
    }
    info->mod = (ctx != NULL) ? &ctx->mod : NULL;
}

int xmp_start_player(xmp_context context, int rate, int flags) {
    (void)rate;
    (void)flags;
    xmp_stub *ctx = (xmp_stub *)context;
    if(ctx == NULL || !ctx->loaded) {
        return -1;
    }
    return 0;
}

void xmp_end_player(xmp_context context) {
    (void)context;
}

int xmp_play_buffer(xmp_context context, void *buffer, int size, int loop) {
    (void)context;
    (void)loop;
    if(buffer != NULL && size > 0) {
        memset(buffer, 0, (size_t)size);
    }
    return 0;
}

int xmp_set_player(xmp_context context, int param, int value) {
    xmp_stub *ctx = (xmp_stub *)context;
    if(ctx == NULL) {
        return -1;
    }
    if(param == XMP_PLAYER_VOLUME) {
        ctx->volume = value;
    }
    return 0;
}
