#pragma once

#include <im3e/api/logger.h>
#include <im3e/api/vulkan_loader.h>
#include <im3e/utils/vk_utils.h>

#include <memory>

namespace im3e {

class VulkanDebugMessageHandler
{
public:
    VulkanDebugMessageHandler(const ILogger& logger, const VulkanInstanceFcts& rFcts, VkInstance vkInstance);

    static VkDebugUtilsMessengerCreateInfoEXT generateDebugUtilsCreateInfo(ILogger& logger);

private:
    std::unique_ptr<ILogger> m_pLogger;
    const VulkanInstanceFcts& m_rFcts;

    VkUniquePtr<VkDebugUtilsMessengerEXT> m_pVkMessenger;
};

}  // namespace im3e