#include "video/renderers/vulkan/helpers/vk_command.h"
#include "utils/allocator.h"
#include "utils/log.h"

bool vk_command_create(vk_command *cmd, VkDevice device, uint32_t graphics_family_index, uint32_t buffer_count) {
    VkCommandPoolCreateInfo pool_info = {0};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = graphics_family_index;

    if(vkCreateCommandPool(device, &pool_info, NULL, &cmd->command_pool) != VK_SUCCESS) {
        log_error("Failed to create command pool");
        return false;
    }

    cmd->buffer_count = buffer_count;
    cmd->command_buffers = omf_malloc(sizeof(VkCommandBuffer) * buffer_count);

    VkCommandBufferAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = cmd->command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = buffer_count;

    if(vkAllocateCommandBuffers(device, &alloc_info, cmd->command_buffers) != VK_SUCCESS) {
        log_error("Failed to allocate command buffers");
        vkDestroyCommandPool(device, cmd->command_pool, NULL);
        omf_free(cmd->command_buffers);
        return false;
    }

    return true;
}

void vk_command_destroy(vk_command *cmd, VkDevice device) {
    if(cmd->command_buffers) {
        vkFreeCommandBuffers(device, cmd->command_pool, cmd->buffer_count, cmd->command_buffers);
        omf_free(cmd->command_buffers);
        cmd->command_buffers = NULL;
    }

    if(cmd->command_pool) {
        vkDestroyCommandPool(device, cmd->command_pool, NULL);
        cmd->command_pool = VK_NULL_HANDLE;
    }
}
