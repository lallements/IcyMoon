#pragma once

#include <im3e/api/vulkan_loader.h>
#include <im3e/utils/types.h>
#include <im3e/vulkan_loaders/vulkan_loaders.h>

namespace im3e {

class VulkanInstance
{
public:
    VulkanInstance(std::unique_ptr<IVulkanLoader> pLoader = createVulkanLoader());

    auto getFcts() -> VulkanInstanceFcts& { return m_fcts; }

private:
    std::unique_ptr<IVulkanLoader> m_pLoader;
    VulkanGlobalFcts m_globalFcts;

    VulkanInstanceFcts m_fcts;
    VkUniquePtr<VkInstance> m_pVkInstance;
};

}  // namespace im3e