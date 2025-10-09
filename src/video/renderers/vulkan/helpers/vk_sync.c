#include "video/renderers/vulkan/helpers/vk_sync.h"
#include "utils/log.h"

bool vk_sync_create(vk_sync *sync, VkDevice device) {
    VkSemaphoreCreateInfo semaphore_info = {0};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {0};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if(vkCreateSemaphore(device, &semaphore_info, NULL, &sync->image_available_semaphore) != VK_SUCCESS ||
       vkCreateSemaphore(device, &semaphore_info, NULL, &sync->render_finished_semaphore) != VK_SUCCESS ||
       vkCreateFence(device, &fence_info, NULL, &sync->in_flight_fence) != VK_SUCCESS) {
        log_error("Failed to create synchronization objects");
        return false;
    }

    return true;
}

void vk_sync_destroy(vk_sync *sync, VkDevice device) {
    if(sync->render_finished_semaphore) {
        vkDestroySemaphore(device, sync->render_finished_semaphore, NULL);
        sync->render_finished_semaphore = VK_NULL_HANDLE;
    }

    if(sync->image_available_semaphore) {
        vkDestroySemaphore(device, sync->image_available_semaphore, NULL);
        sync->image_available_semaphore = VK_NULL_HANDLE;
    }

    if(sync->in_flight_fence) {
        vkDestroyFence(device, sync->in_flight_fence, NULL);
        sync->in_flight_fence = VK_NULL_HANDLE;
    }
}
