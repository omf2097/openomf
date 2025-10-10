#ifndef VK_RENDERPASS_H
#define VK_RENDERPASS_H

#include <stdbool.h>
#include <vulkan/vulkan.h>

/**
 * Create a Vulkan render pass for basic color rendering.
 * The render pass defines how attachments are used during rendering, including
 * load/store operations and layout transitions.
 *
 * @param render_pass Pointer to VkRenderPass to create
 * @param device Logical device to create render pass on
 * @param swapchain_format Format of the swapchain images to render to
 * @return true on success, false on failure
 */
bool vk_renderpass_create(VkRenderPass *render_pass, VkDevice device, VkFormat swapchain_format);

/**
 * Destroy a render pass.
 *
 * @param render_pass Render pass to destroy
 * @param device Logical device used to create the render pass
 */
void vk_renderpass_destroy(VkRenderPass render_pass, VkDevice device);

#endif // VK_RENDERPASS_H
