#pragma once

#include <im3e/api/vulkan_loader.h>
#include <im3e/utils/types.h>

namespace im3e {

class VulkanInstance
{
public:
    VulkanInstance();

    auto getFcts() -> VulkanInstanceFcts& { return m_fcts; }

private:
    std::unique_ptr<IVulkanLoader> m_pLoader;
    VulkanGlobalFcts m_globalFcts;

    VulkanInstanceFcts m_fcts;
    VkUniquePtr<VkInstance> m_pVkInstance;
};

}  // namespace im3e