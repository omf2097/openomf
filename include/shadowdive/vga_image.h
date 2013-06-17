#ifndef _SD_VGA_IMAGE_H
#define _SD_VGA_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _SD_RGBA_IMAGE_H
typedef struct sd_rgba_image_t sd_rgba_image;
#endif

#ifndef _SD_PALETTE_H
typedef struct sd_palette_t sd_palette;
#endif

typedef struct sd_vga_image_t {
    unsigned int w;
    unsigned int h;
    unsigned int len;
    char *data;
} sd_vga_image;

sd_vga_image* sd_vga_image_create(unsigned int w, unsigned int h);
void sd_vga_image_delete(sd_vga_image *img);
sd_vga_image* sd_vga_image_encode(sd_rgba_image *img, sd_palette *pal, int remapping);
sd_rgba_image* sd_vga_image_decode(sd_vga_image *img, sd_palette *pal, int remapping);

#ifdef __cplusplus
}
#endif

#endif // _SD_VGA_IMAGE_H
