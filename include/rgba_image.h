#ifndef _RGBA_IMAGE
#define _RGBA_IMAGE

typedef struct rgba_image_t {
    unsigned int w;
    unsigned int h;
    char *data;
} rgba_image;

rgba_image* sd_rgba_image_create(unsigned int w, unsigned int h);
void sd_rgba_image_delete(rgba_image *img);

#endif // _RGBA_IMAGE
