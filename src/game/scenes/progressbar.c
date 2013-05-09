#include "game/scenes/progressbar.h"
#include "video/image.h"
#include "video/video.h"

void progressbar_create(progress_bar *bar,     
                       unsigned int x, unsigned int y,
                       unsigned int w, unsigned int h,
                       color border_topleft_color,
                       color border_bottomright_color,
                       color bg_color,
                       color int_color, 
                       int orientation) {
    bar->x = x;
    bar->y = y;
    bar->w = w;
    bar->h = h;
    bar->orientation = orientation;
    bar->percentage = 0;
    
    // Temporary image for drawing necessary textures
    image tmp;
    
    // Background
    image_create(&tmp, w, h);
    image_clear(&tmp, bg_color);
    image_rect_bevel(&tmp, 
                     0, 0, w-1, h-1, 
                     border_topleft_color, 
                     border_bottomright_color, 
                     border_bottomright_color, 
                     border_topleft_color);
    texture_create_from_img(&bar->background, &tmp);
    image_free(&tmp);
    
    // Fill color. Since the texture is single color,
    // we can just create a small texture with a right color and stretch it
    image_create(&tmp, 5, 5);
    image_clear(&tmp, int_color);
    texture_create_from_img(&bar->block, &tmp);
    image_free(&tmp);
}

void progressbar_free(progress_bar *bar) {
    texture_free(&bar->background);
    texture_free(&bar->block);
}

void progressbar_set(progress_bar *bar, unsigned int percentage) {
    bar->percentage = (percentage > 100 ? 100 : percentage);
}

void progressbar_render(progress_bar *bar) {
    // Render background
    video_render_sprite(&bar->background, bar->x, bar->y, BLEND_ALPHA_FULL);
    
    // Render progressbar itself. "Cheat" a bit with texture size.
    int oldw = bar->block.w;
    int oldh = bar->block.h;
    float prog = bar->percentage / 100.0f;
    if(prog > 0) {
        bar->block.w = bar->w * prog;
        bar->block.h = bar->h;
        if(bar->orientation == PROGRESSBAR_LEFT) {
            video_render_sprite(&bar->block, bar->x, bar->y, BLEND_ALPHA_FULL);
        } else {
            video_render_sprite(&bar->block, bar->x + (bar->w - bar->block.w), bar->y, BLEND_ALPHA_FULL);
        }
        bar->block.w = oldw;
        bar->block.h = oldh;
    }
}
