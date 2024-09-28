#pragma once

#include "devices.h"
#include "vulkan_instance.h"

#include <im3e/api/device.h>
#include <im3e/api/logger.h>
#include <im3e/api/vulkan_loader.h>
#include <im3e/utils/types.h>

#include <memory>
#include <vector>

namespace im3e {

class VulkanDevice : public IDevice
{
public:
    VulkanDevice(const ILogger& rLogger, DeviceConfig config);

private:
    std::unique_ptr<ILogger> m_pLogger;
    const DeviceConfig m_config;
    const VulkanInstance m_instance;
    const VulkanPhysicalDevice m_physicalDevice;
    VulkanDeviceFcts m_fcts;

    VkUniquePtr<VkDevice> m_pVkDevice;

    std::vector<VkQueue> m_vkComputeQueues;
    std::vector<VkQueue> m_vkGraphicsQueues;
    std::vector<VkQueue> m_vkTransferQueues;
    std::vector<VkQueue> m_vkPresentationQueues;
};

}  // namespace im3e