#pragma once

#include "vulkan_instance.h"

#include <im3e/api/device.h>
#include <im3e/api/vulkan_loader.h>
#include <im3e/utils/types.h>

#include <memory>

namespace im3e {

class Device : public IDevice
{
public:
    Device();

private:
    VulkanInstance m_instance;
};

}  // namespace im3e