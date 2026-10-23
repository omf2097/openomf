#ifndef XMP_H
#define XMP_H

#define XMP_VERCODE 0x040600

#define XMP_FORMAT_8BIT (1 << 0)
#define XMP_FORMAT_UNSIGNED (1 << 1)
#define XMP_FORMAT_MONO (1 << 2)

#define XMP_PLAYER_INTERP 2
#define XMP_PLAYER_VOLUME 7

#define XMP_INTERP_NEAREST 0
#define XMP_INTERP_LINEAR 1
#define XMP_INTERP_SPLINE 2

typedef char *xmp_context;

struct xmp_module {
    char name[64];
    char type[64];
};

struct xmp_module_info {
    struct xmp_module *mod;
};

xmp_context xmp_create_context(void);
void xmp_free_context(xmp_context context);
int xmp_load_module(xmp_context context, const char *path);
void xmp_release_module(xmp_context context);
void xmp_get_module_info(xmp_context context, struct xmp_module_info *info);
int xmp_start_player(xmp_context context, int rate, int flags);
void xmp_end_player(xmp_context context);
int xmp_play_buffer(xmp_context context, void *buffer, int size, int loop);
int xmp_set_player(xmp_context context, int param, int value);

#endif /* XMP_H */
