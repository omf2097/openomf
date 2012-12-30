#ifndef _VGA_IMAGE
#define _VGA_IMAGE

typedef struct sd_rgba_image_t sd_rgba_image;
typedef struct sd_palette_t sd_palette;

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

#endif // _VGA_IMAGE
