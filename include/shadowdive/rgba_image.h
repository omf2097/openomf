#ifndef _SD_RGBA_IMAGE_H
#define _SD_RGBA_IMAGE_H

#ifdef __cplusplus 
extern "C" {
#endif

typedef struct {
    unsigned int w;
    unsigned int h;
    unsigned int len;
    char *data;
} sd_rgba_image;

int sd_rgba_image_create(sd_rgba_image *img, unsigned int w, unsigned int h);
int sd_rgba_image_copy(sd_rgba_image *dst, const sd_rgba_image *src);
int sd_rgba_image_to_ppm(const sd_rgba_image *img, const char *filename);
void sd_rgba_image_free(sd_rgba_image *img);

#ifdef __cplusplus
}
#endif

#endif // _SD_RGBA_IMAGE_H
