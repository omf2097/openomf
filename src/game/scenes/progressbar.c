#include <stdlib.h>
#include "game/scenes/progressbar.h"
#include "video/image.h"
#include "video/video.h"

void progressbar_create_block(progress_bar *bar) {
    float prog = bar->percentage / 100.0f;
    int w = bar->w * prog;
    if(w > 1 && bar->h > 1) {
        image tmp;
        image_create(&tmp, w, bar->h);
        image_clear(&tmp, bar->int_bg_color);
        image_rect_bevel(&tmp, 
                         0, 0, w - 1, bar->h-1, 
                         bar->int_topleft_color, 
                         bar->int_bottomright_color, 
                         bar->int_bottomright_color, 
                         bar->int_topleft_color);
        bar->block = malloc(sizeof(surface));
        surface_create_from_image(bar->block, &tmp);
        image_free(&tmp);
    }
}

void progressbar_create_flashing(progress_bar *bar,
                       unsigned int x, unsigned int y,
                       unsigned int w, unsigned int h,
                       color border_topleft_color,
                       color border_bottomright_color,
                       color bg_color,
                       color bg_color_alt,
                       color int_topleft_color,
                       color int_bottomright_color,
                       color int_bg_color,
                       int orientation) {
    bar->x = x;
    bar->y = y;
    bar->w = w;
    bar->h = h;
    bar->orientation = orientation;
    bar->percentage = 100;
    bar->int_topleft_color = int_topleft_color;
    bar->int_bottomright_color = int_bottomright_color;
    bar->int_bg_color = int_bg_color;
    bar->block = NULL;

    // Background,
    image tmp;
    image_create(&tmp, w, h);
    image_clear(&tmp, bg_color);
    image_rect_bevel(&tmp, 
                     0, 0, w-1, h-1, 
                     border_topleft_color, 
                     border_bottomright_color, 
                     border_bottomright_color, 
                     border_topleft_color);
    surface_create_from_image(&bar->background, &tmp);
    image_free(&tmp);

    image_create(&tmp, w, h);
    image_clear(&tmp, bg_color_alt);
    image_rect_bevel(&tmp, 
                     0, 0, w-1, h-1, 
                     border_topleft_color, 
                     border_bottomright_color, 
                     border_bottomright_color, 
                     border_topleft_color);
    surface_create_from_image(&bar->background_alt, &tmp);
    image_free(&tmp);

    // Bar
    progressbar_create_block(bar);
}

void progressbar_free(progress_bar *bar) {
    surface_free(&bar->background);
    surface_free(&bar->background_alt);
    if(bar->block != NULL) {
        surface_free(bar->block);
        free(bar->block);
        bar->block = NULL;
    }
}

void progressbar_set(progress_bar *bar, unsigned int percentage) {
    bar->percentage = (percentage > 100 ? 100 : percentage);
    if(bar->block != NULL) {
        surface_free(bar->block);
        free(bar->block);
        bar->block = NULL;
    }
    progressbar_create_block(bar);
}

void progressbar_render_flashing(progress_bar *bar, int flip) {
    if(flip) {
        video_render_sprite(&bar->background_alt, bar->x, bar->y, BLEND_ALPHA, 0);
    } else {
        video_render_sprite(&bar->background, bar->x, bar->y, BLEND_ALPHA, 0);
    }
    if(bar->block != NULL) {
        video_render_sprite(
            bar->block, 
            bar->x + (bar->orientation == PROGRESSBAR_LEFT ? 0 : bar->w - bar->block->w + 1), 
            bar->y, 
            BLEND_ALPHA,
            0);
    }
}
