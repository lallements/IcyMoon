#pragma once

#include "vulkan_extensions.h"

#include <im3e/api/logger.h>
#include <im3e/api/vulkan_loader.h>
#include <im3e/utils/types.h>
#include <im3e/vulkan_loaders/vulkan_loaders.h>

namespace im3e {

class VulkanInstance
{
public:
    VulkanInstance(const ILogger& rLogger, bool isDebugEnabled,
                   std::unique_ptr<IVulkanLoader> pLoader = createVulkanLoader());

    auto getFcts() -> VulkanInstanceFcts& { return m_fcts; }

private:
    std::unique_ptr<ILogger> m_pLogger;
    std::unique_ptr<IVulkanLoader> m_pLoader;
    VulkanGlobalFcts m_globalFcts;

    VulkanExtensions m_extensions;

    VulkanInstanceFcts m_fcts;
    VkUniquePtr<VkInstance> m_pVkInstance;
};

}  // namespace im3e