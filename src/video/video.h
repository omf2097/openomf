#ifndef VIDEO_H
#define VIDEO_H

#include <stdbool.h>

#include "formats/palette.h"
#include "video/color.h"
#include "video/enums.h"
#include "video/image.h"
#include "video/surface.h"

#define NATIVE_W 320
#define NATIVE_H 200

int video_init(int window_w, int window_h, bool fullscreen, bool vsync);
int video_reinit(int window_w, int window_h, bool fullscreen, bool vsync);
void video_reinit_renderer(void);
void video_get_state(int *w, int *h, int *fs, int *vsync);
void video_move_target(int x, int y);

/**
 * Render a sprite on the screen. Content of source surface will be saved to atlas for faster rendering.
 * This is the simplest way to draw.
 *
 * @param src_surface Source surface
 * @param x Destination X
 * @param y Destination y
 */
void video_draw(const surface *src_surface, int x, int y);

/**
 * Render a sprite on the screen using palette offset. Content of source surface will be saved to atlas for faster
 * rendering. This can be used to render a sprite using a modified palette offset. The offset is added to the
 * color index of each pixel.
 *
 * @param src_surface Source surface
 * @param x Destination X
 * @param y Destination y
 * @param palette_offset Palette offset (default = 0)
 * @param palette_limit Palette offset max limit (default = 255)
 */
void video_draw_offset(const surface *src_surface, int x, int y, int palette_offset, int palette_limit);

/**
 * Render a sprite on the screen using at given size. Surface will be scaled as needed to fit the
 * target size. Content of source surface will be saved to atlas for faster rendering.
 *
 * @param src_surface Source surface
 * @param x Destination X
 * @param y Destination Y
 * @param w Destination width
 * @param h Destination height
 */
void video_draw_size(const surface *src_surface, int x, int y, int w, int h);

/**
 * Render a sprite on the screen.
 *
 * @param src_surface
 * @param x
 * @param y
 * @param w
 * @param h
 * @param remap_offset
 * @param remap_rounds
 * @param palette_offset
 * @param palette_limit
 * @param flip_mode
 * @param options
 */
void video_draw_full(const surface *src_surface, int x, int y, int w, int h, int remap_offset, int remap_rounds,
                     int palette_offset, int palette_limit, unsigned int flip_mode, unsigned int options);

void video_reset_atlas(void);

void video_render_background(surface *sur);
void video_render_prepare(void);
void video_render_finish(void);
void video_close(void);
void video_screenshot(surface *sur);
void video_area_capture(surface *sur, int x, int y, int w, int h);
void video_set_fade(float fade);

void video_draw_atlas(bool draw_atlas);

#endif // VIDEO_H
