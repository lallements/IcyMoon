#pragma once

#include <im3e/api/vulkan_loader.h>
#include <im3e/utils/types.h>

namespace im3e {

class VulkanLoader : public IVulkanLoader
{
public:
    VulkanLoader(UniquePtrWithDeleter<void> pLibrary, PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
                 PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr);

    auto loadGlobalFcts() -> VulkanGlobalFcts override;
    auto loadInstanceFcts(VkInstance vkInstance) -> VulkanInstanceFcts override;
    auto loadDeviceFcts(VkDevice vkDevice) -> VulkanDeviceFcts override;

private:
    UniquePtrWithDeleter<void> m_pLibrary;
    PFN_vkGetInstanceProcAddr m_vkGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr m_vkGetDeviceProcAddr;
};

}  // namespace im3e