#ifndef VK_SYNC_H
#define VK_SYNC_H

#include <stdbool.h>
#include <vulkan/vulkan.h>

/**
 * Vulkan synchronization context - manages semaphores and fences for frame synchronization.
 * Semaphores coordinate GPU-GPU operations, fences coordinate CPU-GPU operations.
 */
typedef struct vk_sync {
    VkSemaphore image_available_semaphore; // Signals when swapchain image is ready for rendering
    VkSemaphore render_finished_semaphore; // Signals when rendering is complete and ready to present
    VkFence in_flight_fence;               // Ensures we don't render more than one frame at a time
} vk_sync;

/**
 * Create synchronization objects for frame rendering.
 * The fence is created in the signaled state so the first frame doesn't wait forever.
 *
 * @param sync Pointer to vk_sync struct to initialize
 * @param device Logical device to create sync objects on
 * @return true on success, false on failure
 */
bool vk_sync_create(vk_sync *sync, VkDevice device);

/**
 * Destroy synchronization objects.
 *
 * @param sync Sync context to destroy
 * @param device Logical device used to create the resources
 */
void vk_sync_destroy(vk_sync *sync, VkDevice device);

#endif // VK_SYNC_H
