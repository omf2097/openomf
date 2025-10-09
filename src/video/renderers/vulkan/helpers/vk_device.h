#ifndef VK_DEVICE_H
#define VK_DEVICE_H

#include <stdbool.h>
#include <vulkan/vulkan.h>

/**
 * Vulkan device context - manages physical device selection, logical device creation,
 * and queue management for graphics and presentation operations.
 */
typedef struct vk_device {
    VkInstance instance;              // Vulkan instance handle (not owned by this struct)
    VkPhysicalDevice physical_device; // Selected GPU device
    VkDevice device;                  // Logical device handle
    VkQueue graphics_queue;           // Queue for graphics commands
    VkQueue present_queue;            // Queue for presentation (may be same as graphics)
    uint32_t graphics_family_index;   // Index of graphics queue family
    uint32_t present_family_index;    // Index of present queue family
} vk_device;

/**
 * Create a Vulkan logical device and select appropriate queues.
 * Picks the first available physical device and finds queue families that support
 * both graphics operations and presentation to the given surface.
 *
 * @param dev Pointer to vk_device struct to initialize
 * @param instance Vulkan instance to use
 * @param surface Surface that will be used for presentation
 * @return true on success, false on failure
 */
bool vk_device_create(vk_device *dev, VkInstance instance, VkSurfaceKHR surface);

/**
 * Destroy the Vulkan logical device and release resources.
 * Does not destroy the instance or surface (those are owned by the caller).
 *
 * @param dev Pointer to vk_device struct to clean up
 */
void vk_device_destroy(vk_device *dev);

#endif // VK_DEVICE_H
