/**
 * @file video.h
 * @brief Video subsystem interface for rendering sprites and managing display
 */

#ifndef VIDEO_H
#define VIDEO_H

#include <stdbool.h>

#include "formats/palette.h"
#include "video/color.h"
#include "video/enums.h"
#include "video/image.h"
#include "video/surface.h"

#define NATIVE_W 320 ///< Native game resolution width
#define NATIVE_H 200 ///< Native game resolution height

/**
 * @brief Callback type for asynchronous screenshot capture
 * @param rect Screen rectangle that was captured
 * @param data Raw pixel data (RGB format)
 * @param flipped Whether the image data is vertically flipped
 */
typedef void (*video_screenshot_signal)(const SDL_Rect *rect, unsigned char *data, bool flipped);

/**
 * @brief Scan for available renderers; should be run at startup.
 */
void video_scan_renderers(void);

/**
 * @brief Get the number of available renderers
 * @return Number of available renderers
 */
int video_get_renderer_count(void);

/**
 * @brief Get information about a renderer by index
 * @param index Renderer index (0 to video_get_renderer_count()-1)
 * @param name Output pointer for renderer name (can be NULL)
 * @param description Output pointer for renderer description (can be NULL)
 * @return true if renderer info was retrieved, false if index is out of range
 */
bool video_get_renderer_info(int index, const char **name, const char **description);

/**
 * @brief Initialize the video subsystem
 * @param try_name Preferred renderer name (NULL to use the best available)
 * @param window_w Window width in pixels
 * @param window_h Window height in pixels
 * @param fullscreen Enable fullscreen mode
 * @param vsync Enable vertical sync
 * @param aspect Aspect ratio mode
 * @param framerate_limit Maximum framerate (0 for unlimited)
 * @param fb_scale Framebuffer scale factor
 * @param scaling_mode Scaling algorithm mode
 * @return true on success, false on failure
 */
bool video_init(const char *try_name, int window_w, int window_h, bool fullscreen, bool vsync, int aspect,
                int framerate_limit, int fb_scale, int scaling_mode);

/**
 * @brief Reinitialize video with new settings (keeps current renderer)
 * @param window_w Window width in pixels
 * @param window_h Window height in pixels
 * @param fullscreen Enable fullscreen mode
 * @param vsync Enable vertical sync
 * @param aspect Aspect ratio mode
 * @param framerate_limit Maximum framerate (0 for unlimited)
 * @param fb_scale Framebuffer scale factor
 * @param scaling_mode Scaling algorithm mode
 * @return true on success, false on failure
 */
bool video_reinit(int window_w, int window_h, bool fullscreen, bool vsync, int aspect, int framerate_limit,
                  int fb_scale, int scaling_mode);

/**
 * @brief Reset the renderer context (e.g., after losing OpenGL context)
 */
void video_reinit_renderer(void);

/**
 * @brief Get current video state
 * @param w Output for window width (can be NULL)
 * @param h Output for window height (can be NULL)
 * @param fs Output for fullscreen state (can be NULL)
 * @param vsync Output for vsync state (can be NULL)
 * @param aspect Output for aspect ratio mode (can be NULL)
 * @param fb_scale Output for framebuffer scale (can be NULL)
 */
void video_get_state(int *w, int *h, bool *fs, bool *vsync, int *aspect, int *fb_scale);

/**
 * @brief Move the rendering target position
 * @param x Horizontal offset
 * @param y Vertical offset
 */
void video_move_target(int x, int y);

/**
 * @brief Render a sprite on the screen
 *
 * This is the simplest way to draw.
 *
 * @param src_surface Source surface
 * @param x Destination X
 * @param y Destination Y
 */
void video_draw(const surface *src_surface, int x, int y);

/**
 * @brief Render a sprite using palette remapping
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
 * @brief Render a sprite with palette offset
 *
 * This can be used to render a sprite using a modified palette offset.
 * The offset is added to the color index of each pixel. This tricks is used to e.g. render the
 * second HAR using the second palette.
 *
 * @param src_surface Source surface
 * @param x Destination X
 * @param y Destination Y
 * @param palette_offset Palette offset (default = 0)
 * @param palette_limit Palette offset max limit (default = 255)
 */
void video_draw_offset(const surface *src_surface, int x, int y, int palette_offset, int palette_limit);

/**
 * @brief Render a sprite at a given size
 *
 * Surface will be scaled as needed to fit the target size.
 *
 * @param src_surface Source surface
 * @param x Destination X
 * @param y Destination Y
 * @param w Destination width
 * @param h Destination height
 */
void video_draw_size(const surface *src_surface, int x, int y, int w, int h);

/**
 * @brief Render a sprite with full control over all rendering parameters
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

/**
 * @brief Signal that the scene has changed
 *
 * This notifies the renderer that the scene content has changed significantly,
 * allowing it to optimize caching and resource management.
 */
void video_signal_scene_change(void);

/**
 * @brief Prepare for rendering a frame
 * @param framebuffer_options Framebuffer configuration options
 */
void video_render_prepare(unsigned framebuffer_options);

/**
 * @brief Finish rendering the current frame and present it
 */
void video_render_finish(void);

/**
 * @brief Prepare for rendering to a specific screen area
 * @param area Rectangle defining the render area
 */
void video_render_area_prepare(const SDL_Rect *area);

/**
 * @brief Finish rendering to an area and copy result to surface
 * @param dst Destination surface to receive the rendered content
 */
void video_render_area_finish(surface *dst);

/**
 * @brief Close the video subsystem and release all resources
 */
void video_close(void);

/**
 * @brief Schedule an asynchronous screenshot capture
 * @param callback Function to call when screenshot is ready
 */
void video_schedule_screenshot(video_screenshot_signal callback);

/**
 * @brief Toggle atlas debug visualization
 * @param draw_atlas true to enable atlas drawing, false to disable
 */
void video_draw_atlas(bool draw_atlas);

#endif // VIDEO_H
