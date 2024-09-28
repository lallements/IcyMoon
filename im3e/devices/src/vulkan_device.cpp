#include "vulkan_device.h"

#include <im3e/utils/throw_utils.h>

using namespace im3e;
using namespace std;

VulkanDevice::VulkanDevice(const ILogger& rLogger, DeviceConfig config)
  : m_pLogger(rLogger.createChild("VulkanDevice"))
  , m_config(move(config))
  , m_instance(*m_pLogger, m_config.isDebugEnabled,
               createVulkanLoader(VulkanLoaderConfig{
                   .isDebugEnabled = config.isDebugEnabled,
               }))
  , m_physicalDevice(m_instance.choosePhysicalDevice(config.isPresentationSupported))
{
}

auto im3e::createDevice(const ILogger& rLogger, DeviceConfig config) -> shared_ptr<IDevice>
{
    return make_shared<VulkanDevice>(rLogger, move(config));
}