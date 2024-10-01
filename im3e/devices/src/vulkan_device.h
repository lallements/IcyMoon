#pragma once

#include "devices.h"
#include "vulkan_instance.h"

#include <im3e/api/device.h>
#include <im3e/api/image.h>
#include <im3e/api/logger.h>
#include <im3e/api/vulkan_loader.h>
#include <im3e/utils/types.h>
#include <im3e/utils/vk_utils.h>

#include <memory>
#include <vector>

namespace im3e {

class VulkanDevice : public IDevice, public std::enable_shared_from_this<VulkanDevice>
{
public:
    VulkanDevice(const ILogger& rLogger, DeviceConfig config);

    auto getImageFactory() const -> std::shared_ptr<const IImageFactory> override;

private:
    std::unique_ptr<ILogger> m_pLogger;
    const DeviceConfig m_config;
    const VulkanInstance m_instance;
    const VulkanPhysicalDevice m_physicalDevice;
    VulkanDeviceFcts m_fcts;

    VkUniquePtr<VkDevice> m_pVkDevice;
    VkUniquePtr<VmaAllocator> m_pVmaAllocator;

    std::vector<VkQueue> m_vkComputeQueues;
    std::vector<VkQueue> m_vkGraphicsQueues;
    std::vector<VkQueue> m_vkTransferQueues;
    std::vector<VkQueue> m_vkPresentationQueues;

    mutable std::shared_ptr<IImageFactory> m_pImageFactory;
};

}  // namespace im3e