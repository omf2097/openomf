#include "video/renderers/vulkan/vk_renderer.h"
#include "video/renderers/common.h"

#include "video/renderers/vulkan/helpers/vk_command.h"
#include "video/renderers/vulkan/helpers/vk_device.h"
#include "video/renderers/vulkan/helpers/vk_renderpass.h"
#include "video/renderers/vulkan/helpers/vk_swapchain.h"
#include "video/renderers/vulkan/helpers/vk_sync.h"

#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/miscmath.h"
#include "video/vga_state.h"

#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

#define NATIVE_W 320 // Native resolution width (VGA)
#define NATIVE_H 200 // Native resolution height (VGA)

/**
 * Main Vulkan renderer context.
 * Contains all Vulkan state including window, instance, device, swapchain,
 * and rendering synchronization primitives.
 */
typedef struct vk_context {
    // Window and Vulkan instance
    SDL_Window *window;       // SDL window for displaying
    VkInstance instance;      // Vulkan instance
    VkSurfaceKHR surface;     // Window surface for presentation
    VkRenderPass render_pass; // Render pass for basic rendering

    // Helper modules
    vk_device device;       // Device and queue management
    vk_swapchain swapchain; // Swapchain, images, and framebuffers
    vk_command command;     // Command pool and buffers
    vk_sync sync;           // Synchronization objects

    // Display settings
    int viewport_w;    // Actual viewport width (may differ from window)
    int viewport_h;    // Actual viewport height
    int screen_w;      // Window width
    int screen_h;      // Window height
    int fb_scale;      // Framebuffer scale factor
    bool fullscreen;   // Fullscreen mode enabled
    bool vsync;        // Vertical sync enabled
    int aspect;        // Aspect ratio mode (0=4:3, 1=stretch)
    int target_move_x; // Screen shake X offset
    int target_move_y; // Screen shake Y offset

    // Frame timing
    uint64_t framerate_limit; // Frame time limit in performance counter ticks
    uint64_t last_tick;       // Last frame timestamp

    video_screenshot_signal screenshot_cb; // Callback for screenshot capture
} vk_context;

static bool is_available(void) {
    /*
    uint32_t extension_count = 0;
    if(SDL_Vulkan_GetInstanceExtensions(NULL, &extension_count, NULL) == SDL_FALSE) {
        return false;
    }
    */
    return true;
}

static const char *get_description(void) {
    return "Hardware Vulkan renderer";
}

static const char *get_name(void) {
    return "Vulkan";
}

static void set_framerate_limit(vk_context *ctx, int framerate_limit) {
    if(framerate_limit == 0) {
        ctx->framerate_limit = 0;
    } else {
        ctx->framerate_limit = (1.0 / framerate_limit) * SDL_GetPerformanceFrequency();
    }
}

/**
 * Create a Vulkan instance with SDL-required extensions.
 * The instance is the connection between the application and the Vulkan library.
 */
static bool create_instance(vk_context *ctx) {
    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "OpenOMF";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "OpenOMF";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    // Get required extensions from SDL
    uint32_t sdl_extension_count = 0;
    if(SDL_Vulkan_GetInstanceExtensions(ctx->window, &sdl_extension_count, NULL) == SDL_FALSE) {
        log_error("Failed to get SDL Vulkan extension count");
        return false;
    }

    const char **sdl_extensions = omf_malloc(sizeof(char *) * sdl_extension_count);
    if(SDL_Vulkan_GetInstanceExtensions(ctx->window, &sdl_extension_count, sdl_extensions) == SDL_FALSE) {
        log_error("Failed to get SDL Vulkan extensions");
        omf_free(sdl_extensions);
        return false;
    }

    VkInstanceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = sdl_extension_count;
    create_info.ppEnabledExtensionNames = sdl_extensions;
    create_info.enabledLayerCount = 0;

    VkResult result = vkCreateInstance(&create_info, NULL, &ctx->instance);

    if(result != VK_SUCCESS) {
        log_error("Failed to create Vulkan instance: %d", result);
        omf_free(sdl_extensions);
        return false;
    }

    log_info("Vulkan instance created with %u extensions:", sdl_extension_count);
    for(uint32_t i = 0; i < sdl_extension_count; i++) {
        log_info(" * %s", sdl_extensions[i]);
    }

    omf_free(sdl_extensions);
    return true;
}

