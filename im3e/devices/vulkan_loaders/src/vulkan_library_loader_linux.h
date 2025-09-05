#pragma once

#include <im3e/utils/core/throw_utils.h>
#include <im3e/utils/core/types.h>

#include <dlfcn.h>
#include <vulkan/vulkan.h>

#include <array>
#include <string_view>

namespace im3e::vulkan_loader {

struct VulkanLibraryLoader
{
    VulkanLibraryLoader()
    {
        constexpr std::array<std::string_view, 2U> VulkanLibraryNames{
            "libvulkan.so.1",
            "libvulkan.so",
        };

        for (const auto& rVulkanLibraryName : VulkanLibraryNames)
        {
            if (void* pLib = dlopen(rVulkanLibraryName.data(), RTLD_NOW))
            {
                this->pLibrary = UniquePtrWithDeleter<void>(pLib, [](void* pLib) { dlclose(pLib); });
                break;
            }
        }
        throwIfNull<std::runtime_error>(this->pLibrary.get(), "Could not load Vulkan shared library file");

        this->vkGetInstanceProcAddr = throwIfNull<std::runtime_error>(
            reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(this->pLibrary.get(), "vkGetInstanceProcAddr")),
            "Failed to load vkGetInstanceProcAddr");

        this->vkGetDeviceProcAddr = throwIfNull<std::runtime_error>(
            reinterpret_cast<PFN_vkGetDeviceProcAddr>(dlsym(this->pLibrary.get(), "vkGetDeviceProcAddr")),
            "Failed to load vkGetDeviceProcAddr");
    }

    UniquePtrWithDeleter<void> pLibrary;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
};

}  // namespace im3e::vulkan_loader