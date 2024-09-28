#pragma once

#include "vulkan_debug_message_handler.h"
#include "vulkan_extensions.h"
#include "vulkan_physical_devices.h"

#include <im3e/api/logger.h>
#include <im3e/api/vulkan_loader.h>
#include <im3e/utils/types.h>
#include <im3e/vulkan_loaders/vulkan_loaders.h>

namespace im3e {

class VulkanInstance
{
public:
    VulkanInstance(const ILogger& rLogger, bool isDebugEnabled, std::unique_ptr<IVulkanLoader> pLoader);

    auto choosePhysicalDevice(const IsPresentationSupportedFct& rIsPresentationSupported) -> VulkanPhysicalDevice;

    auto getFcts() -> VulkanInstanceFcts& { return m_fcts; }

private:
    std::unique_ptr<ILogger> m_pLogger;
    std::unique_ptr<IVulkanLoader> m_pLoader;
    VulkanGlobalFcts m_globalFcts;

    VulkanExtensions m_extensions;

    VulkanInstanceFcts m_fcts;
    VkUniquePtr<VkInstance> m_pVkInstance;
    std::unique_ptr<VulkanDebugMessageHandler> m_pDebugMessageHandler;
};

}  // namespace im3e