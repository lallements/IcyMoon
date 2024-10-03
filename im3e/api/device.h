#pragma once

#include "image.h"
#include "vulkan_loader.h"

namespace im3e {

class IDevice
{
public:
    virtual ~IDevice() = default;

    virtual auto getVkDevice() const -> VkDevice = 0;
    virtual auto getVmaAllocator() const -> VmaAllocator = 0;
    virtual auto getFcts() const -> const VulkanDeviceFcts& = 0;
    virtual auto getImageFactory() const -> std::shared_ptr<const IImageFactory> = 0;
};

}  // namespace im3e