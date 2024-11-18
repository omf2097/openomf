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

typedef void (*video_screenshot_signal)(const SDL_Rect *rect, unsigned char *data,
                                        bool flipped); // Asynchronous screenshot signal

bool video_init(int window_w, int window_h, bool fullscreen, bool vsync);
bool video_reinit(int window_w, int window_h, bool fullscreen, bool vsync);
void video_reinit_renderer(void);
void video_get_state(int *w, int *h, bool *fs, bool *vsync);
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
 * Render a sprite using remapping.
 *
 * @param src_surface Source surface
 * @param x Destination X
 * @param y Destination Y
 * @param remap_offset Remapping offset to use
 * @param remap_rounds Count of rounds to run the pixel through the remapping table
 * @param options Renderer effect options
 */
void video_draw_remap(const surface *src_surface, int x, int y, int remap_offset, int remap_rounds,
                      unsigned int options);

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
 * @param src_surface Source surface
 * @param x Destination X
 * @param y Destination Y
 * @param w Destination width
 * @param h Destination height
 * @param remap_offset Remapping offset to use
 * @param remap_rounds Count of rounds to run the pixel through the remapping table
 * @param palette_offset Palette offset (default = 0)
 * @param palette_limit Palette offset max limit (default = 255)
 * @param opacity Opacity of the image (0-255, default = 255 -- fully visible)
 * @param flip_mode Sprite flipping options
 * @param options Renderer effect options
 */
void video_draw_full(const surface *src_surface, int x, int y, int w, int h, int remap_offset, int remap_rounds,
                     int palette_offset, int palette_limit, int opacity, unsigned int flip_mode, unsigned int options);

void video_signal_scene_change(void);

void video_render_prepare(void);
void video_render_finish(void);
void video_render_area_prepare(const SDL_Rect *area);
void video_render_area_finish(surface *dst);

void video_close(void);
void video_schedule_screenshot(video_screenshot_signal callback);

void video_draw_atlas(bool draw_atlas);

#endif // VIDEO_H
