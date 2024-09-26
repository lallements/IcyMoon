#pragma once

#include "devices.h"
#include "vulkan_instance.h"

#include <im3e/api/device.h>
#include <im3e/api/logger.h>
#include <im3e/api/vulkan_loader.h>
#include <im3e/utils/types.h>

#include <memory>

namespace im3e {

class VulkanDevice : public IDevice
{
public:
    VulkanDevice(const ILogger& rLogger, DeviceConfig config);

private:
    std::unique_ptr<ILogger> m_pLogger;
    const DeviceConfig m_config;
    VulkanInstance m_instance;
};

}  // namespace im3e