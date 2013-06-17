#ifndef _SD_RGBA_IMAGE_H
#define _SD_RGBA_IMAGE_H

#ifdef __cplusplus 
extern "C" {
#endif

typedef struct sd_rgba_image_t {
    unsigned int w;
    unsigned int h;
    char *data;
} sd_rgba_image;

sd_rgba_image* sd_rgba_image_create(unsigned int w, unsigned int h);
void sd_rgba_image_to_ppm(sd_rgba_image *img, const char *filename);
void sd_rgba_image_delete(sd_rgba_image *img);

#ifdef __cplusplus
}
#endif

#endif // _SD_RGBA_IMAGE_H
