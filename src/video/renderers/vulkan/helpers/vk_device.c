#include "video/renderers/vulkan/helpers/vk_device.h"
#include "utils/allocator.h"
#include "utils/log.h"

/**
 * Find queue families that support graphics and presentation.
 * Searches through all available queue families on the physical device to find
 * one that supports graphics operations and one that can present to the surface.
 * These may be the same queue family.
 *
 * @param dev Device context to store queue family indices
 * @param surface Surface to check presentation support against
 * @return true if both graphics and present queues were found, false otherwise
 */
static bool find_queue_families(vk_device *dev, VkSurfaceKHR surface) {
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(dev->physical_device, &queue_family_count, NULL);

    VkQueueFamilyProperties *queue_families = omf_malloc(sizeof(VkQueueFamilyProperties) * queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(dev->physical_device, &queue_family_count, queue_families);

    bool graphics_found = false;
    bool present_found = false;

    for(uint32_t i = 0; i < queue_family_count; i++) {
        if(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            dev->graphics_family_index = i;
            graphics_found = true;
        }

        VkBool32 present_support = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(dev->physical_device, i, surface, &present_support);
        if(present_support) {
            dev->present_family_index = i;
            present_found = true;
        }

        if(graphics_found && present_found) {
            break;
        }
    }

    omf_free(queue_families);
    return graphics_found && present_found;
}

/**
 * Select a physical device (GPU) to use for rendering.
 * Currently just picks the first available device. A more sophisticated
 * implementation could rank devices by suitability.
 *
 * @param dev Device context to store physical device
 * @param surface Surface to check queue family support against
 * @return true if a suitable device was found, false otherwise
 */
static bool pick_physical_device(vk_device *dev, VkSurfaceKHR surface) {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(dev->instance, &device_count, NULL);

    if(device_count == 0) {
        log_error("Failed to find GPUs with Vulkan support");
        return false;
    }

    VkPhysicalDevice *devices = omf_malloc(sizeof(VkPhysicalDevice) * device_count);
    vkEnumeratePhysicalDevices(dev->instance, &device_count, devices);

    // Just pick the first device for now
    dev->physical_device = devices[0];

    // Log device properties
    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(dev->physical_device, &device_properties);

    log_info("Vulkan device selected:");
    log_info(" * Device: %s", device_properties.deviceName);
    log_info(" * Vendor ID: 0x%X", device_properties.vendorID);
    log_info(" * Device ID: 0x%X", device_properties.deviceID);
    log_info(" * API Version: %d.%d.%d", VK_VERSION_MAJOR(device_properties.apiVersion),
             VK_VERSION_MINOR(device_properties.apiVersion), VK_VERSION_PATCH(device_properties.apiVersion));
    log_info(" * Driver Version: %d.%d.%d", VK_VERSION_MAJOR(device_properties.driverVersion),
             VK_VERSION_MINOR(device_properties.driverVersion), VK_VERSION_PATCH(device_properties.driverVersion));

    const char *device_type = "Unknown";
    switch(device_properties.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            device_type = "Integrated GPU";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            device_type = "Discrete GPU";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            device_type = "Virtual GPU";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            device_type = "CPU";
            break;
        default:
            break;
    }
    log_info(" * Type: %s", device_type);

    omf_free(devices);

    return find_queue_families(dev, surface);
}

/**
 * Create a logical device with required queues and extensions.
 * Sets up queue create infos for graphics and present queues (which may be the same),
 * enables the swapchain extension, and creates the logical device.
 *
 * @param dev Device context with physical device and queue indices already set
 * @return true on success, false on failure
 */
static bool create_logical_device(vk_device *dev) {
    float queue_priority = 1.0f;

    // Create queue create infos
    VkDeviceQueueCreateInfo queue_create_infos[2];
    uint32_t queue_create_info_count = 0;

    // Graphics queue
    queue_create_infos[queue_create_info_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[queue_create_info_count].pNext = NULL;
    queue_create_infos[queue_create_info_count].flags = 0;
    queue_create_infos[queue_create_info_count].queueFamilyIndex = dev->graphics_family_index;
    queue_create_infos[queue_create_info_count].queueCount = 1;
    queue_create_infos[queue_create_info_count].pQueuePriorities = &queue_priority;
    queue_create_info_count++;

    // Present queue (if different from graphics)
    if(dev->graphics_family_index != dev->present_family_index) {
        queue_create_infos[queue_create_info_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[queue_create_info_count].pNext = NULL;
        queue_create_infos[queue_create_info_count].flags = 0;
        queue_create_infos[queue_create_info_count].queueFamilyIndex = dev->present_family_index;
        queue_create_infos[queue_create_info_count].queueCount = 1;
        queue_create_infos[queue_create_info_count].pQueuePriorities = &queue_priority;
        queue_create_info_count++;
    }

    VkPhysicalDeviceFeatures device_features = {0};

    const char *device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = queue_create_info_count;
    create_info.pQueueCreateInfos = queue_create_infos;
    create_info.pEnabledFeatures = &device_features;
    create_info.enabledExtensionCount = 1;
    create_info.ppEnabledExtensionNames = device_extensions;
    create_info.enabledLayerCount = 0;

    if(vkCreateDevice(dev->physical_device, &create_info, NULL, &dev->device) != VK_SUCCESS) {
        log_error("Failed to create logical device");
        return false;
    }

    vkGetDeviceQueue(dev->device, dev->graphics_family_index, 0, &dev->graphics_queue);
    vkGetDeviceQueue(dev->device, dev->present_family_index, 0, &dev->present_queue);

    log_info("Vulkan logical device created:");
    log_info(" * Graphics queue family: %u", dev->graphics_family_index);
    log_info(" * Present queue family: %u", dev->present_family_index);
    if(dev->graphics_family_index == dev->present_family_index) {
        log_info(" * Using unified queue for graphics and present");
    } else {
        log_info(" * Using separate queues for graphics and present");
    }

    return true;
}

bool vk_device_create(vk_device *dev, VkInstance instance, VkSurfaceKHR surface) {
    dev->instance = instance;

    if(!pick_physical_device(dev, surface)) {
        return false;
    }

    if(!create_logical_device(dev)) {
        return false;
    }

    return true;
}

void vk_device_destroy(vk_device *dev) {
    if(dev->device) {
        vkDestroyDevice(dev->device, NULL);
        dev->device = VK_NULL_HANDLE;
    }
}
