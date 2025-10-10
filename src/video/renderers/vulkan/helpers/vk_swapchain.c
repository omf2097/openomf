#include "video/renderers/vulkan/helpers/vk_swapchain.h"
#include "utils/allocator.h"
#include "utils/log.h"

bool vk_swapchain_create(vk_swapchain *sc, VkPhysicalDevice physical_device, VkDevice device, VkSurfaceKHR surface,
                         int window_w, int window_h, bool vsync, uint32_t graphics_family_index,
                         uint32_t present_family_index) {
    // Query surface capabilities and supported formats/modes
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, NULL);
    VkSurfaceFormatKHR *formats = omf_malloc(sizeof(VkSurfaceFormatKHR) * format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, formats);

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, NULL);
    VkPresentModeKHR *present_modes = omf_malloc(sizeof(VkPresentModeKHR) * present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes);

    // Choose surface format (prefer BGRA8 SRGB for better color accuracy)
    VkSurfaceFormatKHR surface_format = formats[0];
    for(uint32_t i = 0; i < format_count; i++) {
        if(formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surface_format = formats[i];
            break;
        }
    }

    // Choose present mode (FIFO is guaranteed to be available and provides vsync)
    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR; // Always available, vsync enabled
    if(!vsync) {
        // Try to find IMMEDIATE mode for no vsync (lower latency but potential tearing)
        for(uint32_t i = 0; i < present_mode_count; i++) {
            if(present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
                break;
            }
        }
    }

    // Choose swap extent (resolution of swapchain images)
    VkExtent2D extent;
    if(capabilities.currentExtent.width != UINT32_MAX) {
        extent = capabilities.currentExtent;
    } else {
        extent.width = (uint32_t)window_w;
        extent.height = (uint32_t)window_h;
        extent.width =
            (extent.width < capabilities.minImageExtent.width) ? capabilities.minImageExtent.width : extent.width;
        extent.width =
            (extent.width > capabilities.maxImageExtent.width) ? capabilities.maxImageExtent.width : extent.width;
        extent.height =
            (extent.height < capabilities.minImageExtent.height) ? capabilities.minImageExtent.height : extent.height;
        extent.height =
            (extent.height > capabilities.maxImageExtent.height) ? capabilities.maxImageExtent.height : extent.height;
    }

    // Request one more than the minimum to avoid waiting on the driver
    uint32_t image_count = capabilities.minImageCount + 1;
    if(capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
        image_count = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queue_family_indices[] = {graphics_family_index, present_family_index};
    if(graphics_family_index != present_family_index) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    create_info.preTransform = capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    if(vkCreateSwapchainKHR(device, &create_info, NULL, &sc->swapchain) != VK_SUCCESS) {
        log_error("Failed to create swapchain");
        omf_free(formats);
        omf_free(present_modes);
        return false;
    }

    sc->image_format = surface_format.format;
    sc->extent = extent;

    vkGetSwapchainImagesKHR(device, sc->swapchain, &sc->image_count, NULL);
    sc->images = omf_malloc(sizeof(VkImage) * sc->image_count);
    vkGetSwapchainImagesKHR(device, sc->swapchain, &sc->image_count, sc->images);

    // Log swapchain details
    const char *format_name = "Unknown";
    if(surface_format.format == VK_FORMAT_B8G8R8A8_SRGB) {
        format_name = "BGRA8_SRGB";
    } else if(surface_format.format == VK_FORMAT_B8G8R8A8_UNORM) {
        format_name = "BGRA8_UNORM";
    } else if(surface_format.format == VK_FORMAT_R8G8B8A8_SRGB) {
        format_name = "RGBA8_SRGB";
    } else if(surface_format.format == VK_FORMAT_R8G8B8A8_UNORM) {
        format_name = "RGBA8_UNORM";
    }

    const char *present_mode_name = "Unknown";
    if(present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
        present_mode_name = "IMMEDIATE (no vsync)";
    } else if(present_mode == VK_PRESENT_MODE_FIFO_KHR) {
        present_mode_name = "FIFO (vsync)";
    } else if(present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
        present_mode_name = "MAILBOX";
    }

    log_info("Vulkan swapchain created:");
    log_info(" * Format: %s", format_name);
    log_info(" * Present mode: %s", present_mode_name);
    log_info(" * Extent: %ux%u", extent.width, extent.height);
    log_info(" * Image count: %u", sc->image_count);

    omf_free(formats);
    omf_free(present_modes);
    return true;
}

bool vk_swapchain_create_image_views(vk_swapchain *sc, VkDevice device) {
    sc->image_views = omf_malloc(sizeof(VkImageView) * sc->image_count);

    for(uint32_t i = 0; i < sc->image_count; i++) {
        VkImageViewCreateInfo create_info = {0};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = sc->images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = sc->image_format;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        if(vkCreateImageView(device, &create_info, NULL, &sc->image_views[i]) != VK_SUCCESS) {
            log_error("Failed to create image view %u", i);
            return false;
        }
    }

    return true;
}

bool vk_swapchain_create_framebuffers(vk_swapchain *sc, VkDevice device, VkRenderPass render_pass) {
    sc->framebuffers = omf_malloc(sizeof(VkFramebuffer) * sc->image_count);

    for(uint32_t i = 0; i < sc->image_count; i++) {
        VkImageView attachments[] = {sc->image_views[i]};

        VkFramebufferCreateInfo framebuffer_info = {0};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = sc->extent.width;
        framebuffer_info.height = sc->extent.height;
        framebuffer_info.layers = 1;

        if(vkCreateFramebuffer(device, &framebuffer_info, NULL, &sc->framebuffers[i]) != VK_SUCCESS) {
            log_error("Failed to create framebuffer %u", i);
            return false;
        }
    }

    return true;
}

void vk_swapchain_destroy(vk_swapchain *sc, VkDevice device) {
    if(sc->framebuffers) {
        for(uint32_t i = 0; i < sc->image_count; i++) {
            vkDestroyFramebuffer(device, sc->framebuffers[i], NULL);
        }
        omf_free(sc->framebuffers);
        sc->framebuffers = NULL;
    }

    if(sc->image_views) {
        for(uint32_t i = 0; i < sc->image_count; i++) {
            vkDestroyImageView(device, sc->image_views[i], NULL);
        }
        omf_free(sc->image_views);
        sc->image_views = NULL;
    }

    if(sc->swapchain) {
        vkDestroySwapchainKHR(device, sc->swapchain, NULL);
        sc->swapchain = VK_NULL_HANDLE;
    }

    if(sc->images) {
        omf_free(sc->images);
        sc->images = NULL;
    }
}
