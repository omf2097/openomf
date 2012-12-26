#ifndef _VGA_IMAGE
#define _VGA_IMAGE

#include "rgba_image.h"
#include "palette.h"

typedef struct vga_image_t {
    unsigned int w;
    unsigned int h;
    unsigned int len;
    char *data;
} vga_image;

vga_image* sd_vga_image_create(unsigned int w, unsigned int h);
void sd_vga_image_delete(vga_image *img);
vga_image* sd_vga_image_encode(rgba_image *img, palette *pal, int remapping);
rgba_image* sd_vga_image_decode(vga_image *img, palette *pal, int remapping);

#endif // _VGA_IMAGE
