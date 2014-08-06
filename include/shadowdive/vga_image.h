#ifndef _SD_VGA_IMAGE_H
#define _SD_VGA_IMAGE_H

#include "shadowdive/rgba_image.h"
#include "shadowdive/palette.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned int w; // Pixel width
    unsigned int h; // Pixel height
    unsigned int len; // Byte length
    char *data; // Palette representation of image data
    char *stencil; // holds 0 or 1 indicating whether a pixel is present
} sd_vga_image;

int sd_vga_image_create(sd_vga_image *img, unsigned int w, unsigned int h);
int sd_vga_image_copy(sd_vga_image *dst, const sd_vga_image *src);
void sd_vga_image_free(sd_vga_image *img);

int sd_vga_image_encode(
    sd_vga_image *dst,
    const sd_rgba_image *src,
    const sd_palette *pal,
    int remapping);

int sd_vga_image_decode(
    sd_rgba_image *dst,
    const sd_vga_image *src,
    const sd_palette *pal,
    int remapping);

#ifdef __cplusplus
}
#endif

#endif // _SD_VGA_IMAGE_H
