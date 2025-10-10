#ifndef VK_COMMAND_H
#define VK_COMMAND_H

#include <stdbool.h>
#include <vulkan/vulkan.h>

/**
 * Vulkan command context - manages command pools and command buffers.
 * Command buffers are used to record rendering commands which are then submitted to queues.
 */
typedef struct vk_command {
    VkCommandPool command_pool;       // Pool from which command buffers are allocated
    VkCommandBuffer *command_buffers; // Array of command buffers (one per swapchain image)
    uint32_t buffer_count;            // Number of command buffers
} vk_command;

/**
 * Create a command pool and allocate command buffers.
 * The command pool is created with the RESET_COMMAND_BUFFER flag to allow
 * individual command buffer resets.
 *
 * @param cmd Pointer to vk_command struct to initialize
 * @param device Logical device to create pool on
 * @param graphics_family_index Queue family index for graphics operations
 * @param buffer_count Number of command buffers to allocate
 * @return true on success, false on failure
 */
bool vk_command_create(vk_command *cmd, VkDevice device, uint32_t graphics_family_index, uint32_t buffer_count);

/**
 * Destroy command pool and free command buffers.
 * Command buffers are automatically freed when the pool is destroyed.
 *
 * @param cmd Command context to destroy
 * @param device Logical device used to create the resources
 */
void vk_command_destroy(vk_command *cmd, VkDevice device);

#endif // VK_COMMAND_H
