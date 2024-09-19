#pragma once

#include <vulkan/vulkan.h>

namespace im3e {

/// @brief Pointers to Vulkan global functions.
/// The list of global functions can be found in vkGetInstanceProcAddr reference:
/// https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#vkGetInstanceProcAddr
struct VulkanGlobalFcts
{
    PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;
    PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
    PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
    PFN_vkCreateInstance vkCreateInstance;
};

struct VulkanInstanceFcts
{
    PFN_vkDestroyInstance vkDestroyInstance;
    PFN_vkCreateDevice vkCreateDevice;
};

struct VulkanDeviceFcts
{
    PFN_vkDestroyDevice vkDestroyDevice;
};

class IVulkanLoader
{
public:
    virtual ~IVulkanLoader() = default;

    virtual auto loadGlobalFcts() -> VulkanGlobalFcts = 0;
    virtual auto loadInstanceFcts(VkInstance vkInstance) -> VulkanInstanceFcts = 0;
    virtual auto loadDeviceFcts(VkDevice vkDevice) -> VulkanDeviceFcts = 0;
};

}  // namespace im3e