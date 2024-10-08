#pragma once

#include <im3e/api/device.h>

#include <memory>

namespace im3e {

auto createVulkanMemoryAllocator(std::shared_ptr<const IDevice> pDevice, VmaVulkanFunctions vmaFcts)
    -> std::unique_ptr<IMemoryAllocator>;

}  // namespace im3e