/**
 * Initialize the Vulkan renderer context.
 * Sets up the window, Vulkan instance, device, swapchain, and all rendering resources.
 */
static bool setup_context(void *userdata, int window_w, int window_h, bool fullscreen, bool vsync, int aspect,
                          int framerate_limit, int fb_scale) {
    vk_context *ctx = userdata;
    ctx->screen_w = window_w;
    ctx->screen_h = window_h;
    ctx->fullscreen = fullscreen;
    ctx->framerate_limit = framerate_limit;
    ctx->fb_scale = fb_scale;
    ctx->vsync = vsync;
    ctx->aspect = aspect;
    ctx->target_move_x = 0;
    ctx->target_move_y = 0;
    ctx->last_tick = SDL_GetPerformanceCounter();
    set_framerate_limit(ctx, framerate_limit);

    // Create SDL window with Vulkan support
    uint32_t flags = SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN;
    if(fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    ctx->window =
        SDL_CreateWindow("OpenOMF", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_w, window_h, flags);
    if(!ctx->window) {
        log_error("Failed to create SDL window: %s", SDL_GetError());
        goto error_0;
    }

    if(!create_instance(ctx)) {
        goto error_1;
    }

    if(SDL_Vulkan_CreateSurface(ctx->window, ctx->instance, &ctx->surface) == SDL_FALSE) {
        log_error("Failed to create Vulkan surface: %s", SDL_GetError());
        goto error_2;
    }

    if(!vk_device_create(&ctx->device, ctx->instance, ctx->surface)) {
        goto error_3;
    }

    if(!vk_swapchain_create(&ctx->swapchain, ctx->device.physical_device, ctx->device.device, ctx->surface, window_w,
                            window_h, vsync, ctx->device.graphics_family_index, ctx->device.present_family_index)) {
        goto error_4;
    }

    if(!vk_swapchain_create_image_views(&ctx->swapchain, ctx->device.device)) {
        goto error_5;
    }

    if(!vk_renderpass_create(&ctx->render_pass, ctx->device.device, ctx->swapchain.image_format)) {
        goto error_6;
    }

    if(!vk_swapchain_create_framebuffers(&ctx->swapchain, ctx->device.device, ctx->render_pass)) {
        goto error_7;
    }

    if(!vk_command_create(&ctx->command, ctx->device.device, ctx->device.graphics_family_index,
                          ctx->swapchain.image_count)) {
        goto error_8;
    }

    if(!vk_sync_create(&ctx->sync, ctx->device.device)) {
        goto error_9;
    }

    SDL_Vulkan_GetDrawableSize(ctx->window, &ctx->viewport_w, &ctx->viewport_h);

    vga_state_mark_dirty();

    log_info("Vulkan Renderer initialized successfully!");
    log_info(" * Window size: %dx%d", ctx->screen_w, ctx->screen_h);
    log_info(" * Drawable size: %dx%d", ctx->viewport_w, ctx->viewport_h);
    log_info(" * Framebuffer scale: %dx", ctx->fb_scale);
    log_info(" * Fullscreen: %s", ctx->fullscreen ? "Yes" : "No");
    log_info(" * VSync: %s", ctx->vsync ? "Enabled" : "Disabled");
    log_info(" * Aspect ratio: %s", ctx->aspect == 0 ? "4:3" : "Stretch");
    return true;

error_9:
    vk_command_destroy(&ctx->command, ctx->device.device);

error_8:
    vk_swapchain_destroy(&ctx->swapchain, ctx->device.device);

error_7:
    vk_renderpass_destroy(ctx->render_pass, ctx->device.device);

error_6:
    // Image views are destroyed by vk_swapchain_destroy

error_5:
    vk_swapchain_destroy(&ctx->swapchain, ctx->device.device);

error_4:
    vk_device_destroy(&ctx->device);

error_3:
    vkDestroySurfaceKHR(ctx->instance, ctx->surface, NULL);

error_2:
    vkDestroyInstance(ctx->instance, NULL);

error_1:
    SDL_DestroyWindow(ctx->window);

error_0:
    return false;
}

static void get_context_state(void *userdata, int *window_w, int *window_h, bool *fullscreen, bool *vsync, int *aspect,
                              int *fb_scale) {
    vk_context *ctx = userdata;
    if(window_w != NULL)
        *window_w = ctx->screen_w;
    if(window_h != NULL)
        *window_h = ctx->screen_h;
    if(fullscreen != NULL)
        *fullscreen = ctx->fullscreen;
    if(vsync != NULL)
        *vsync = ctx->vsync;
    if(aspect != NULL)
        *aspect = ctx->aspect;
    if(fb_scale != NULL)
        *fb_scale = ctx->fb_scale;
}

static bool reset_context_with(void *userdata, int window_w, int window_h, bool fullscreen, bool vsync, int aspect,
                               int framerate_limit, int fb_scale) {
    vk_context *ctx = userdata;
    ctx->screen_w = window_w;
    ctx->screen_h = window_h;
    ctx->fullscreen = fullscreen;
    ctx->vsync = vsync;
    ctx->aspect = aspect;

    set_framerate_limit(ctx, framerate_limit);

    // For now, just log that we would need to recreate the swapchain
    // Full implementation would require recreating the swapchain
    log_info("Vulkan renderer reset requested - not fully implemented yet.");
    return true;
}

static void reset_context(void *userdata) {
    return;
}

static void close_context(void *userdata) {
    vk_context *ctx = userdata;

    vkDeviceWaitIdle(ctx->device.device);

    vk_sync_destroy(&ctx->sync, ctx->device.device);
    vk_command_destroy(&ctx->command, ctx->device.device);
    vk_swapchain_destroy(&ctx->swapchain, ctx->device.device);
    vk_renderpass_destroy(ctx->render_pass, ctx->device.device);
    vk_device_destroy(&ctx->device);
    vkDestroySurfaceKHR(ctx->instance, ctx->surface, NULL);
    vkDestroyInstance(ctx->instance, NULL);
    SDL_DestroyWindow(ctx->window);

    log_info("Vulkan renderer closed.");
}

static void draw_surface(void *userdata, const surface *src_surface, SDL_Rect *dst, int remap_offset, int remap_rounds,
                         int palette_offset, int palette_limit, int opacity, unsigned int flip_mode,
                         unsigned int options) {
    // vk_context *ctx = userdata;
    // TODO: Implement surface drawing
    // This would involve uploading texture data and recording draw commands
}

static void move_target(void *userdata, int x, int y) {
    vk_context *ctx = userdata;
    ctx->target_move_x = x;
    ctx->target_move_y = y;
}

static void render_prepare(void *userdata, unsigned framebuffer_options) {
    // vk_context *ctx = userdata;
    // TODO: Implement render preparation
}

/**
 * Finish rendering a frame and present it to the screen.
 * Waits for the previous frame to finish, acquires a swapchain image,
 * records and submits rendering commands, then presents the result.
 */
static void render_finish(void *userdata) {
    vk_context *ctx = userdata;

    // Wait for previous frame to finish rendering
    vkWaitForFences(ctx->device.device, 1, &ctx->sync.in_flight_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(ctx->device.device, 1, &ctx->sync.in_flight_fence);

    // Acquire next swapchain image
    uint32_t image_index;
    vkAcquireNextImageKHR(ctx->device.device, ctx->swapchain.swapchain, UINT64_MAX, ctx->sync.image_available_semaphore,
                          VK_NULL_HANDLE, &image_index);

    // Record command buffer for this frame
    VkCommandBuffer command_buffer = ctx->command.command_buffers[image_index];
    vkResetCommandBuffer(command_buffer, 0);

    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    vkBeginCommandBuffer(command_buffer, &begin_info);

    VkRenderPassBeginInfo render_pass_info = {0};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = ctx->render_pass;
    render_pass_info.framebuffer = ctx->swapchain.framebuffers[image_index];
    render_pass_info.renderArea.offset.x = 0;
    render_pass_info.renderArea.offset.y = 0;
    render_pass_info.renderArea.extent = ctx->swapchain.extent;

    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;

    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    // TODO: Record actual drawing commands here

    vkCmdEndRenderPass(command_buffer);
    vkEndCommandBuffer(command_buffer);

    // Submit command buffer to graphics queue
    VkSubmitInfo submit_info = {0};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {ctx->sync.image_available_semaphore};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    VkSemaphore signal_semaphores[] = {ctx->sync.render_finished_semaphore};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    vkQueueSubmit(ctx->device.graphics_queue, 1, &submit_info, ctx->sync.in_flight_fence);

    // Present the rendered image to the screen
    VkPresentInfoKHR present_info = {0};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swapchains[] = {ctx->swapchain.swapchain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &image_index;

    vkQueuePresentKHR(ctx->device.present_queue, &present_info);

    // Manual framerate limiting (when vsync is off or additional limiting is needed)
    if(ctx->framerate_limit != 0) {
        const uint64_t frame_time = SDL_GetPerformanceCounter() - ctx->last_tick;
        if(frame_time < ctx->framerate_limit) {
            const double wait = ctx->framerate_limit - frame_time;
            const double ms_conv = SDL_GetPerformanceFrequency() / 1000;
            SDL_Delay(wait / ms_conv);
        }
        ctx->last_tick = SDL_GetPerformanceCounter();
    }
}

static void render_area_prepare(void *userdata, const SDL_Rect *area) {
    // vk_context *ctx = userdata;
    // TODO: Implement offscreen rendering preparation
}

static void render_area_finish(void *userdata, surface *dst) {
    // vk_context *ctx = userdata;
    // TODO: Implement offscreen rendering finish
}

static void capture_screen(void *userdata, video_screenshot_signal screenshot_cb) {
    vk_context *ctx = userdata;
    ctx->screenshot_cb = screenshot_cb;
}

static void signal_scene_change(void *userdata) {
    // vk_context *ctx = userdata;
    // TODO: Implement scene change handling
}

static void signal_draw_atlas(void *userdata, bool toggle) {
    // vk_context *ctx = userdata;
    // TODO: Implement atlas drawing toggle
}

/**
 * Allocate renderer context structure.
 */
static void renderer_create(renderer *vk_renderer) {
    vk_renderer->ctx = omf_calloc(1, sizeof(vk_context));
}

/**
 * Free renderer context structure.
 */
static void renderer_destroy(renderer *vk_renderer) {
    omf_free(vk_renderer->ctx);
}

/**
 * Set all renderer callback functions.
 * This is called by the video system to initialize the renderer interface.
 */
void vk_renderer_set_callbacks(renderer *vk_renderer) {
    vk_renderer->is_available = is_available;
    vk_renderer->get_description = get_description;
    vk_renderer->get_name = get_name;

    vk_renderer->create = renderer_create;
    vk_renderer->destroy = renderer_destroy;

    vk_renderer->setup_context = setup_context;
    vk_renderer->get_context_state = get_context_state;
    vk_renderer->reset_context_with = reset_context_with;
    vk_renderer->reset_context = reset_context;
    vk_renderer->close_context = close_context;

    vk_renderer->draw_surface = draw_surface;
    vk_renderer->move_target = move_target;
    vk_renderer->render_prepare = render_prepare;
    vk_renderer->render_finish = render_finish;
    vk_renderer->render_area_prepare = render_area_prepare;
    vk_renderer->render_area_finish = render_area_finish;

    vk_renderer->capture_screen = capture_screen;
    vk_renderer->signal_scene_change = signal_scene_change;
    vk_renderer->signal_draw_atlas = signal_draw_atlas;
}
