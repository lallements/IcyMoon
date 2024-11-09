#pragma once

#include "vulkan_memory_allocator.h"

#include <im3e/api/device.h>
#include <im3e/api/image.h>

namespace im3e {

auto createVulkanImageFactory(const IDevice& rDevice, std::shared_ptr<IVulkanMemoryAllocator> pMemoryAllocator)
    -> std::unique_ptr<IImageFactory>;

}  // namespace im3e