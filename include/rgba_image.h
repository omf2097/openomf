#ifndef _RGBA_IMAGE
#define _RGBA_IMAGE

typedef struct rgba_image_t {
    unsigned int w;
    unsigned int h;
    char *data;
} sd_rgba_image;

sd_rgba_image* sd_rgba_image_create(unsigned int w, unsigned int h);
void sd_rgba_image_delete(sd_rgba_image *img);

#endif // _RGBA_IMAGE
