#pragma once

#include <im3e/utils/vk_utils.h>

namespace im3e {

/// @brief Pointers to Vulkan global functions.
/// The list of global functions can be found in vkGetInstanceProcAddr reference:
/// https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#vkGetInstanceProcAddr
struct VulkanGlobalFcts
{
    PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion{};
    PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties{};
    PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties{};
    PFN_vkCreateInstance vkCreateInstance{};
};

struct VulkanInstanceFcts
{
    PFN_vkDestroyInstance vkDestroyInstance{};
    PFN_vkCreateDevice vkCreateDevice{};

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT{};
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT{};

    PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices{};
    PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties{};
    PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures{};
    PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties{};
    PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties{};
    PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties{};
};

struct VulkanDeviceFcts
{
    PFN_vkDestroyDevice vkDestroyDevice{};
    PFN_vkGetDeviceQueue vkGetDeviceQueue{};
    PFN_vkGetImageSubresourceLayout vkGetImageSubresourceLayout{};
    PFN_vkMapMemory vkMapMemory{};
    PFN_vkUnmapMemory vkUnmapMemory{};
};

class IVulkanLoader
{
public:
    virtual ~IVulkanLoader() = default;

    virtual auto loadGlobalFcts() const -> VulkanGlobalFcts = 0;
    virtual auto loadInstanceFcts(VkInstance vkInstance) const -> VulkanInstanceFcts = 0;
    virtual auto loadDeviceFcts(VkDevice vkDevice) const -> VulkanDeviceFcts = 0;
    virtual auto loadVmaFcts(VkInstance vkInstance, VkDevice vkDevice) const -> VmaVulkanFunctions = 0;
};

}  // namespace im3e