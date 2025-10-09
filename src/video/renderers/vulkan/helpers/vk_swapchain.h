#ifndef VK_SWAPCHAIN_H
#define VK_SWAPCHAIN_H

#include <stdbool.h>
#include <vulkan/vulkan.h>

/**
 * Vulkan swapchain context - manages the presentation images, image views, and framebuffers.
 * The swapchain is the bridge between rendering and displaying images on screen.
 */
typedef struct vk_swapchain {
    VkSwapchainKHR swapchain;    // Swapchain handle
    VkImage *images;             // Array of swapchain images (owned by swapchain)
    VkImageView *image_views;    // Array of image views for the swapchain images
    VkFramebuffer *framebuffers; // Array of framebuffers (one per swapchain image)
    uint32_t image_count;        // Number of images in the swapchain
    VkFormat image_format;       // Pixel format of swapchain images
    VkExtent2D extent;           // Width and height of swapchain images
} vk_swapchain;

/**
 * Create a Vulkan swapchain with appropriate format and present mode.
 * Chooses the best surface format (preferring BGRA8 SRGB) and present mode
 * (FIFO for vsync, IMMEDIATE for no vsync).
 *
 * @param sc Pointer to vk_swapchain struct to initialize
 * @param physical_device Physical device to query capabilities from
 * @param device Logical device to create swapchain on
 * @param surface Window surface to present to
 * @param window_w Desired window width
 * @param window_h Desired window height
 * @param vsync Whether to enable vsync (FIFO mode)
 * @param graphics_family_index Queue family index for graphics
 * @param present_family_index Queue family index for presentation
 * @return true on success, false on failure
 */
bool vk_swapchain_create(vk_swapchain *sc, VkPhysicalDevice physical_device, VkDevice device, VkSurfaceKHR surface,
                         int window_w, int window_h, bool vsync, uint32_t graphics_family_index,
                         uint32_t present_family_index);

/**
 * Create image views for all swapchain images.
 * Image views describe how to access the image data and are required for rendering.
 *
 * @param sc Swapchain context with images already created
 * @param device Logical device to create image views on
 * @return true on success, false on failure
 */
bool vk_swapchain_create_image_views(vk_swapchain *sc, VkDevice device);

/**
 * Create framebuffers for each swapchain image.
 * Framebuffers connect render passes to actual image attachments.
 *
 * @param sc Swapchain context with image views already created
 * @param device Logical device to create framebuffers on
 * @param render_pass Render pass that will be used with these framebuffers
 * @return true on success, false on failure
 */
bool vk_swapchain_create_framebuffers(vk_swapchain *sc, VkDevice device, VkRenderPass render_pass);

/**
 * Destroy swapchain and all associated resources.
 * Cleans up framebuffers, image views, swapchain, and image array.
 *
 * @param sc Swapchain context to destroy
 * @param device Logical device used to create the resources
 */
void vk_swapchain_destroy(vk_swapchain *sc, VkDevice device);

#endif // VK_SWAPCHAIN_H
